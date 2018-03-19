#include <functional>
#include <vector>
#include <iostream>

#include "../SQL_Parser/SQL_AST.hpp"
#include "filter_operator.hpp"

using namespace std;

namespace SQL_Compiler {
    FilterIterator::FilterIterator(OperatorIterator&& it_, TupleFilter const& f): it(std::move(it_)), filter(f) {
        skip();
    }

    void FilterIterator::skip() {
        while (!it.is_done() && !filter(*it)) {
            ++it;
        }
    }

    Tuple const& FilterIterator::dereference() const {
        return *it;
    }

    void FilterIterator::increment() {
        ++it;
        skip();
    }

    bool FilterIterator::is_done() const {
        return it.is_done();
    }

    void FilterIterator::restart() {
        it.restart();
    }

    namespace {
        typedef function<string const& (Tuple const&)> Accessor;
        struct GetOperand : public boost::static_visitor<Accessor> {
            Context const& ctx;
            GetOperand(Context const& ctx_):ctx(ctx_){}

            Accessor operator()(SQL_AST::attribute const& att) const {
                int ind = ctx[att.relation_][att.column_];
                return [=](Tuple const& t) -> string const& {
                    return t[ind];
                };
            }

            Accessor operator()(string const& str) const {
                return [=](Tuple const&) -> string const& {
                    return str;
                };
            }
        };
    }

    TupleFilter Filter::compile(SQL_AST::atomic_condition const& atomic_cond) {
        auto get_left = boost::apply_visitor(GetOperand(ctx), atomic_cond.left_);
        auto get_right = boost::apply_visitor(GetOperand(ctx), atomic_cond.right_);

        TupleFilter filter;
        switch (atomic_cond.op_) {
            case SQL_AST::comparison_operator::EQ:
                filter = [get_left,get_right](Tuple const& t) -> bool {
                    return get_left(t) == get_right(t);
                };
            break ;
            case SQL_AST::comparison_operator::NEQ:
                filter = [get_left,get_right](Tuple const& t) -> bool {
                    return get_left(t) != get_right(t);
                };
            break ;
            case SQL_AST::comparison_operator::GT:
                filter = [get_left,get_right](Tuple const& t) -> bool {
                    return get_left(t) > get_right(t);
                };
            break ;
            case SQL_AST::comparison_operator::GTE:
                filter = [get_left,get_right](Tuple const& t) -> bool {
                    return get_left(t) >= get_right(t);
                };
            break ;
            case SQL_AST::comparison_operator::LT:
                filter = [get_left,get_right](Tuple const& t) -> bool {
                    return get_left(t) < get_right(t);
                };
            break ;
            case SQL_AST::comparison_operator::LTE:
                filter = [get_left,get_right](Tuple const& t) -> bool {
                    return get_left(t) <= get_right(t);
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
        : nextOp(nextOp), ctx(ctx), conds(conds)
    {
            compile();
    }

    void Filter::compile() {
        filter = compile(conds);
    }

    OperatorIterator Filter::begin() const {
        return OperatorIterator(new FilterIterator(nextOp->begin(), filter));
    }
}
