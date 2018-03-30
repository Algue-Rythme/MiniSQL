#ifndef JOIN_OPERATOR_HPP
#define JOIN_OPERATOR_HPP

#include <functional>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "base_operator.hpp"

namespace SQL_Compiler {
    typedef std::function<Tuple (Tuple const&)> Joiner;
    typedef std::unordered_map<Tuple, std::vector<Tuple>,TupleHasher> JoinMapper;
    class JoinIterator : public PolymorphicIterator {
        JoinMapper left;
        JoinMapper right;
        decltype(left.begin()) bucket_left;
        decltype(right.begin()) bucket_right;
        decltype(bucket_left->second.begin()) it_left;
        decltype(bucket_right->second.begin()) it_right;
        Joiner const& joiner_left;
        Joiner const& joiner_right;
        Tuple cache;
        void match_buckets();
        void fill_map(JoinMapper&, Joiner const&, OperatorIterator&&);
        void merge_left_right();
    public:
        JoinIterator(OperatorIterator&&, OperatorIterator&&, Joiner const&, Joiner const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    typedef std::pair<std::string,std::string> JoinKey;
    class Join : public BaseOperator {
        std::unique_ptr<BaseOperator> left;
        std::unique_ptr<BaseOperator> right;
        Relation rleft;
        Relation rright;
        std::vector<JoinKey> keys;
        Joiner joiner_left;
        Joiner joiner_right;
        Joiner compile(Relation const& rel, std::vector<std::string> const& names);
    public:
        Join(
            BaseOperator * const,
            BaseOperator * const,
            Relation const&,
            Relation const&,
            std::vector<JoinKey> const&
        );
        void compile();
        OperatorIterator begin() const override;
    };
}

#endif
