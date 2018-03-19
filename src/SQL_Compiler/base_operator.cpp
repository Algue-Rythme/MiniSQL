#include <stdexcept>

#include "base_operator.hpp"

using namespace std;

namespace SQL_Compiler {
    Tuple const& OperatorIterator::dereference() const {
        if (is_done())
            throw range_error("attempt to dereference an is_done() OperatorIterator");
        return **it_ptr;
    }

    void OperatorIterator::increment() {
        if (is_done())
            throw range_error("attempt to increment an is_done() OperatorIterator");
        ++(*it_ptr);
    }

    bool OperatorIterator::is_done() const {
        return it_ptr->is_done();
    }

    void OperatorIterator::restart() {
        it_ptr->restart();
    }

    OperatorIterator::OperatorIterator(OperatorIterator&& _it) : it_ptr(std::move(_it.it_ptr)) {}
}
