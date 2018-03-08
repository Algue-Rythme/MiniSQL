#include "query_def.hpp"
#include "SQL_config.hpp"

namespace SQL_Parser {
    BOOST_SPIRIT_INSTANTIATE(query_type, iterator_type, context_type)
}
