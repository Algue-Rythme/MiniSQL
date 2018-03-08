#include <iostream>
#include <string>
#include <vector>

#include "SQL_Parser/SQL_parser.hpp"
#include "SQL_Compiler/query_builder.hpp"

using namespace std;

bool ask_query(istream& in, string& str) {
    #ifdef CONSOLE_MODE
    cout << ">> ";
    #endif
    return static_cast<bool>(getline(in, str));
}

int main() {
    string in;
    while(ask_query(cin, in)) {
        try {
            SQL_AST::query ast;
            SQL_Parser::parse(ast, in);
            cout << ast << endl;
            SQL_Compiler::build(ast);
        } catch (SQL_Parser::Parsing_Error const& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Parsing abort." << endl;
        } catch (const SQL_Compiler::Semantic_Error& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Compilation abort." << endl;
        }
    }
}
