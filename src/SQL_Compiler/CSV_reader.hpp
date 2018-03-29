#ifndef CSV_READER_HPP
#define CSV_READER_HPP

#include "base_operator.hpp"

#include <string>
#include <fstream>

namespace SQL_Compiler {
    class CSV_ReaderIterator : public PolymorphicIterator {
        std::vector<Tuple> const& tuples;
        decltype(tuples.cbegin()) it;
    public:
        CSV_ReaderIterator(std::vector<Tuple> const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    class CSV_Reader : public BaseOperator {
        std::string filename;
        std::vector<Tuple> tuples;
    public:
        CSV_Reader(std::string const& filename);
        OperatorIterator begin() const override;
    };
}

#endif
