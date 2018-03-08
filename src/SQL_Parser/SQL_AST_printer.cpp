#include <iostream>
#include "SQL_AST.hpp"

using namespace std;
namespace x3 = boost::spirit::x3;

namespace {
    class SQL_AST_Printer {
        ostream& out;
    public:
        SQL_AST_Printer(ostream& out): out(out) {}

        template<typename T>
        void operator()(vector<T> const& t, string const& sep) {
            if (!t.empty())
                (*this)(t[0]);
            for (unsigned int i = 1; i < t.size(); ++i) {
                out << sep;
                (*this)(t[i]);
            }
        }

        void operator()(SQL_AST::query const& q) {
            (*this)(q.select_);
        }

        void operator()(SQL_AST::select const& s) {
            out << "SELECT ";
            (*this)(s.projections_);
            out << " FROM ";
            (*this)(s.relations_);
            out << " WHERE ";
            (*this)(s.or_conditions_);
        }

        void operator()(SQL_AST::projections const& p) {
            (*this)(p.project_rename_, ", ");
        }

        void operator()(SQL_AST::project_rename const& pr) {
            (*this)(pr.attribute_);
            if (pr.rename_) {
                out << " AS ";
                (*this)(pr.rename_.value());
            }
        }

        void operator()(SQL_AST::attribute const& a) {
            (*this)(a.relation_);
            out << ".";
            (*this)(a.column_);
        }

        void operator()(SQL_AST::carthesian_product const& c) {
            (*this)(c.relations_, ", ");
        }

        void operator()(SQL_AST::relation const& r) {
            (*this)(r.filename_);
            out << " ";
            (*this)(r.alias_);
        }

        void operator()(SQL_AST::or_conditions const& oc) {
            (*this)(oc.and_conditions_, " OR ");
        }

        void operator()(SQL_AST::and_conditions const& ac) {
            (*this)(ac.atomic_conditions_, " AND ");
        }

        void operator()(SQL_AST::atomic_condition const& tc) {
            (*this)(tc.left_);
            (*this)(tc.op_);
            (*this)(tc.right_);
        }

        void operator()(SQL_AST::comparison_operator const& op) {
            switch (op) {
                case SQL_AST::comparison_operator::EQ: out << "="; break ;
                case SQL_AST::comparison_operator::NEQ: out << "<>"; break ;
                case SQL_AST::comparison_operator::GT: out << ">"; break ;
                case SQL_AST::comparison_operator::GTE: out << ">="; break ;
                case SQL_AST::comparison_operator::LT: out << "<"; break ;
                case SQL_AST::comparison_operator::LTE: out << "<="; break ;
            }
        }

        void operator()(string const& str) {
            out << str;
        }
    };
}

namespace SQL_AST {
    void print(std::ostream& out, SQL_AST::query const& ast) {
        SQL_AST_Printer p(out);
        p(ast);
    }
}
