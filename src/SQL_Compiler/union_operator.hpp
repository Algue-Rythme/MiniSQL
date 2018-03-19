#ifndef UNION_OPERATOR_HPP
#define UNION_OPERATOR_HPP

#include "base_operator.hpp"

namespace SQL_Compiler {
    class UnionIterator : public PolymorphicIterator {
        OperatorIterator left;
        OperatorIterator right;
    public:
        UnionIterator(OperatorIterator&&, OperatorIterator&&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    class UnionOperator: public BaseOperator {
        std::unique_ptr<BaseOperator> left;
        std::unique_ptr<BaseOperator> right;
    public:
        UnionOperator(BaseOperator*, BaseOperator*);
        OperatorIterator begin() const override;
    };
}

#endif
