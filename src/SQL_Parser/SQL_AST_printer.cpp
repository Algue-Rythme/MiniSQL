#include <iostream>
#include "SQL_AST.hpp"

using namespace std;
namespace x3 = boost::spirit::x3;

namespace {
    class SQL_AST_Printer : public boost::static_visitor<> {
        ostream& out;
    public:
        SQL_AST_Printer(ostream& out): out(out) {}

        template<typename T, typename F>
        void operator()(vector<T> const& t, string const& sep, F f) {
            if (!t.empty())
                f(t[0]);
            for (unsigned int i = 1; i < t.size(); ++i) {
                out << sep;
                f(t[i]);
            }
        }

        void operator()(SQL_AST::atomic_condition const& atom) {
            boost::apply_visitor(*this, atom);
        }

        void operator()(SQL_AST::minus_op const& minus_op) {
            out << "(";
            boost::apply_visitor(*this, minus_op.left_);
            out << ") MINUS (";
            boost::apply_visitor(*this, minus_op.right_);
            out << ")";
        }

        void operator()(SQL_AST::union_op const& union_op) {
            out << "(";
            boost::apply_visitor(*this, union_op.left_);
            out << ") UNION (";
            boost::apply_visitor(*this, union_op.right_);
            out << ")";
        }

        void operator()(SQL_AST::select const& s) {
            out << "SELECT ";
            (*this)(s.projections_);
            out << " FROM ";
            (*this)(s.relations_);
            out << " WHERE ";
            (*this)(s.or_conditions_);
            if (s.group_by_) {
                out << " GROUP BY ";
                (*this)(s.group_by_.value(), ", ", *this);
            }
            if (s.order_by_) {
                out << " ORDER BY ";
                (*this)(s.order_by_.value(), ", ", *this);
            }
        }

        void operator()(SQL_AST::projections const& p) {
            (*this)(p, ", ", *this);
        }

        void operator()(SQL_AST::project_rename const& pr) {
            (*this)(pr.attribute_);
            if (pr.rename_) {
                out << " AS ";
                (*this)(pr.rename_.value());
            }
        }

        void operator()(SQL_AST::attribute const& a) {
            out << a.relation_;
            out << ".";
            out << a.column_;
        }

        void operator()(SQL_AST::cartesian_product const& c) {
            auto visitor =
                [this](decltype(c.back()) v){
                    return boost::apply_visitor(*this, v);
                };
            (*this)(c, ", ", visitor);
        }

        void operator()(SQL_AST::load_file const& load_file) {
            (*this)(load_file.filename_);
            out << " " << load_file.alias_;
        }

        void operator()(SQL_AST::subquery const& subquery) {
            out << "(";
            boost::apply_visitor(*this, subquery.query_);
            out << ") ";
            out << subquery.alias_;
        }

        void operator()(SQL_AST::or_conditions const& oc) {
            (*this)(oc, " OR ", *this);
        }

        void operator()(SQL_AST::and_conditions const& ac) {
            (*this)(ac, " AND ", *this);
        }

        void operator()(SQL_AST::comparison_condition const& tc) {
            boost::apply_visitor(*this, tc.left_);
            (*this)(tc.op_);
            boost::apply_visitor(*this, tc.right_);
        }

        void operator()(SQL_AST::in_condition const& ic) {
            (*this)(ic.att_);
            switch (ic.in_type_) {
                case SQL_AST::in_type::IN: out << " IN "; break;
                case SQL_AST::in_type::NOT_IN: out << " NOT IN "; break;
            }
            out << "(";
            boost::apply_visitor(*this, ic.query_);
            out << ")";
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

        void operator()(SQL_AST::order_by_clause c) {
            (*this)(c.att_);
            switch (c.order_type_) {
                case SQL_AST::order_type::ASC:
                out << " ASC";
                break;
                case SQL_AST::order_type::DESC:
                out << " DESC";
                break;
            }
        }

        void operator()(string const& str) {
            out << "\"" << str << "\"";
        }
    };
}

namespace SQL_AST {
    void print(std::ostream& out, SQL_AST::query const& ast) {
        SQL_AST_Printer printer(out);
        boost::apply_visitor(printer, ast);
    }
}
