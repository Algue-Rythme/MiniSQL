#ifndef CSV_READER_HPP
#define CSV_READER_HPP

#include "base_operator.hpp"

#include <string>
#include <fstream>

namespace SQL_Compiler {
    class CSV_ReaderIterator : public PolymorphicIterator {
        std::string filename;
        int line_count;
        std::ifstream file;
        Tuple t;
        void skip_lines(int);
    public:
        CSV_ReaderIterator(std::string const&);
        CSV_ReaderIterator(CSV_ReaderIterator const&);
        Tuple const& dereference() const override;
        void increment() override;
        bool is_done() const override;
        void restart() override;
    };

    class CSV_Reader : public BaseOperator {
        std::string filename;
    public:
        CSV_Reader(std::string const& filename);
        OperatorIterator begin() const override;
    };
}

#endif
