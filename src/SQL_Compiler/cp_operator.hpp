#ifndef CARTESIAN_PRODUCT_OPERATOR_HPP
#define CARTESIAN_PRODUCT_OPERATOR_HPP

#include <memory>
#include <vector>

#include "tuple.hpp"
#include "base_operator.hpp"

namespace SQL_Compiler {
    class CartesianProductIterator : public PolymorphicIterator {
        std::vector<OperatorIterator> its;
        Tuple t;
        void build_tuple();
    public:
        CartesianProductIterator(std::vector<std::unique_ptr<BaseOperator>> const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    class CartesianProduct : public BaseOperator {
        std::vector<std::unique_ptr<BaseOperator>> relations;
    public:
        CartesianProduct(std::vector<BaseOperator*> const&);
        OperatorIterator begin() const override;
    };
}

#endif
