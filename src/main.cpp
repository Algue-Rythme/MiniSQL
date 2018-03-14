#include <iostream>
#include <string>
#include <vector>

#include "SQL_Parser/SQL_parser.hpp"
#include "SQL_Compiler/query_builder.hpp"
#include "SQL_Compiler/operators.hpp"

using namespace std;

bool ask_query(istream& in, string& str) {
    #ifdef CONSOLE_MODE
    cout << ">> ";
    #endif
    return static_cast<bool>(getline(in, str));
}

int main() {
    SQL_Compiler::CSV_Reader r("database/homes.csv");
    for (auto it = begin(r); !it.is_done(); ++it) {
        auto t = *it;
        for (auto const& e : t) {
            cout << e << ", ";
        }
        cout << endl;
    }
    /*
    string in;
    while(ask_query(cin, in)) {
        try {
            SQL_AST::query ast;
            SQL_Parser::parse(ast, in); // perform syntax checking
            cout << ast << endl;
            SQL_Compiler::build(ast); // perform a semantic checking
        } catch (SQL_Parser::ParsingError const& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Parsing abort." << endl;
        } catch (const SQL_Compiler::SemanticError& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Compilation abort." << endl;
        }
    }*/
}
