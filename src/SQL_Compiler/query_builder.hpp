#ifndef QUERY_BUILDER_HPP
#define QUERY_BUILDER_HPP

#include <iostream>
#include <stdexcept>

#include "../SQL_Parser/SQL_AST.hpp"

namespace SQL_Compiler {

    class Semantic_Error : public std::runtime_error {
    public:
        Semantic_Error(std::string const& msg):runtime_error(msg.c_str()){}
    };

    void build(SQL_AST::query const& ast);
}

#endif
