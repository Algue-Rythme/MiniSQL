#ifndef SQL_AST_ADAPTED_HPP
#define SQL_AST_ADAPTED_HPP

#include <boost/fusion/include/adapt_struct.hpp>

#include "SQL_AST.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::atomic_condition,
    (SQL_AST::attribute, left_)
    (SQL_AST::comparison_operator, op_)
    (SQL_AST::attribute, right_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::and_conditions,
    (std::vector<SQL_AST::atomic_condition>, atomic_conditions_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::or_conditions,
    (std::vector<SQL_AST::and_conditions>, and_conditions_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::relation,
    (std::string, filename_)
    (std::string, alias_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::carthesian_product,
    (std::vector<SQL_AST::relation>, relations_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::attribute,
    (std::string, relation_)
    (std::string, column_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::project_rename,
    (SQL_AST::attribute, attribute_)
    (boost::optional<std::string>, rename_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::projections,
    (std::vector<SQL_AST::project_rename>, project_rename_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::select,
    (SQL_AST::projections, projections_)
    (SQL_AST::carthesian_product, relations_)
    (SQL_AST::or_conditions, or_conditions_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::query,
    (SQL_AST::select, select_)
)

#endif
