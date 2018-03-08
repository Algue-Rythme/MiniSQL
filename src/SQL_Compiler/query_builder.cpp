#include <exception>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iterator>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include "query_builder.hpp"

using namespace std;

namespace SQL_Compiler {
    namespace {
        class Relation {
            string name;
            unordered_map<string, int> attributes;
        public:
            Relation() = delete;
            Relation(string const& filename) : name(filename) {
                ifstream file(filename);
                if (!file)
                    throw Semantic_Error("fail to open "+filename);
                boost::spirit::istream_iterator iter(file >> noskipws), eof;
                vector<string> names;
                auto const parser = (+(boost::spirit::x3::ascii::alnum)) % (*("," | boost::spirit::x3::ascii::blank));
                boost::spirit::x3::parse(iter, eof, parser, names);
                for (int att = 0; att < (int)names.size(); ++att) {
                    attributes[names[att]] = att;
                }
            }
            Relation(Relation const&) = default;
            Relation& operator=(Relation const&) = default;
            int operator[](string const& attribute) const {
                if (attributes.count(attribute) > 0)
                    return attributes.at(attribute);
                throw Semantic_Error(attribute + " is not an attribute of " + name);
            }
        };

        class Context {
            unordered_map<string, Relation> relations;
        public:
            Context() = default;
            Context(Context const&) = default;
            Context& operator=(Context const&) = default;

            void add_relation(string const& name, Relation const& r) {
                if (relations.count(name) > 0)
                    throw Semantic_Error(name + " is already the name of an existing relation");
                relations.insert({name, r});
            }

            Relation const& operator[](string const& name) const {
                if (relations.count(name) == 0)
                    throw Semantic_Error(name + " is not the name of a relation");
                return relations.at(name);
            }

            void extend_from(SQL_AST::carthesian_product const& relations) {
                for (auto const& sql_relation : relations.relations_) {
                    Relation relation(sql_relation.filename_);
                    add_relation(sql_relation.alias_, relation);
                }
            }
        };

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
