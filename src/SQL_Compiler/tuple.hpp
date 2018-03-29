#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <boost/functional/hash.hpp>

namespace SQL_Compiler {
    typedef std::vector<std::string> Tuple;

    class TupleHasher {
    public:
        inline std::size_t operator()(Tuple const& t) const {
            return boost::hash_range(std::begin(t), std::end(t));
        }
    };
}

std::ostream& operator<<(std::ostream&, SQL_Compiler::Tuple const&);

#endif
