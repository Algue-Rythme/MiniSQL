#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include "CSV_reader.hpp"

using namespace std;
namespace x3 = boost::spirit::x3;

namespace SQL_Compiler {
    CSV_ReaderIterator::CSV_ReaderIterator(vector<Tuple> const& tuples_)
        : tuples(tuples_), it(tuples.cbegin()) {}

    Tuple const& CSV_ReaderIterator::dereference() const {
        return *it;
    }

    void CSV_ReaderIterator::increment() {
        ++it;
    }

    bool CSV_ReaderIterator::is_done() const {
        return it == tuples.end();
    }

    void CSV_ReaderIterator::restart() {
        it = tuples.begin();
    }

    CSV_Reader::CSV_Reader(string const& filename_)
        : filename(filename_)
    {
        ifstream file(filename);
        vector<string> lines;
        boost::spirit::istream_iterator iter(file >> noskipws), eof;
        auto const parser = (*(x3::char_ - x3::eol)) % x3::eol;
        x3::parse(iter, eof, parser, lines);
        bool first_line = true;
        for (auto line : lines) {
            if (first_line) {
                first_line = false;
                continue ;
            }
            boost::tokenizer<boost::escaped_list_separator<char>> tok(line);
            Tuple t(tok.begin(), tok.end());
            if (t.empty() || (t.size() == 1 || t.back().empty()))
                continue ;
            tuples.push_back(t);
        }
    }

    OperatorIterator CSV_Reader::begin() const {
        return OperatorIterator(new CSV_ReaderIterator(tuples));
    }
}
