#include "union_operator.hpp"

namespace SQL_Compiler {
    UnionIterator::UnionIterator(OperatorIterator&& left_, OperatorIterator&& right_)
        : left(std::move(left_)), right(std::move(right_)) {}

    Tuple const& UnionIterator::dereference() const {
        if (!left.is_done())
            return *left;
        return *right;
    }

    void UnionIterator::increment() {
        if (!left.is_done())
            ++left;
        else
            ++right;
    }

    bool UnionIterator::is_done() const {
        return right.is_done();
    }

    void UnionIterator::restart() {
        left.restart();
        right.restart();
    }

    UnionOperator::UnionOperator(BaseOperator* left_, BaseOperator* right_):left(left_),right(right_){}

    OperatorIterator UnionOperator::begin() const {
        return OperatorIterator(new UnionIterator(left->begin(), right->begin()));
    }
}
