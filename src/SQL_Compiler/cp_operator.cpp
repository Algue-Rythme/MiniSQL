#include "cp_operator.hpp"

namespace SQL_Compiler {
    CarthesianProductIterator::CarthesianProductIterator(std::vector<std::unique_ptr<BaseOperator>> const& operators)
    : curRunning(0)
    {
        its.reserve(operators.size());
        for (auto const& op : operators) {
            its.emplace_back(op->begin());
        }
    }

    Tuple const& CarthesianProductIterator::dereference() const {
        return *its.at(curRunning);
    }

    void CarthesianProductIterator::increment() {
        if (is_done())
            return ;
        ++its[curRunning];
        while (curRunning < (int)its.size() && its[curRunning.is_done()]) {
            ++curRunning;
        }
        for (int i = 0; i < curRunning; ++i) {
            its[i].reset();
        }
    }

    bool CarthesianProductIterator::is_done() const {
        return curRunning >= its.size();
    }

    void CarthesianProductIterator::reset() {
        curRunning = 0;
        for (auto& it : its) {
            its.reset();
        }
    }

    CarthesianProduct(std::vector<BaseOperator*> const& relations_ptr, Context const& ctx): ctx(ctx) {
        relations.reserve(relations_ptr.size());
        for (auto const& ptr : relations_ptr) {
            relations.emplace_back(ptr);
        }
    }

    OperatorIterator CarthesianProduct::begin() const {
        return CarthesianProductIterator(relations);
    }
};
