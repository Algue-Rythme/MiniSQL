#include "minus_operator.hpp"

using namespace std;

namespace SQL_Compiler {
    MinusIterator::MinusIterator(OperatorIterator&& from_, BaseOperator const& except_)
        : from(move(from_))
    {
        for (auto it = begin(except_); !it.is_done(); ++it) {
            except.insert(*it);
        }
        skip();
    }

    bool MinusIterator::must_be_removed(Tuple const& t) const {
        return except.count(t) != 0;
    }

    void MinusIterator::skip() {
        while (!from.is_done() && must_be_removed(*from)) {
            ++from;
        }
    }

    Tuple const& MinusIterator::dereference() const {
        return *from;
    }

    void MinusIterator::increment() {
        ++from;
        skip();
    }

    bool MinusIterator::is_done() const {
        return from.is_done();
    }

    void MinusIterator::restart() {
        from.restart();
    }

    MinusOperator::MinusOperator(BaseOperator* from_, BaseOperator* except_):from(from_), except(except_) {}

    OperatorIterator MinusOperator::begin() const {
        return OperatorIterator(new MinusIterator(from->begin(), *except));
    }
}
