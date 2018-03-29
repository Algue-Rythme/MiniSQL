#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <stdexcept>

#include "../SQL_Parser/SQL_AST.hpp"

namespace SQL_Optimizer {
    class NormalizingError : public std::runtime_error {
    public:
        NormalizingError(std::string const& msg):runtime_error(msg.c_str()){}
    };

    SQL_AST::query push_down_select(SQL_AST::query const&);
}

#endif
