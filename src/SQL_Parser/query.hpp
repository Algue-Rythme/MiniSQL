#ifndef QUERY_HPP
#define QUERY_HPP

#include <boost/spirit/home/x3.hpp>
#include "SQL_AST.hpp"

namespace SQL_Parser {
    struct query_class;
    typedef boost::spirit::x3::rule<query_class, SQL_AST::query> query_type;
    BOOST_SPIRIT_DECLARE(query_type)
    query_type const& get_query();
}

#endif
