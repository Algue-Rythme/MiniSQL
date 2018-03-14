#ifndef SQL_PARSER_HPP
#define SQL_PARSER_HPP

#include <string>
#include <stdexcept>

#include "SQL_AST.hpp"
#include "query.hpp"
#include "SQL_error_handler.hpp"
#include "SQL_config.hpp"

namespace SQL_Parser{
    class ParsingError : public std::runtime_error {
    public:
        ParsingError(std::string const& msg):runtime_error(msg.c_str()){}
    };
    void parse(SQL_AST::query& ast, std::string const& source);
}

#endif
