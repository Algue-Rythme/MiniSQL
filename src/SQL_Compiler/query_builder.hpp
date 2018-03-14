#ifndef QUERY_BUILDER_HPP
#define QUERY_BUILDER_HPP

#include "../SQL_Parser/SQL_AST.hpp"
#include "context.hpp"

namespace SQL_Compiler {
    void build(SQL_AST::query const& ast);
}

#endif
