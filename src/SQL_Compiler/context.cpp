#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include "context.hpp"

using namespace std;
namespace x3 = boost::spirit::x3;

namespace SQL_Compiler {

    /*
    * Relation
    */

    Relation::Relation(vector<string> const& names, string const& name, int shift) : m_name(name) {
        for (int att = 0; att < (int)names.size(); ++att) {
            attributes[names[att]] = att + shift;
        }
    }

    int Relation::operator[](string const& attribute) const {
        if (attributes.count(attribute) > 0)
            return attributes.at(attribute);
        throw SemanticError(attribute + " is not an attribute of " + name());
    }

    string const& Relation::name() const {
        return m_name;
    }

    int Relation::nb_attributes() const {
        return attributes.size();
    }

    /*
    * Context
    */

    void Context::add_relation(Relation const& r) {
        if (relations.count(r.name()) > 0)
            throw SemanticError(r.name() + " is already the name of an existing relation");
        relations.insert({r.name(), r});
    }

    Relation const& Context::operator[](string const& name) const {
        if (relations.count(name) == 0)
            throw SemanticError(name + " is not the name of a relation");
        return relations.at(name);
    }

    namespace {
        class GetAttributeNames : public boost::static_visitor<vector<std::string>> {
        public:
            vector<std::string> operator()(SQL_AST::minus_op const& minus_op) {
                auto left = boost::apply_visitor(*this, minus_op.left_);
                auto right = boost::apply_visitor(*this, minus_op.right_);
                if (left != right)
                    throw SemanticError("Attribute names of an UNION must be the same");
                return left;
            }

            vector<std::string> operator()(SQL_AST::union_op const& union_op) {
                auto left = boost::apply_visitor(*this, union_op.left_);
                auto right = boost::apply_visitor(*this, union_op.right_);
                if (left != right)
                    throw SemanticError("Attribute names of a MINUS must be the same");
                return left;
            }

            vector<std::string> operator()(SQL_AST::select const& select) {
                vector<std::string> names;
                for (auto& project : select.projections_) {
                    if (project.rename_) {
                        names.push_back(project.rename_.value());
                    } else {
                        names.push_back(project.attribute_.column_);
                    }
                }
                return names;
            }
        };
    }

    vector<string> get_attributes_names(SQL_AST::query const& q) {
        GetAttributeNames getter;
        return boost::apply_visitor(getter, q);
    }

    vector<string> get_attributes_names(std::string const& filename) {
        vector<std::string> names;
        ifstream file(filename);
        if (!file)
            throw SemanticError("fail to open "+filename);
        boost::spirit::istream_iterator iter(file >> noskipws), eof;
        auto const parser = (+(x3::ascii::alnum)) % (*("," | x3::ascii::blank));
        x3::parse(iter, eof, parser, names);
        return names;
    }

    vector<string> get_attributes_names(SQL_AST::from_relation const& from_rel) {
        if (auto subquery_ptr = boost::get<SQL_AST::subquery>(&from_rel)) {
            auto const& subquery = *subquery_ptr;
            return get_attributes_names(subquery.query_);
        } else if (auto load_file_ptr = boost::get<SQL_AST::load_file>(&from_rel)) {
            auto const& load_file = *load_file_ptr;
            return get_attributes_names(load_file.filename_);
        } else {
            throw runtime_error("[context.cpp][get_attributes_names] Critical Error");
        }
    }

    string get_alias(SQL_AST::from_relation const& rel) {
        return boost::apply_visitor([](auto const& rel) -> string { return rel.alias_;}, rel);
    }

    void extend_from(Context& ctx, SQL_AST::cartesian_product const& relations) {
        int shift = 0;
        for (auto const& sql_relation : relations) {
            vector<std::string> names;
            string alias = get_alias(sql_relation);
            if (auto load_file_ptr = boost::get<SQL_AST::load_file>(&sql_relation)) {
                auto const& filename = load_file_ptr->filename_;
                names = get_attributes_names(filename);
            } else if (auto subquery_ptr = boost::get<SQL_AST::subquery>(&sql_relation)) {
                names = get_attributes_names(subquery_ptr->query_);
            }
            Relation relation(names, alias, shift);
            ctx.add_relation(relation);
            shift += relation.nb_attributes();
        }
    }
}
