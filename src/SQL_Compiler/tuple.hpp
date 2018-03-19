#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <iostream>
#include <string>
#include <vector>

namespace SQL_Compiler {
    class Tuple : public std::vector<std::string> {};
}

std::ostream& operator<<(std::ostream&, SQL_Compiler::Tuple const&);

#endif
