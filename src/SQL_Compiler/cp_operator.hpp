#ifndef CARTHESIAN_PRODUCT_OPERATOR_HPP
#define CARTHESIAN_PRODUCT_OPERATOR_HPP

#include <memory>
#include <vector>

#include "tuple.hpp"
#include "base_operator.hpp"

namespace SQL_Compiler {
    class CarthesianProductIterator : public PolymorphicIterator {
        std::vector<OperatorIterator> its;
        int curRunning;
    public:
        CarthesianProductIterator(std::vector<std::unique_ptr<BaseOperator>> const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void reset() override;
    };

    class CarthesianProduct : public BaseOperator {
        std::vector<std::unique_ptr<BaseOperator>> relations;
        Context ctx;
    public:
        CarthesianProduct(std::vector<BaseOperator*> const&, Context const&);
        OperatorIterator begin() const override;
    };
}

#endif
