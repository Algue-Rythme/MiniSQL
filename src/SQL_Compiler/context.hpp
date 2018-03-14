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
        Relation(std::string const& filename, std::string const& name);
        int operator[](std::string const& attribute) const;
        std::string const& name() const;
    };

    class Context {
        std::unordered_map<std::string, Relation> relations;
    public:
        void add_relation(Relation const& r);
        Relation const& operator[](std::string const& name) const;
        void extend_from(SQL_AST::carthesian_product const& relations);
    };
}

#endif
