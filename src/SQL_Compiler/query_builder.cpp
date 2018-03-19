#include "query_builder.hpp"
#include "context.hpp"
#include "operators.hpp"

#include <functional>

using namespace std;

namespace SQL_Compiler {
    namespace {
        class QueryBuilder : boost::static_visitor<BaseOperator*> {
        public:
            QueryBuilder() = default;

            BaseOperator* operator()(Context const& ctx, SQL_AST::relation const& relation) {
                return new CSV_Reader(relation.filename_);
            }

            vector<BaseOperator*> build_operators(Context const& ctx, SQL_AST::cartesian_product const& prod) {
                vector<BaseOperator*> operators;
                operators.reserve(prod.relations_.size());
                for (auto const& rel : prod.relations_) {
                    operators.emplace_back((*this)(ctx, rel));
                }
                return operators;
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
                ctx.extend_from(select.relations_);
                BaseOperator* filter = new Filter(product, ctx, select.or_conditions_);
                (*this)(ctx, select.projections_);
                BaseOperator* project = new Project(filter, ctx, select.projections_);
                return project;
            }

            void operator()(Context const& ctx, SQL_AST::projections const& projections) {
                for (auto const& proj : projections.project_rename_) {
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

    unique_ptr<BaseOperator> build(SQL_AST::query const& ast) {
        Context ctx;
        QueryBuilder builder;
        auto visitor = std::bind(builder, ctx, std::placeholders::_1);
        return unique_ptr<BaseOperator>(boost::apply_visitor(visitor, ast));
    }
}
