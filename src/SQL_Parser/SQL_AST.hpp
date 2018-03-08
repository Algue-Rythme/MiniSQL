#ifndef SQL_AST_HPP
#define SQL_AST_HPP

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace SQL_AST {
    namespace x3 = boost::spirit::x3;

    struct nil {};

    struct attribute {
        std::string relation_;
        std::string column_;
    };

    struct project_rename {
        attribute attribute_;
        boost::optional<std::string> rename_;
    };

    struct projections {
        std::vector<project_rename> project_rename_;
    };

    struct relation {
        std::string filename_;
        std::string alias_;
    };

    struct carthesian_product {
        std::vector<relation> relations_;
    };

    enum class comparison_operator {
        EQ
        , NEQ
        , GT
        , GTE
        , LT
        , LTE
    };

    struct atomic_condition {
        SQL_AST::attribute left_;
        comparison_operator op_;
        SQL_AST::attribute right_;
    };

    struct and_conditions {
        std::vector<atomic_condition> atomic_conditions_;
    };

    struct or_conditions {
        std::vector<and_conditions> and_conditions_;
    };

    struct select {
        projections projections_;
        carthesian_product relations_;
        or_conditions or_conditions_;
    };

    struct query {
        select select_;
    };

    inline std::ostream& operator<<(std::ostream& out, nil) {
        out << "nil";
        return out;
    }

    void print(std::ostream& out, SQL_AST::query const& ast);

    inline std::ostream& operator<<(std::ostream& out, SQL_AST::query const& ast) {
        print(out, ast);
        return out;
    }
}

#endif
