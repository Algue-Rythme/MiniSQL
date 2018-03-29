#ifndef MINUS_OPERATOR_HPP
#define MINUS_OPERATOR_HPP

#include <unordered_set>

#include "base_operator.hpp"

namespace SQL_Compiler {

    class MinusIterator : public PolymorphicIterator {
        OperatorIterator from;
        std::unordered_set<Tuple, TupleHasher> except;
        void skip();
        bool must_be_removed(Tuple const& t) const;
    public:
        MinusIterator(OperatorIterator&&, BaseOperator const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    class MinusOperator: public BaseOperator {
        std::unique_ptr<BaseOperator> from;
        std::unique_ptr<BaseOperator> except;
    public:
        MinusOperator(BaseOperator*, BaseOperator*);
        OperatorIterator begin() const override;
    };
}

#endif
