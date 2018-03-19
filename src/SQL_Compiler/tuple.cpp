#include <iterator>

#include <boost/spirit/include/karma.hpp>

#include "tuple.hpp"

using namespace std;

ostream& operator<<(ostream& out, SQL_Compiler::Tuple const& t) {
    namespace karma = boost::spirit::karma;;
    std::ostream_iterator<char> it(out);
    karma::generate(it, (karma::left_align(16)[karma::string] % "|") << karma::eol, t);
    return out;
}
