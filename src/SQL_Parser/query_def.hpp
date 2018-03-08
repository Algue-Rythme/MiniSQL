#ifndef QUERY_DEF_HPP
#define QUERY_DEF_HPP

#include "SQL_AST.hpp"
#include "SQL_AST_adapted.hpp"
#include "query.hpp"
#include "SQL_error_handler.hpp"
#include "SQL_annotation.hpp"

namespace SQL_Parser {
    namespace x3 = boost::spirit::x3;

    using x3::char_;
    using x3::raw;
    using x3::lexeme;
    using x3::alpha;
    using x3::alnum;
    using x3::lit;
    using x3::expect;
    using x3::string;

    query_type const query = "query";

    struct select_class;
    typedef x3::rule<select_class, SQL_AST::select> select_type;
    select_type const select = "select";

    struct projections_class;
    typedef x3::rule<projections_class, SQL_AST::projections> projections_type;
    projections_type const projections = "projections";

    struct project_rename_class;
    typedef x3::rule<project_rename_class, SQL_AST::project_rename> project_rename_type;
    project_rename_type const project_rename = "project_rename";

    struct attribute_class;
    typedef x3::rule<attribute_class, SQL_AST::attribute> attribute_type;
    attribute_type const attribute = "attribute";

    struct carthesian_product_class;
    typedef x3::rule<carthesian_product_class, SQL_AST::carthesian_product> carthesian_product_type;
    carthesian_product_type const carthesian_product = "carthesian_product";

    struct relation_class;
    typedef x3::rule<relation_class, SQL_AST::relation> relation_type;
    relation_type const relation = "relation";

    struct or_conditions_class;
    typedef x3::rule<or_conditions_class, SQL_AST::or_conditions> or_conditions_type;
    or_conditions_type const or_conditions = "or_conditions";

    struct and_conditions_class;
    typedef x3::rule<and_conditions_class, SQL_AST::and_conditions> and_conditions_type;
    and_conditions_type const and_conditions = "and_conditions";

    struct atomic_condition_class;
    typedef x3::rule<atomic_condition_class, SQL_AST::atomic_condition> atomic_condition_type;
    atomic_condition_type const atomic_condition = "atomic_condition";

    struct identifier_class;
    typedef x3::rule<identifier_class, std::string> identifier_type;
    identifier_type const identifier = "identifier";

    auto const identifier_def = raw[lexeme[(alpha | '_') >> *(alnum | '_')]];

    auto const attribute_def = identifier > "." > identifier;
    auto const project_rename_def = attribute >> -("as" > identifier);
    auto const projections_def = project_rename % ",";

    auto const filename_def =
        expect["\""] >
        raw[lexeme[
            *(alnum | '_' | '-' | '/')
            > expect[".csv"]
        ]] >
        expect["\""];
    auto const relation_def = filename_def > identifier;
    auto const carthesian_product_def = relation % ",";

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

    auto const atomic_condition_def = attribute > comparison_operator_ > attribute;
    auto const and_conditions_def = atomic_condition % "and";
    auto const or_conditions_def = and_conditions % "or";

    auto const select_def =
        expect["select"] > projections >
        expect["from"] > carthesian_product >
        expect["where"] > or_conditions;

    auto const query_def = select > ";";

    BOOST_SPIRIT_DEFINE(
        query
        , select
        , projections
        , project_rename
        , attribute
        , carthesian_product
        , relation
        , or_conditions
        , and_conditions
        , atomic_condition
        , identifier
    )

    struct query_class : annotation_base, error_handler_base {};
    struct select_class : annotation_base {};
    struct projections_class : annotation_base {};
    struct project_rename_class : annotation_base {};
    struct attribute_class : annotation_base {};
    struct carthesian_product_class : annotation_base {};
    struct relation_class : annotation_base {};
    struct or_conditions_class : annotation_base {};
    struct and_conditions_class : annotation_base {};
    struct atomic_condition_class : annotation_base {};
    struct identifier_class : annotation_base {};

    query_type const& get_query() {
        return query;
    }
}

#endif
