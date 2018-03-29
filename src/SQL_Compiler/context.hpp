#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iterator>

#include "../SQL_Parser/SQL_AST.hpp"

namespace SQL_Compiler {
    class SemanticError : public std::runtime_error {
    public:
        SemanticError(std::string const& msg):runtime_error(msg.c_str()){}
    };

    class Relation {
        std::string m_name;
        std::unordered_map<std::string, int> attributes;
    public:
        Relation(std::vector<std::string> const& names, std::string const& name, int shift = 0);
        int operator[](std::string const& attribute) const;
        std::string const& name() const;
        int nb_attributes() const;
    };

    class Context {
        std::unordered_map<std::string, Relation> relations;
    public:
        void add_relation(Relation const& r);
        Relation const& operator[](std::string const& name) const;
    };

    std::vector<std::string> get_attributes_names(SQL_AST::query const& q);
    std::vector<std::string> get_attributes_names(std::string const& filename);
    std::vector<std::string> get_attributes_names(SQL_AST::from_relation const& from_rel);
    std::string get_alias(SQL_AST::from_relation const& rel);

    void extend_from(Context& ctx, SQL_AST::cartesian_product const& relations);
}

#endif
