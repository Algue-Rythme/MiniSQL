#include "sorter_operator.hpp"

#include <algorithm>
#include <iostream>

using namespace std;

namespace SQL_Compiler {
    namespace {
        struct Sortable {
            Sortable(unsigned int index_, std::vector<int> const& data_) : index(index_), data(data_) {}
            unsigned int index;
            friend bool operator<(Sortable const&, Sortable const&);
        private:
            vector<int> data;
        };

        bool operator<(Sortable const& s1, Sortable const& s2) {
            return s1.data < s2.data;
        }
    }

    SorterIterator::SorterIterator(OperatorIterator&& it, OrderByProjecter const& projecter)
    {
        vector<Sortable> sortables;
        for (unsigned int index = 0; !it.is_done(); ++it, ++index) {
            sortables.emplace_back(index, projecter(*it));
        }
        sort(begin(sortables), end(sortables));
        vector<unsigned int> dest(sortables.size());
        for (unsigned int i = 0; i < sortables.size(); ++i) {
            dest[sortables[i].index] = i;
        }
        tuples.resize(sortables.size());
        it.restart();
        for (unsigned int index = 0; !it.is_done(); ++it, ++index) {
            tuples[dest[index]] = *it;
        }
        restart();
    }

    Tuple const& SorterIterator::dereference() const {
        return *it_s;
    }

    void SorterIterator::increment() {
        ++it_s;
    }

    bool SorterIterator::is_done() const {
        return it_s == end(tuples);
    }

    void SorterIterator::restart() {
        it_s = tuples.begin();
    }

    Sorter::Sorter(BaseOperator * const nextOp, Context const& ctx_, SQL_AST::order_by const& order_by_)
        : nextOp(nextOp), ctx(ctx_), order_by(order_by_)
    {
        compile();
    }

    void Sorter::compile() {
        vector<unsigned int> indices;
        vector<bool> is_asc;
        for (unsigned int i = 0; i < order_by.size(); ++i) {
            auto const& att = order_by[i].att_;
            auto const& order_type = order_by[i].order_type_;
            indices.push_back(ctx[att.relation_][att.column_]);
            is_asc.push_back(order_type == SQL_AST::order_type::ASC);
        }
        projecter = [indices,is_asc](Tuple const& t) {
            vector<int> values(indices.size());
            for (unsigned int i = 0; i < indices.size(); ++i) {
                int v = stoi(t[indices[i]]);
                values[i] = is_asc[i] ? v : -v;
            }
            return values;
        };
    }

    OperatorIterator Sorter::begin() const {
        return OperatorIterator(new SorterIterator(nextOp->begin(), projecter));
    }
}
