#ifndef FILTER_OPERATOR_HPP
#define FILTER_OPERATOR_HPP

#include <functional>

#include "../SQL_Parser/SQL_AST.hpp"
#include "base_operator.hpp"

namespace SQL_Compiler {
    typedef std::function<bool (Tuple const&)> TupleFilter;

    class FilterIterator : public PolymorphicIterator {
        OperatorIterator it;
        TupleFilter const& filter;
        void skip();
    public:
        FilterIterator(OperatorIterator&&, TupleFilter const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    class Filter : public BaseOperator {
        std::unique_ptr<BaseOperator> nextOp;
        Context ctx;
        SQL_AST::or_conditions conds;
        TupleFilter filter;
        TupleFilter compile(SQL_AST::atomic_condition const& atomic_cond);
        TupleFilter compile(SQL_AST::and_conditions const& and_conds);
        TupleFilter compile(SQL_AST::or_conditions const& or_conds);
    public:
        Filter(BaseOperator * const nextOp, Context const& ctx, SQL_AST::or_conditions const& cond);
        void compile();
        OperatorIterator begin() const override;
    };
}

#endif
