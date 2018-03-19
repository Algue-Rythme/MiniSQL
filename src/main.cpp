#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "SQL_Parser/SQL_parser.hpp"
#include "SQL_Compiler/query_builder.hpp"
#include "SQL_Compiler/operators.hpp"
#include "SQL_Compiler/tuple.hpp"

using namespace std;

inline bool is_blank(string const& str) {
    return all_of(begin(str), end(str), [](char c){
        return isspace(static_cast<unsigned char>(c));
    });
}

bool ask_query(istream& in, string& str, char sep='\n') {
    #ifdef CONSOLE_MODE
    cout << ">> ";
    #endif
    bool success = true;
    do {
        success = static_cast<bool>(getline(in, str, sep));
    } while (success && is_blank(str));
    return success;
}

int main(int argc, char * argv[]) {
    string in;
    while(ask_query(cin, in, ';')) {
        try {
            SQL_AST::query ast;
            SQL_Parser::parse(ast, in); // perform syntax checking
            cout << ast << endl;
            auto op = SQL_Compiler::build(ast); // perform a semantic checking
            for (auto it = begin(*op); !it.is_done(); ++it) {
                cout << *it;
            }
        } catch (SQL_Parser::ParsingError const& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Parsing abort." << endl;
        } catch (SQL_Compiler::SemanticError const& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Compilation abort." << endl;
        }
    }
}
