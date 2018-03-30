#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>

#include "SQL_Parser/SQL_parser.hpp"
#include "SQL_Parser/normal_form.hpp"
#include "SQL_Compiler/query_builder.hpp"
#include "SQL_Compiler/operators.hpp"
#include "SQL_Compiler/tuple.hpp"
#include "SQL_Compiler/optimizer.hpp"

#define CONSOLE_MODE

using namespace std;

void print_header(SQL_AST::query const& ast) {
    auto header = SQL_Compiler::get_attributes_names(ast);
    ostringstream ss;
    ss << header;
    string sep(ss.str().size(), '-');
    cout << header << "\n" << sep << endl;
}

inline bool is_blank(string const& str) {
    return all_of(begin(str), end(str), [](char c){
        return isspace(static_cast<unsigned char>(c));
    });
}

bool ask_query(istream& in, string& str, char sep='\n') {
    #ifdef CONSOLE_MODE
    cout << "minisql> ";
    #endif
    bool success = true;
    do {
        success = static_cast<bool>(getline(in, str, sep));
    } while (success && is_blank(str));
    return success;
}

int main(int argc, char * argv[]) {
    ios::sync_with_stdio(false);
    #ifdef CONSOLE_MODE
    cout << "MiniSQL version 1.0 (" << __DATE__ << ", " << __TIME__ << ")";
    cout << " by Louis Bethune" << endl;
    #endif
    string in;
    while(ask_query(cin, in, ';')) {
        try {
            SQL_AST::query ast;
            SQL_Parser::parse(ast, in);
            //cout << ast << endl;
            ast = SQL_Parser::to_normal_form(ast);
            //cout << "[NF] " << ast << endl;
            auto op = SQL_Compiler::build(ast);
            print_header(ast);
            for (auto it = begin(*op); !it.is_done(); ++it) {
                cout << *it << "\n";
            }
        } catch (SQL_Parser::ParsingError const& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Parsing abort." << endl;
        } catch (SQL_Compiler::SemanticError const& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Compilation abort." << endl;
        } catch (SQL_Optimizer::NormalizingError const& e) {
            cout << "[Error] " << e.what() << endl;
            cout << "Optimization abort." << endl;
        }
    }
    cout << endl;
}
