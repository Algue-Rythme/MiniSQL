#include <algorithm>

#include "normal_form.hpp"
#include "../SQL_Compiler/context.hpp"

using namespace std;
namespace x3 = boost::spirit::x3;

namespace SQL_Parser {
    namespace {
        class NormalFormTransformer : public boost::static_visitor<SQL_AST::query> {
        public:
            SQL_AST::query operator()(SQL_AST::minus_op minus_op) const {
                minus_op.left_ = boost::apply_visitor(*this, minus_op.left_);
                minus_op.right_ = boost::apply_visitor(*this, minus_op.right_);
                return SQL_AST::query{minus_op};
            }

            SQL_AST::query operator()(SQL_AST::union_op union_op) const {
                union_op.left_ = boost::apply_visitor(*this, union_op.left_);
                union_op.right_ = boost::apply_visitor(*this, union_op.right_);
                return SQL_AST::query{union_op};
            }

            SQL_AST::query operator()(SQL_AST::select s) const {
                if (s.or_conditions_.size() >= 2) {
                    auto q = transform_or_into_unions(s);
                    return boost::apply_visitor(*this, q);
                }
                normalize_subqueries(s.or_conditions_);
                auto nf_not_in = transform_not_in_into_minus(s);
                return nf_not_in;
            }

            void normalize_subqueries(SQL_AST::or_conditions& or_cond) const {
                for (auto& and_cond : or_cond) {
                    for (auto& atomic_cond : and_cond) {
                        if (auto in_cond_ptr = boost::get<SQL_AST::in_condition>(&atomic_cond)) {
                            in_cond_ptr->query_ = boost::apply_visitor(*this, in_cond_ptr->query_);
                        }
                    }
                }
            }

            SQL_AST::query transform_not_in_into_minus(SQL_AST::select const& s) const {
                if (s.or_conditions_.size() == 0)
                    return SQL_AST::query{s};
                SQL_AST::and_conditions const& conds = s.or_conditions_.back();
                auto find_not_in_cond =
                    [](SQL_AST::atomic_condition const& atom){
                        if (auto in_ptr = boost::get<SQL_AST::in_condition>(&atom)) {
                            return in_ptr->in_type_ == SQL_AST::in_type::NOT_IN;
                        }
                        return false;
                    };
                auto it = find_if(begin(conds), end(conds), find_not_in_cond);
                SQL_AST::select left = s;
                left.or_conditions_.back().assign(begin(conds), it);
                if (it != end(conds)) {
                    copy(it+1, end(conds), back_inserter(left.or_conditions_.back()));
                    if (auto in_ptr = boost::get<SQL_AST::in_condition>(&(*it))) {
                        SQL_AST::in_condition in_cond = *in_ptr;
                        in_cond.in_type_ = SQL_AST::in_type::IN;
                        SQL_AST::select right = s;
                        right.or_conditions_.back().assign(begin(conds), it);
                        right.or_conditions_.back().emplace_back(in_cond);
                        copy(it+1, end(conds), back_inserter(right.or_conditions_.back()));
                        auto left_q = transform_not_in_into_minus(left);
                        auto right_q = transform_not_in_into_minus(right);
                        SQL_AST::minus_op minus_op;
                        minus_op.left_ = left_q;
                        minus_op.right_ = right_q;
                        return SQL_AST::query{minus_op};
                    } else {
                        throw runtime_error("[normal_form.cpp][transform_not_in_into_minus] Critical Error");
                    }
                }
                return transform_in_into_join(left);
            }

            SQL_AST::query transform_in_into_join(SQL_AST::select s) const {
                if (s.or_conditions_.size() == 0)
                    return SQL_AST::query{s};
                SQL_AST::and_conditions const& old_conds = s.or_conditions_[0];
                SQL_AST::and_conditions transformed_conds;
                SQL_AST::cartesian_product& transformed_relations = s.relations_;
                for (auto const& atomic_cond : old_conds) {
                    if (auto in_ptr = boost::get<SQL_AST::in_condition>(&atomic_cond)) {
                        if (auto recur_s_ptr = boost::get<x3::forward_ast<SQL_AST::select>>(&in_ptr->query_)) {
                            auto s_ptr = recur_s_ptr->get_pointer();
                            SQL_AST::comparison_condition join_cond;
                            join_cond.left_ = in_ptr->att_;
                            join_cond.op_ = SQL_AST::comparison_operator::EQ;
                            if (s_ptr->projections_.size() != 1) {
                                throw SQL_Compiler::SemanticError(
                                    "The nested query "
                                    "WHERE ... IN (SELECT * FROM ...)"
                                    " must contain only one attribute in the projection"
                                );
                            }
                            join_cond.right_ = s_ptr->projections_[0].attribute_;
                            transformed_conds.emplace_back(join_cond);
                            for (auto const& new_atom_cond : s_ptr->or_conditions_[0]) {
                                transformed_conds.push_back(new_atom_cond);
                            }
                            for (auto const& new_rel : s_ptr->relations_) {
                                transformed_relations.push_back(new_rel);
                            }
                        } else {
                            throw SQL_Compiler::SemanticError(
                                "WHERE ... IN conditions with non trivial subqueries"
                                "(i.e select with no OR condition)"
                                " are not supported yet"
                            );
                        }
                    } else {
                        transformed_conds.push_back(atomic_cond);
                    }
                }
                s.or_conditions_[0] = transformed_conds;
                s.relations_ = transformed_relations;
                return SQL_AST::query{s};
            }

            SQL_AST::query bind_condition(SQL_AST::select const& old, SQL_AST::and_conditions const& cond) const {
                SQL_AST::select s = old;
                s.or_conditions_.assign({cond});
                return SQL_AST::query{s};
            }

            SQL_AST::query recurse_or_into_union(
                SQL_AST::select const& old,
                SQL_AST::or_conditions const& conds,
                unsigned int index) const {
                if (conds.empty())
                    return SQL_AST::query{old};
                auto A = bind_condition(old, conds[index]);
                if (index+1 == conds.size())
                    return A;
                SQL_AST::minus_op minus{};
                minus.left_ = recurse_or_into_union(old, conds, index+1);
                minus.right_ = A;
                SQL_AST::union_op sub{};
                sub.left_ = A;
                sub.right_ = minus;
                return SQL_AST::query{sub};
            }

            SQL_AST::query transform_or_into_unions(SQL_AST::select const& s) const {
                return recurse_or_into_union(s, s.or_conditions_, 0);
            }
        };
    }

    SQL_AST::query to_normal_form(SQL_AST::query const& q) {
        NormalFormTransformer nf;
        return boost::apply_visitor(nf, q);
    }
}
