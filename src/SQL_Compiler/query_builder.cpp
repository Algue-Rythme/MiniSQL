#include "query_builder.hpp"
#include "context.hpp"

using namespace std;

namespace SQL_Compiler {
    namespace {
        class QueryBuilder {
        public:
            QueryBuilder() = default;

            void visit(SQL_AST::query const& ast) {
                Context ctx;
                visit(ctx, ast.select_);
            }

            void visit(Context const& old_ctx, SQL_AST::select const& select) {
                Context ctx = old_ctx;
                ctx.extend_from(select.relations_);
                visit(ctx, select.or_conditions_);
                visit(ctx, select.projections_);
            }

            template<typename T>
            void visit(Context const& ctx, vector<T> const& t) {
                for (auto const& e : t) {
                    visit(ctx, e);
                }
            }

            void visit(Context const& ctx, SQL_AST::projections const& projections) {
                visit(ctx, projections.project_rename_);
            }

            void visit(Context const& ctx, SQL_AST::project_rename const& project_rename) {
                visit(ctx, project_rename.attribute_);
            }

            void visit(Context const& ctx, SQL_AST::or_conditions const& or_conditions) {
                visit(ctx, or_conditions.and_conditions_);
            }

            void visit(Context const& ctx, SQL_AST::and_conditions const& and_conditions) {
                visit(ctx, and_conditions.atomic_conditions_);
            }

            void visit(Context const& ctx, SQL_AST::atomic_condition const& atomic_condition) {
                visit(ctx, atomic_condition.left_);
                visit(ctx, atomic_condition.right_);
            }

            void visit(Context const& ctx, SQL_AST::attribute const& attribute) {
                auto const& relation = ctx[attribute.relation_];
                relation[attribute.column_];
            }
        };
    }

    void build(SQL_AST::query const& ast) {
        QueryBuilder builder;
        builder.visit(ast);
    }
}
