#ifndef SORTER_OPERATOR_HPP
#define SORTER_OPERATOR_HPP

#include <functional>

#include "base_operator.hpp"

namespace SQL_Compiler {
    typedef std::function<std::vector<int> (Tuple const&)> OrderByProjecter;

    class SorterIterator : public PolymorphicIterator {
        std::vector<Tuple> tuples;
        decltype(tuples.begin()) it_s;
    public:
        SorterIterator(OperatorIterator&&, OrderByProjecter const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    class Sorter : public BaseOperator {
        std::unique_ptr<BaseOperator> nextOp;
        Context ctx;
        SQL_AST::order_by order_by;
        OrderByProjecter projecter;
    public:
        Sorter(BaseOperator * const nextOp, Context const& ctx, SQL_AST::order_by const& order_by);
        OperatorIterator begin() const override;
        void compile();
    };
}

#endif
