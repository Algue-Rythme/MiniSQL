#ifndef SQL_CONFIG_HPP
#define SQL_CONFIG_HPP

#include <boost/spirit/home/x3.hpp>
#include "SQL_error_handler.hpp"

namespace SQL_Parser {
    typedef std::string::const_iterator iterator_type;
    typedef x3::phrase_parse_context<x3::ascii::space_type>::type phrase_context_type;
    typedef error_handler<iterator_type> error_handler_type;
    typedef x3::with_context<
        error_handler_tag
      , std::reference_wrapper<error_handler_type> const
      , phrase_context_type>::type
    context_type;
}

#endif
