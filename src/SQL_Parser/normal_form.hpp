#ifndef NORMAL_FORM_HPP
#define NORMAL_FORM_HPP

#include "SQL_AST.hpp"

namespace SQL_Parser {
    SQL_AST::query to_normal_form(SQL_AST::query const&);
}

#endif
