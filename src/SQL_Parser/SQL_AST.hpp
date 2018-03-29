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

    struct select;
    struct union_op;
    struct minus_op;

    struct query : x3::variant
        < x3::forward_ast<select>
        , x3::forward_ast<union_op>
        , x3::forward_ast<minus_op>
    > {
        using base_type::base_type;
        using base_type::operator=;
    };

    struct attribute {
        std::string relation_;
        std::string column_;
    };

    struct project_rename {
        attribute attribute_;
        boost::optional<std::string> rename_;
    };

    struct projections : public std::vector<project_rename> {};

    struct load_file {
        std::string filename_;
        std::string alias_;
    };

    struct subquery {
        query query_;
        std::string alias_;
    };

    struct from_relation : x3::variant<load_file, subquery>  {
        from_relation() = default;
        from_relation(from_relation const&) = default;
        from_relation& operator=(from_relation const&) = default;
        using base_type::base_type;
        using base_type::operator=;
    };

    struct cartesian_product : public std::vector<from_relation> {};

    struct operand : x3::variant
        < attribute
        , std::string
    > {
        using base_type::base_type;
        using base_type::operator=;
    };

    enum class comparison_operator {
        EQ
        , NEQ
        , GT
        , GTE
        , LT
        , LTE
    };

    struct comparison_condition {
        operand left_;
        comparison_operator op_;
        operand right_;
    };

    enum class in_type {
        IN,
        NOT_IN
    };

    struct in_condition {
        attribute att_;
        in_type in_type_;
        query query_;
    };

    struct atomic_condition : x3::variant
        < comparison_condition
        , in_condition
    > {
        atomic_condition() = default;
        atomic_condition(atomic_condition const&) = default;
        atomic_condition& operator=(atomic_condition const&) = default;
        using base_type::base_type;
        using base_type::operator=;
    };

    struct and_conditions : public std::vector<atomic_condition> {};
    struct or_conditions : public std::vector<and_conditions> {};

    struct group_by : public std::vector<attribute> {};

    enum class order_type {
        ASC,
        DESC
    };

    struct order_by_clause {
        attribute att_;
        order_type order_type_;
    };

    struct order_by : public std::vector<order_by_clause> {};

    struct select {
        projections projections_;
        cartesian_product relations_;
        or_conditions or_conditions_;
        boost::optional<group_by> group_by_;
        boost::optional<order_by> order_by_;
    };

    struct union_op {
        query left_;
        query right_;
    };

    struct minus_op {
        query left_;
        query right_;
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
