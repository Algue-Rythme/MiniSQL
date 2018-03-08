#include "SQL_parser.hpp"

using namespace std;
namespace x3 = boost::spirit::x3;

namespace SQL_Parser {
    void parse(SQL_AST::query& ast, string const& source) {
        using SQL_Parser::iterator_type;
        using boost::spirit::x3::with;
        using SQL_Parser::error_handler_type;

        iterator_type iter(source.begin());
        iterator_type end(source.end());
        error_handler_type error_handler(iter, end, cout, source.c_str());

        auto const parser =
            with<SQL_Parser::error_handler_tag>(ref(error_handler))
            [
                get_query()
            ];

        using boost::spirit::x3::ascii::space;
        bool success = x3::phrase_parse(iter, end, parser, space, ast);

        if (!success)
            throw Parsing_Error("syntax errors");
        if (iter != end) {
            throw Parsing_Error("expecting end of input here: \"" + string(iter, end) + "\"");
        }
    }
}
