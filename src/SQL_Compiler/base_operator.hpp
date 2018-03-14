#ifndef BASE_OPERATOR_HPP
#define BASE_OPERATOR_HPP

#include <iterator>
#include <memory>

#include <boost/iterator/iterator_facade.hpp>

#include "context.hpp"
#include "tuple.hpp"

namespace SQL_Compiler {
    template<typename T>
    using IteratorFacadeTuple = boost::iterator_facade<
        T
        , Tuple
        , boost::incrementable_traversal_tag
        , Tuple const&
        , std::ptrdiff_t
    >;

    class PolymorphicIterator : public IteratorFacadeTuple<PolymorphicIterator> {
    public:
        virtual Tuple const& dereference() const = 0;
        virtual void increment() = 0;
        virtual bool is_done() const = 0;
        virtual void reset() = 0;
    };

    class OperatorIterator : public IteratorFacadeTuple<OperatorIterator>
    {
        std::unique_ptr<PolymorphicIterator> it_ptr;
    public:
        template<typename T>
        OperatorIterator(T const& _it): it_ptr(_it) {}
        OperatorIterator(OperatorIterator&& _it);
        Tuple const& dereference() const;
        void increment();
        bool is_done() const;
        void reset();
    };

    class BaseOperator {
    public:
        virtual OperatorIterator begin() const = 0;
    };
}

#endif
