#ifndef SQL_AST_ADAPTED_HPP
#define SQL_AST_ADAPTED_HPP

#include <boost/fusion/include/adapt_struct.hpp>

#include "SQL_AST.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::comparison_condition
    , left_
    , op_
    , right_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::in_condition
    , att_
    , in_type_
    , query_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::subquery
    , query_
    , alias_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::load_file
    , filename_
    , alias_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::attribute
    , relation_
    , column_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::project_rename
    , attribute_
    , rename_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::order_by_clause
    , att_
    , order_type_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::select
    , projections_
    , relations_
    , or_conditions_
    , group_by_
    , order_by_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::union_op
    , left_
    , right_
)

BOOST_FUSION_ADAPT_STRUCT(
    SQL_AST::minus_op
    , left_
    , right_
)

#endif
