#include "cp_operator.hpp"

namespace SQL_Compiler {
    CartesianProductIterator::CartesianProductIterator(std::vector<std::unique_ptr<BaseOperator>> const& operators) {
        its.reserve(operators.size());
        for (auto const& op : operators) {
            its.emplace_back(op->begin());
        }
        build_tuple();
    }

    void CartesianProductIterator::build_tuple() {
        t.clear();
        if (is_done())
            return ;
        for (auto const& it : its) {
            copy(begin(*it), end(*it), back_inserter(t));
        }
    }

    Tuple const& CartesianProductIterator::dereference() const {
        return t;
    }

    void CartesianProductIterator::increment() {
        unsigned int curRunning = 0;
        ++its[curRunning];
        while (its[curRunning].is_done()) {
            ++curRunning;
            if (curRunning >= its.size())
                break ;
            ++its[curRunning];
        }
        if (is_done())
            return ;
        for (unsigned int i = 0; i < curRunning; ++i) {
            its[i].restart();
        }
        build_tuple();
    }

    bool CartesianProductIterator::is_done() const {
        return its.back().is_done();
    }

    void CartesianProductIterator::restart() {
        for (auto& it : its) {
            it.restart();
        }
    }

    CartesianProduct::CartesianProduct(std::vector<BaseOperator*> const& relations_ptr) {
        relations.reserve(relations_ptr.size());
        for (auto const& ptr : relations_ptr) {
            relations.emplace_back(ptr);
        }
    }

    OperatorIterator CartesianProduct::begin() const {
        return OperatorIterator(new CartesianProductIterator(relations));
    }
}
