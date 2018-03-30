#include "join_operator.hpp"

#include <algorithm>
#include <iostream>

using namespace std;

namespace SQL_Compiler {
    JoinIterator::JoinIterator(
        OperatorIterator&& from_left,
        OperatorIterator&& from_right,
        Joiner const& joiner_left_,
        Joiner const& joiner_right_)
    : joiner_left(joiner_left_), joiner_right(joiner_right_)
    {
        fill_map(left, joiner_left, move(from_left));
        fill_map(right, joiner_right, move(from_right));
        restart();
    }

    void JoinIterator::fill_map(
        JoinMapper& target,
        Joiner const& joiner,
        OperatorIterator&& from) {
        for (; !from.is_done(); ++from) {
            Tuple t = joiner(*from);
            target[t].push_back(*from);
        }
    }

    void JoinIterator::match_buckets() {
        if (is_done())
            return ;
        bucket_right = right.find(bucket_left->first);
        while (bucket_right == end(right)) {
            ++bucket_left;
            if (is_done())
                return ;
            bucket_right = right.find(bucket_left->first);
        }
        it_left = begin(bucket_left->second);
        it_right = begin(bucket_right->second);
    }

    Tuple const& JoinIterator::dereference() const {
        return cache;
    }

    void JoinIterator::merge_left_right() {
        cache = *it_left;
        copy(begin(*it_right), end(*it_right), back_inserter(cache));
    }

    void JoinIterator::increment() {
        ++it_right;
        if (it_right == end(bucket_right->second)) {
            it_right = begin(bucket_right->second);
            ++it_left;
            if (it_left == end(bucket_left->second)) {
                ++bucket_left;
                match_buckets();
                if (is_done())
                    return ;
            }
        }
        merge_left_right();
    }

    bool JoinIterator::is_done() const {
        return bucket_left == end(left);
    }

    void JoinIterator::restart() {
        bucket_left = begin(left);
        match_buckets();
        merge_left_right();
    }

    Join::Join(
        BaseOperator * const left_,
        BaseOperator * const right_,
        Relation const& rleft_,
        Relation const& rright_,
        std::vector<JoinKey> const& keys_
    ) : left(left_), right(right_), rleft(rleft_), rright(rright_), keys(keys_)
    {
        compile();
    }

    Joiner Join::compile(Relation const& rel, vector<string> const& names) {
        vector<int> indices;
        for (auto const& name : names) {
            indices.push_back(rel[name]);
        }
        return [indices](Tuple const& source){
            Tuple t; t.reserve(indices.size());
            for (auto const& i : indices) {
                t.push_back(source[i]);
            }
            return t;
        };
    }

    void Join::compile() {
        vector<string> names_left;
        vector<string> names_right;
        for (auto const& key : keys) {
            names_left.push_back(key.first);
            names_right.push_back(key.second);
        }
        joiner_left = compile(rleft, names_left);
        joiner_right = compile(rright, names_right);
    }

    OperatorIterator Join::begin() const {
        return OperatorIterator(new JoinIterator(left->begin(), right->begin(), joiner_left, joiner_right));
    }
}
