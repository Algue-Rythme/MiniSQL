#include <iostream>
#include <boost/tokenizer.hpp>

#include "CSV_reader.hpp"

using namespace std;

namespace SQL_Compiler {
    CSV_ReaderIterator::CSV_ReaderIterator(string const& _filename)
        : filename(_filename), line_count(0), file(filename) {
        skip_lines(1);
        increment();
    }

    CSV_ReaderIterator::CSV_ReaderIterator(CSV_ReaderIterator const& other)
        : filename(other.filename), line_count(0), file(filename) {
        clog << "Warning : expansive copy of ifstream" << endl;
        skip_lines(other.line_count - 1);
        increment();
    }

    void CSV_ReaderIterator::skip_lines(int nb_skip) {
        string line;
        for (int i = 0; i < nb_skip; ++i) {
            getline(file, line);
            line_count += 1;
        }
    }

    Tuple const& CSV_ReaderIterator::dereference() const {
        return t;
    }

    void CSV_ReaderIterator::increment() {
        string line;
        getline(file, line);
        boost::tokenizer<boost::escaped_list_separator<char>> tok(line);
        t.assign(begin(tok), end(tok));
        line_count += 1;
    }

    bool CSV_ReaderIterator::is_done() const {
        return t.empty();
    }

    void CSV_ReaderIterator::restart() {
        line_count = 0;
        file.close();
        file.open(filename);
        skip_lines(1);
        increment();
    }

    CSV_Reader::CSV_Reader(std::string const& filename)
        : filename(filename) {}

    OperatorIterator CSV_Reader::begin() const {
        return OperatorIterator(new CSV_ReaderIterator(filename));
    }
}
