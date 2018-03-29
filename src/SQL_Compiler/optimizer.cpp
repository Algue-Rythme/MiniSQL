#include "optimizer.hpp"
#include "context.hpp"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <boost/optional.hpp>

using namespace std;
using namespace SQL_Compiler;

namespace SQL_Optimizer {
    namespace {
        class PushDowner : public boost::static_visitor<SQL_AST::query> {
            unordered_set<string> join_names;
            unordered_set<string> prefixes;

            string get_new_join_name(string const& s1, string const& s2) {
                string middle = "@";
                string cur = s1 + middle + s2;
                while (join_names.count(cur) > 0) {
                    middle += "@";
                    cur = s1 + middle + s2;
                }
                return cur;
            }

            string get_new_prefix(string const& s) {
                string suffix = "@";
                string cur = s + suffix;
                while (prefixes.count(cur) > 0) {
                    suffix += "@";
                    cur = s + suffix;
                }
                return cur;

            }
        public:
            SQL_AST::query operator()(SQL_AST::minus_op minus_op) {
                minus_op.left_ = boost::apply_visitor(*this, minus_op.left_);
                minus_op.right_ = boost::apply_visitor(*this, minus_op.right_);
                return SQL_AST::query{minus_op};
            }

            SQL_AST::query operator()(SQL_AST::union_op union_op) {
                union_op.left_ = boost::apply_visitor(*this, union_op.left_);
                union_op.right_ = boost::apply_visitor(*this, union_op.right_);
                return SQL_AST::query{union_op};
            }

            struct GetName : public boost::static_visitor<string> {
                string operator()(SQL_AST::attribute const& att) const {
                    return att.relation_;
                }

                string operator()(string const& str) const {
                    return std::string{};
                }
            };

            boost::optional<string> get_atom_name(SQL_AST::atomic_condition const& atom) {
                if (auto comp_cond_ptr = boost::get<SQL_AST::comparison_condition>(&atom)) {
                    auto const& comp_cond = *comp_cond_ptr;
                    auto left = boost::apply_visitor(GetName(), comp_cond.left_);
                    auto right = boost::apply_visitor(GetName(), comp_cond.right_);
                    if (left == right || right.empty())
                        return left;
                    if (left.empty())
                        return right;
                    return boost::none;
                } else {
                    throw NormalizingError("[optimizer.cpp] cannot work with IN or NOT IN conditions");
                }
            }

            boost::optional<pair<string, string>> get_pair_name(SQL_AST::atomic_condition const& atom) {
                if (auto comp_cond_ptr = boost::get<SQL_AST::comparison_condition>(&atom)) {
                    auto const& comp_cond = *comp_cond_ptr;
                    auto left = boost::apply_visitor(GetName(), comp_cond.left_);
                    auto right = boost::apply_visitor(GetName(), comp_cond.right_);
                    if (left.empty() || right.empty() || left == right)
                        return boost::none;
                    if (left < right)
                        return make_pair(left, right);
                    return make_pair(right, left);
                } else {
                    throw NormalizingError("[optimizer.cpp] cannot work with IN or NOT IN conditions");
                }
            }

            void push_down(SQL_AST::cartesian_product& relations) {
                for (auto& rel : relations) {
                    if (auto subquery_ptr = boost::get<SQL_AST::subquery>(&rel)) {
                        auto const& subquery = *subquery_ptr;
                        boost::apply_visitor(*this, subquery.query_);
                    }
                }
            }

            SQL_AST::query operator()(SQL_AST::select const& s) {
                auto no_atom = push_down_atoms(s);
                auto reordered_joins = reorder_joins(no_atom);
                return SQL_AST::query{reordered_joins};
            }

            SQL_AST::cartesian_product::const_iterator
            find_alias(SQL_AST::cartesian_product const& relations, string const& alias) {
                return find_if(begin(relations), end(relations),
                [&](auto const& rel){
                    return get_alias(rel) == alias;
                });
            }

            SQL_AST::projections decoration_from(string const& table, vector<string> const& names, string const& prefix) {
                SQL_AST::projections projections;
                for (auto const& name : names) {
                    SQL_AST::project_rename pr;
                    pr.attribute_.relation_ = table;
                    pr.attribute_.column_ = name;
                    pr.rename_ = prefix + name;
                    projections.push_back(pr);
                }
                return projections;
            }

            void update_attr_table(SQL_AST::operand& op, string const& pattern, string const& newname) {
                if (auto att_ptr = boost::get<SQL_AST::attribute>(&op)) {
                    auto& att = *att_ptr;
                    if (att.relation_ == pattern)
                        att.relation_ = newname;
                }
            }

            void update_attr_column(SQL_AST::operand& op, string const& pattern, string const& prefix) {
                if (auto att_ptr = boost::get<SQL_AST::attribute>(&op)) {
                    auto& att = *att_ptr;
                    if (att.relation_ == pattern)
                        att.column_ = prefix + att.column_;
                }
            }

            void rename_conds_table(SQL_AST::and_conditions& conds, string const& pattern, string const& newname) {
                for (auto& cond : conds) {
                    if (auto comp_ptr = boost::get<SQL_AST::comparison_condition>(&cond)) {
                        auto& comp = *comp_ptr;
                        update_attr_table(comp.left_, pattern, newname);
                        update_attr_table(comp.right_, pattern, newname);
                    }
                }
             }

             void rename_projections_table(SQL_AST::projections& projections, string const& pattern, string const& newname) {
                 for (auto& proj : projections) {
                     if (proj.attribute_.relation_ == pattern)
                        proj.attribute_.relation_ = newname;
                 }
             }

             void rename_conds_column(SQL_AST::and_conditions& conds, string const& pattern, string const& prefix) {
                 for (auto& cond : conds) {
                     if (auto comp_ptr = boost::get<SQL_AST::comparison_condition>(&cond)) {
                         auto& comp = *comp_ptr;
                         update_attr_column(comp.left_, pattern, prefix);
                         update_attr_column(comp.right_, pattern, prefix);
                     }
                 }
             }

             void rename_projections_column(SQL_AST::projections& projections, string const& pattern, string const& prefix) {
                 for (auto& proj : projections) {
                     if (proj.attribute_.relation_ == pattern)
                        proj.attribute_.column_ = prefix + proj.attribute_.column_;
                 }
             }

            SQL_AST::select reorder_joins(SQL_AST::select s) {
                if (s.relations_.size() <= 2)
                    return s;
                if (s.or_conditions_.empty())
                    return s;
                if (s.or_conditions_.size() > 1)
                    throw NormalizingError("[optimizer.cpp] cannot work with multiple OR conditions");
                boost::optional<pair<string, string>> opt_join_names;
                SQL_AST::and_conditions join_conds;
                SQL_AST::and_conditions remaining;
                for (auto const& cond : s.or_conditions_.back()) {
                    auto pair_name = get_pair_name(cond);
                    if (pair_name && !opt_join_names) {
                        opt_join_names = pair_name;
                    }
                    if (opt_join_names && opt_join_names.value() == pair_name) {
                        join_conds.push_back(cond);
                    } else {
                        remaining.push_back(cond);
                    }
                }
                if (!opt_join_names)
                    throw runtime_error("[optimizer.cpp] Empty join condition : STOP");
                auto join_names = opt_join_names.value();
                SQL_AST::cartesian_product relations;
                auto t1 = find_alias(s.relations_, join_names.first);
                auto t2 = find_alias(s.relations_, join_names.second);
                for (auto it = begin(s.relations_); it != end(s.relations_); ++it) {
                    if (it == t1 || it == t2)
                        continue ;
                    relations.push_back(*it);
                }
                string new_name = get_new_join_name(join_names.first, join_names.second);
                string new_name_t1 = get_new_prefix(join_names.first);
                string new_name_t2 = get_new_prefix(join_names.second);
                auto names_t1 = get_attributes_names(*t1);
                auto names_t2 = get_attributes_names(*t2);
                SQL_AST::projections p1 = decoration_from(join_names.first, names_t1, new_name_t1);
                SQL_AST::projections p2 = decoration_from(join_names.second, names_t2, new_name_t2);
                rename_conds_column(remaining, join_names.first, new_name_t1);
                rename_conds_column(remaining, join_names.second, new_name_t2);
                rename_projections_column(s.projections_, join_names.first, new_name_t1);
                rename_projections_column(s.projections_, join_names.second, new_name_t2);
                rename_conds_table(remaining, join_names.first, new_name);
                rename_conds_table(remaining, join_names.second, new_name);
                rename_projections_table(s.projections_, join_names.first, new_name);
                rename_projections_table(s.projections_, join_names.second, new_name);
                SQL_AST::select nt;
                copy(begin(p1), end(p1), back_inserter(nt.projections_));
                copy(begin(p2), end(p2), back_inserter(nt.projections_));
                nt.relations_.push_back(*t1);
                nt.relations_.push_back(*t2);
                nt.or_conditions_.push_back(join_conds);
                nt = push_down_atoms(nt);
                SQL_AST::subquery subnt;
                subnt.query_ = SQL_AST::query{nt};
                subnt.alias_ = new_name;
                s.relations_ = relations;
                s.relations_.emplace_back(subnt);
                s.or_conditions_.back() = remaining;
                s = reorder_joins(s);
                return s;
            }

            SQL_AST::select push_down_atoms(SQL_AST::select s) {
                if (s.relations_.size() == 1)
                    return s;
                if (s.or_conditions_.empty())
                    return s;
                if (s.or_conditions_.size() > 1)
                    throw NormalizingError("[optimizer.cpp] cannot work with multiple OR conditions");
                push_down(s.relations_);
                unordered_map<string, SQL_AST::and_conditions> restricted;
                SQL_AST::and_conditions remaining;
                for (auto const& cond : s.or_conditions_.back()) {
                    auto name = get_atom_name(cond);
                    if (name) {
                        restricted[name.value()].push_back(cond);
                    } else {
                        remaining.push_back(cond);
                    }
                }
                s.or_conditions_.back() = remaining;
                SQL_AST::cartesian_product relations;
                for (auto const& from_rel : s.relations_) {
                    string alias = get_alias(from_rel);
                    if (restricted.count(alias) == 0) {
                        relations.push_back(from_rel);
                    } else {
                        SQL_AST::select sub;
                        vector<string> names = get_attributes_names(from_rel);
                        for (auto const& name : names) {
                            SQL_AST::project_rename p;
                            p.attribute_.relation_ = alias;
                            p.attribute_.column_ = name;
                            p.rename_ = boost::none;
                            sub.projections_.push_back(p);
                        }
                        sub.relations_.push_back(from_rel);
                        sub.or_conditions_.push_back(restricted[alias]);
                        sub = push_down_atoms(sub);
                        SQL_AST::subquery subq;
                        subq.query_ = sub;
                        subq.alias_ = alias;
                        relations.emplace_back(subq);
                    }
                }
                s.relations_ = relations;
                return s;
            }
        };
    }

    SQL_AST::query push_down_select(SQL_AST::query const& q) {
        PushDowner push_down;
        return boost::apply_visitor(push_down, q);
    }
}
