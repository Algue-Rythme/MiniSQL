#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include "context.hpp"

using namespace std;

namespace SQL_Compiler {

    /*
    * Relation
    */

    Relation::Relation(string const& filename, string const& name) : m_name(name) {
        ifstream file(filename);
        if (!file)
            throw SemanticError("fail to open "+filename);
        boost::spirit::istream_iterator iter(file >> noskipws), eof;
        vector<string> names;
        auto const parser = (+(boost::spirit::x3::ascii::alnum)) % (*("," | boost::spirit::x3::ascii::blank));
        boost::spirit::x3::parse(iter, eof, parser, names);
        for (int att = 0; att < (int)names.size(); ++att) {
            attributes[names[att]] = att;
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

    void Context::extend_from(SQL_AST::carthesian_product const& relations) {
        for (auto const& sql_relation : relations.relations_) {
            Relation relation(sql_relation.filename_, sql_relation.alias_);
            add_relation(relation);
        }
    }
}
