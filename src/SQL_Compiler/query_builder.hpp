#ifndef QUERY_BUILDER_HPP
#define QUERY_BUILDER_HPP

#include <memory>

#include "../SQL_Parser/SQL_AST.hpp"
#include "context.hpp"
#include "operators.hpp"

namespace SQL_Compiler {
    std::unique_ptr<BaseOperator> build(SQL_AST::query ast, Context const& ctx = Context());
}

#endif
