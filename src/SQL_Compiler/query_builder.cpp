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

            BaseOperator* operator()(Context const& old_ctx, SQL_AST::select const& select) {
                auto forward_ops = build_operators(old_ctx, select.relations_);
                BaseOperator* product = new CartesianProduct(forward_ops);
                Context ctx = old_ctx;
                extend_from(ctx, select.relations_);
                BaseOperator* filter = new Filter(product, ctx, select.or_conditions_);
                BaseOperator* next_op = filter;
                if (select.order_by_) {
                    BaseOperator* order_by = new Sorter(filter, ctx, select.order_by_.get());
                    next_op = order_by;
                }
                (*this)(ctx, select.projections_);
                BaseOperator* project = new Project(next_op, ctx, select.projections_);
                return project;
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
        // cout << "[OPT] " << ast << endl;
        QueryBuilder builder;
        auto visitor = std::bind(builder, ctx, std::placeholders::_1);
        return unique_ptr<BaseOperator>(boost::apply_visitor(visitor, ast));
    }
}
