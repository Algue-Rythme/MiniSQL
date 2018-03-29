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

        struct ConditionCompiler : public boost::static_visitor<TupleFilter> {
            Context const& ctx;
            ConditionCompiler(Context const& ctx_):ctx(ctx_){}

            TupleFilter operator()(SQL_AST::in_condition const&) const {
                throw SemanticError("\"WHERE ... IN (...)\" clause not supported yet");
            }

            TupleFilter operator()(SQL_AST::comparison_condition const& comparison_cond) const {
                auto get_left = boost::apply_visitor(GetOperand(ctx), comparison_cond.left_);
                auto get_right = boost::apply_visitor(GetOperand(ctx), comparison_cond.right_);

                TupleFilter filter;
                switch (comparison_cond.op_) {
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
                            return stoi(get_left(t)) > stoi(get_right(t));
                        };
                    break ;
                    case SQL_AST::comparison_operator::GTE:
                        filter = [get_left,get_right](Tuple const& t) -> bool {
                            return stoi(get_left(t)) >= stoi(get_right(t));
                        };
                    break ;
                    case SQL_AST::comparison_operator::LT:
                        filter = [get_left,get_right](Tuple const& t) -> bool {
                            return stoi(get_left(t)) < stoi(get_right(t));
                        };
                    break ;
                    case SQL_AST::comparison_operator::LTE:
                        filter = [get_left,get_right](Tuple const& t) -> bool {
                            return stoi(get_left(t)) <= stoi(get_right(t));
                        };
                    break ;
                }
                return filter;
            }

            TupleFilter operator()(SQL_AST::and_conditions const& and_conds) const {
                vector<TupleFilter> atomic_conds;
                for (auto const& atomic_cond : and_conds) {
                    atomic_conds.push_back(boost::apply_visitor(*this, atomic_cond));
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

            TupleFilter operator()(SQL_AST::or_conditions const& or_conds) const {
                vector<TupleFilter> and_conds;
                for (auto const& and_cond : or_conds) {
                    and_conds.push_back((*this)(and_cond));
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
        };
    }

    Filter::Filter(BaseOperator * const nextOp_, Context const& ctx, SQL_AST::or_conditions const& conds)
        : nextOp(nextOp_), ctx(ctx), conds(conds)
    {
        compile();
    }

    void Filter::compile() {
        ConditionCompiler comp(ctx);
        filter = comp(conds);
    }

    OperatorIterator Filter::begin() const {
        return OperatorIterator(new FilterIterator(nextOp->begin(), filter));
    }
}
