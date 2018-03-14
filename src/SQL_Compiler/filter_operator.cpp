#include <functional>
#include <vector>

#include "../SQL_Parser/SQL_AST.hpp"
#include "filter_operator.hpp"

using namespace std;

namespace SQL_Compiler {
    FilterIterator::FilterIterator(OperatorIterator&& it, TupleFilter const& f): it(std::move(it)), filter(f) {}

    Tuple const& FilterIterator::dereference() const {
        return *it;
    }

    void FilterIterator::increment() {
        do {
            ++it;
        } while (!it.is_done() && !filter(*it));
    }

    bool FilterIterator::is_done() const {
        return it.is_done();
    }

    void FilterIterator::reset() {
        it.reset();
    }

    TupleFilter Filter::compile(SQL_AST::atomic_condition const& atomic_cond) {
        auto const& left = atomic_cond.left_;
        auto const& right = atomic_cond.right_;
        int ind1 = ctx[left.relation_][left.column_];
        int ind2 = ctx[right.relation_][right.column_];
        TupleFilter filter;
        switch (atomic_cond.op_) {
            case SQL_AST::comparison_operator::EQ:
                filter = [ind1,ind2](Tuple const& t) -> bool {
                    return t[ind1] == t[ind2];
                };
            break ;
            case SQL_AST::comparison_operator::NEQ:
                filter = [ind1,ind2](Tuple const& t) -> bool {
                    return t[ind1] != t[ind2];
                };
            break ;
            case SQL_AST::comparison_operator::GT:
                filter = [ind1,ind2](Tuple const& t) -> bool {
                    return t[ind1] > t[ind2];
                };
            break ;
            case SQL_AST::comparison_operator::GTE:
                filter = [ind1,ind2](Tuple const& t) -> bool {
                    return t[ind1] >= t[ind2];
                };
            break ;
            case SQL_AST::comparison_operator::LT:
                filter = [ind1,ind2](Tuple const& t) -> bool {
                    return t[ind1] < t[ind2];
                };
            break ;
            case SQL_AST::comparison_operator::LTE:
                filter = [ind1,ind2](Tuple const& t) -> bool {
                    return t[ind1] <= t[ind2];
                };
            break ;
        }
        return filter;
    }

    TupleFilter Filter::compile(SQL_AST::and_conditions const& and_conds) {
        vector<TupleFilter> atomic_conds;
        for (auto const& atomic_cond : and_conds.atomic_conditions_) {
            atomic_conds.emplace_back(compile(atomic_cond));
        }
        return [atomic_conds](Tuple const& t) -> bool {
            for (auto const& atomic_cond : atomic_conds) {
                if (!atomic_cond(t)) {
                    return false;
                }
            }
            return true;
        };
    }

    TupleFilter Filter::compile(SQL_AST::or_conditions const& or_conds) {
        vector<TupleFilter> and_conds;
        for (auto const& and_cond : or_conds.and_conditions_) {
            and_conds.emplace_back(compile(and_cond));
        }
        return [and_conds](Tuple const& t) -> bool {
            for (auto const& and_cond : and_conds) {
                if (and_cond(t)) {
                    return true;
                }
            }
            return false;
        };
    }

    Filter::Filter(BaseOperator * const nextOp, Context const& ctx, SQL_AST::or_conditions const& conds)
        : nextOp(nextOp), ctx(ctx), conds(conds) {}

    void Filter::compile() {
        filter = compile(conds);
    }

    OperatorIterator Filter::begin() const {
        return OperatorIterator(new FilterIterator(nextOp->begin(), filter));
    }
}
