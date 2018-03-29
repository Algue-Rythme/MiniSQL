#ifndef QUERY_DEF_HPP
#define QUERY_DEF_HPP

#include <algorithm>
#include <string>

#include "SQL_AST.hpp"
#include "SQL_AST_adapted.hpp"
#include "query.hpp"
#include "SQL_error_handler.hpp"
#include "SQL_annotation.hpp"

namespace SQL_Parser {
    namespace x3 = boost::spirit::x3;

    using x3::char_;
    using x3::raw;
    using x3::omit;
    using x3::lexeme;
    using x3::alpha;
    using x3::alnum;
    using x3::lit;
    using x3::expect;
    using x3::string;
    using x3::no_case;

    query_type const query = "query";

    struct minus_op_class;
    typedef x3::rule<minus_op_class, SQL_AST::minus_op> minus_op_type;
    minus_op_type const minus_op = "minus_op";

    struct union_op_class;
    typedef x3::rule<union_op_class, SQL_AST::union_op> union_op_type;
    union_op_type const union_op = "union_op";

    struct select_class;
    typedef x3::rule<select_class, SQL_AST::select> select_type;
    select_type const select = "select";

    struct group_by_class;
    typedef x3::rule<group_by_class, SQL_AST::group_by> group_by_type;
    group_by_type const group_by = "group_by";

    struct order_by_class;
    typedef x3::rule<order_by_class, SQL_AST::order_by> order_by_type;
    order_by_type const order_by = "order_by";

    struct order_by_clause_class;
    typedef x3::rule<order_by_clause_class, SQL_AST::order_by_clause> order_by_clause_type;
    order_by_clause_type const order_by_clause = "order_by_clause";

    struct projections_class;
    typedef x3::rule<projections_class, SQL_AST::projections> projections_type;
    projections_type const projections = "projections";

    struct project_rename_class;
    typedef x3::rule<project_rename_class, SQL_AST::project_rename> project_rename_type;
    project_rename_type const project_rename = "project_rename";

    struct attribute_class;
    typedef x3::rule<attribute_class, SQL_AST::attribute> attribute_type;
    attribute_type const attribute = "attribute";

    struct cartesian_product_class;
    typedef x3::rule<cartesian_product_class, SQL_AST::cartesian_product> cartesian_product_type;
    cartesian_product_type const cartesian_product = "cartesian_product";

    struct load_file_class;
    typedef x3::rule<load_file_class, SQL_AST::load_file> load_file_type;
    load_file_type const load_file = "load_file";

    struct subquery_class;
    typedef x3::rule<subquery_class, SQL_AST::subquery> subquery_type;
    subquery_type const subquery = "subquery";

    struct from_relation_class;
    typedef x3::rule<from_relation_class, SQL_AST::from_relation> from_relation_type;
    from_relation_type const from_relation = "from_relation";

    struct or_conditions_class;
    typedef x3::rule<or_conditions_class, SQL_AST::or_conditions> or_conditions_type;
    or_conditions_type const or_conditions = "or_conditions";

    struct and_conditions_class;
    typedef x3::rule<and_conditions_class, SQL_AST::and_conditions> and_conditions_type;
    and_conditions_type const and_conditions = "and_conditions";

    struct comparison_condition_class;
    typedef x3::rule<comparison_condition_class, SQL_AST::comparison_condition> comparison_condition_type;
    comparison_condition_type const comparison_condition = "comparison_condition";

    struct in_condition_class;
    typedef x3::rule<in_condition_class, SQL_AST::in_condition> in_condition_type;
    in_condition_type const in_condition = "in_condition";

    struct identifier_class;
    typedef x3::rule<identifier_class, std::string> identifier_type;
    identifier_type const identifier = "identifier";

    auto const identifier_def = raw[lexeme[(alpha | '_') >> *(alnum | '_')]];

    auto const attribute_def = identifier >> "." > identifier;
    auto const project_rename_def = attribute >> -(no_case["as"] > identifier);
    auto const projections_def = project_rename % ",";

    auto const filename_def =
        x3::lit("\"") >>
        lexeme[raw[
            *(alnum | '_' | '-' | '/')
            > expect[".csv"]
        ]] >
        expect["\""];

    auto const subquery_def = "(" > query > ")" > identifier;
    auto const load_file_def = filename_def > identifier;
    auto const from_relation_def = load_file | subquery;
    auto const cartesian_product_def = from_relation % ",";

    struct comparison_operator_ : x3::symbols<SQL_AST::comparison_operator> {
        comparison_operator_() {
            add
                ("=", SQL_AST::comparison_operator::EQ)
                ("<>", SQL_AST::comparison_operator::NEQ)
                ("!=", SQL_AST::comparison_operator::NEQ)
                ("<=", SQL_AST::comparison_operator::LTE)
                ("<", SQL_AST::comparison_operator::LT)
                (">=", SQL_AST::comparison_operator::GTE)
                (">", SQL_AST::comparison_operator::GT)
            ;
        }
    } comparison_operator_;

    auto const field_def = "\"" > lexeme[*(char_ - "\"")] > "\"";
    auto const operand_def = (attribute | field_def);
    auto const comparison_condition_def = operand_def >> comparison_operator_ > operand_def;

    using x3::_val;
    auto const not_in_type_def
        = x3::rule<class not_in_type_def, SQL_AST::in_type>()
        = (no_case["not"] >> no_case["in"])[([](auto& ctx){ _val(ctx) = SQL_AST::in_type::NOT_IN;})];
    auto const in_type_def
        = x3::rule<class in_type_def, SQL_AST::in_type>()
        = (no_case["in"])[([](auto& ctx){ _val(ctx) = SQL_AST::in_type::IN;})];
    auto const in_or_not_in_def
        = x3::rule<class in_or_not_in_def, SQL_AST::in_type>()
        = not_in_type_def | in_type_def;

    auto const in_condition_def = attribute >> in_or_not_in_def > "(" > query > ")";

    auto const atomic_condition_def = (comparison_condition | in_condition);
    auto const and_conditions_def = atomic_condition_def % no_case["and"];
    auto const or_conditions_def = and_conditions % no_case["or"];

    auto const group_by_def = attribute % ",";

    struct order_type_ : x3::symbols<SQL_AST::order_type> {
        order_type_() {
            add("asc", SQL_AST::order_type::ASC)("desc", SQL_AST::order_type::DESC);
        }
    } order_type_;

    auto const order_by_clause_def = attribute >> (no_case[order_type_] | x3::attr(SQL_AST::order_type::ASC));
    auto const order_by_def = order_by_clause % ",";

    auto const select_def =
        no_case["select"] > projections >
        no_case["from"] > cartesian_product >
        no_case["where"] > or_conditions
        >> -(no_case["group"] > no_case["by"] > group_by)
        >> -(no_case["order"] > no_case["by"] > order_by);

    auto const union_op_def = "(" >> query >> ")" >> no_case["union"] >> x3::lit("(") >> query >> x3::lit(")");
    auto const minus_op_def = "(" >> query >> ")" >> no_case["minus"] >> x3::lit("(") >> query >> x3::lit(")");

    auto const query_def = (select | union_op | minus_op);

    BOOST_SPIRIT_DEFINE(
        query
        , minus_op
        , union_op
        , select
        , group_by
        , order_by
        , order_by_clause
        , projections
        , project_rename
        , attribute
        , cartesian_product
        , load_file
        , subquery
        , from_relation
        , or_conditions
        , and_conditions
        , comparison_condition
        , in_condition
        , identifier
    )

    struct query_class : annotation_base, error_handler_base {};
    struct minus_op_class : annotation_base {};
    struct union_op_class : annotation_base {};
    struct select_class : annotation_base {};
    struct group_by_class : annotation_base {};
    struct order_by_class : annotation_base {};
    struct order_by_clause_class : annotation_base {};
    struct projections_class : annotation_base {};
    struct project_rename_class : annotation_base {};
    struct attribute_class : annotation_base {};
    struct cartesian_product_class : annotation_base {};
    struct load_file : annotation_base {};
    struct subquery : annotation_base {};
    struct from_relation : annotation_base {};
    struct or_conditions_class : annotation_base {};
    struct and_conditions_class : annotation_base {};
    struct comparison_condition_class : annotation_base {};
    struct in_condition_class : annotation_base {};
    struct identifier_class : annotation_base {};

    query_type const& get_query() {
        return query;
    }
}

#endif
