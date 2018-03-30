#include "query_builder.hpp"
#include "context.hpp"
#include "operators.hpp"
#include "optimizer.hpp"

#include <functional>
#include <string>

using namespace std;

namespace SQL_Compiler {
    namespace {
        class QueryBuilder : boost::static_visitor<BaseOperator*> {
        public:
            QueryBuilder() = default;

            BaseOperator* operator()(Context const& ctx, string const& filename) {
                return new CSV_Reader(filename);
            }

            vector<BaseOperator*> build_operators(Context const& ctx, SQL_AST::cartesian_product const& prod) {
                vector<BaseOperator*> operators;
                operators.reserve(prod.size());
                for (auto const& rel : prod) {
                    if (auto load_file_ptr = boost::get<SQL_AST::load_file>(&rel)) {
                        auto const& load_file = *load_file_ptr;
                        operators.push_back((*this)(ctx, load_file.filename_));
                    }
                    else if (auto subquery_ptr = boost::get<SQL_AST::subquery>(&rel)) {
                        auto const& subquery = *subquery_ptr;
                        auto visitor = std::bind(*this, ctx, std::placeholders::_1);
                        auto ptr = boost::apply_visitor(visitor, subquery.query_);
                        operators.push_back(ptr);
                    }
                }
                return operators;
            }

            BaseOperator* operator()(Context const& ctx, SQL_AST::minus_op const& minus_op) {
                auto visitor = std::bind(*this, ctx, std::placeholders::_1);
                auto left = boost::apply_visitor(visitor, minus_op.left_);
                auto right = boost::apply_visitor(visitor, minus_op.right_);
                return new MinusOperator(left, right);
            }

            BaseOperator* operator()(Context const& ctx, SQL_AST::union_op const& union_op) {
                auto visitor = std::bind(*this, ctx, std::placeholders::_1);
                auto left = boost::apply_visitor(visitor, union_op.left_);
                auto right = boost::apply_visitor(visitor, union_op.right_);
                return new UnionOperator(left, right);
            }

            boost::optional<SQL_AST::attribute> get_attribute(SQL_AST::operand const& op) {
                if (auto attr_ptr = boost::get<SQL_AST::attribute>(&op)) {
                    return *attr_ptr;
                }
                return boost::none;
            }

            pair<vector<JoinKey>, SQL_AST::and_conditions>
            extract_join(SQL_AST::and_conditions const& and_conds, string const& left, string const& right) {
                vector<JoinKey> extracted;
                SQL_AST::and_conditions remaining;
                for (auto const& cond : and_conds) {
                    if (auto comp_ptr = boost::get<SQL_AST::comparison_condition>(&cond)) {
                        if (comp_ptr->op_ == SQL_AST::comparison_operator::EQ) {
                            auto attr1 = get_attribute(comp_ptr->left_);
                            auto attr2 = get_attribute(comp_ptr->right_);
                            if (attr1 && attr2) {
                                if (attr1.value().relation_ == left)
                                    extracted.emplace_back(attr1.value().column_, attr2.value().column_);
                                else
                                    extracted.emplace_back(attr2.value().column_, attr1.value().column_);
                                continue ;
                            }
                        }
                    }
                    remaining.push_back(cond);
                }
                return make_pair(extracted, remaining);
            }

            BaseOperator* build_filter(
                Context const& ctx,
                vector<BaseOperator*> const& forward_ops,
                SQL_AST::select const& s) {
                if (s.or_conditions_.size() == 1 && forward_ops.size() == 2) {
                    Relation r1 = build_relation(s.relations_[0]);
                    Relation r2 = build_relation(s.relations_[1]);
                    auto extracted = extract_join(s.or_conditions_.back(), r1.name(), r2.name());
                    auto const& extracted_join = extracted.first;
                    auto const& remaining = extracted.second;
                    if (!extracted_join.empty()) {
                        BaseOperator* join = new Join(forward_ops[0], forward_ops[1], r1, r2, extracted_join);
                        if (!remaining.empty()) {
                            SQL_AST::or_conditions or_conds; or_conds.push_back(remaining);
                            BaseOperator* filter = new Filter(join, ctx, or_conds);
                            return filter;
                        }
                        return join;
                    }
                }
                BaseOperator* product = new CartesianProduct(forward_ops);
                BaseOperator* filter = new Filter(product, ctx, s.or_conditions_);
                return filter;
            }

            BaseOperator* operator()(Context const& old_ctx, SQL_AST::select const& select) {
                auto forward_ops = build_operators(old_ctx, select.relations_);
                Context ctx = old_ctx;
                extend_from(ctx, select.relations_);
                BaseOperator* next_op = build_filter(ctx, forward_ops, select);
                if (select.order_by_) {
                    next_op = new Sorter(next_op, ctx, select.order_by_.get());
                }
                (*this)(ctx, select.projections_);
                next_op = new Project(next_op, ctx, select.projections_);
                return next_op;
            }

            void operator()(Context const& ctx, SQL_AST::projections const& projections) {
                for (auto const& proj : projections) {
                    (*this)(ctx, proj);
                }
            }

            void operator()(Context const& ctx, SQL_AST::project_rename const& project_rename) {
                (*this)(ctx, project_rename.attribute_);
            }

            void operator()(Context const& ctx, SQL_AST::attribute const& attribute) {
                auto const& relation = ctx[attribute.relation_];
                relation[attribute.column_];
            }
        };
    }

    unique_ptr<BaseOperator> build(SQL_AST::query ast, Context const& ctx) {
        ast = SQL_Optimizer::push_down_select(ast);
        //cout << "[OPT] " << ast << endl;
        QueryBuilder builder;
        auto visitor = std::bind(builder, ctx, std::placeholders::_1);
        return unique_ptr<BaseOperator>(boost::apply_visitor(visitor, ast));
    }
}
