#ifndef PROJECT_OPERATOR_HPP
#define PROJECT_OPERATOR_HPP

#include <functional>

#include "base_operator.hpp"

namespace SQL_Compiler {
    typedef std::function<Tuple (Tuple const&)> Projecter;

    class ProjectIterator : public PolymorphicIterator {
        OperatorIterator it;
        Projecter const& projecter;
        Tuple t;
    public:
        ProjectIterator(OperatorIterator&&, Projecter const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    class Project : public BaseOperator {
        std::unique_ptr<BaseOperator> nextOp;
        Context ctx;
        Projecter projecter;
        SQL_AST::projections projections;
    public:
        Project(BaseOperator * const, Context const& ctx, SQL_AST::projections const&);
        OperatorIterator begin() const override;
        void compile();
    };
}

#endif
