#include <vector>
#include <iostream>

#include "project_operator.hpp"

using namespace std;

namespace SQL_Compiler {
    ProjectIterator::ProjectIterator(OperatorIterator&& _it, Projecter const& projecter)
    : it(std::move(_it)), projecter(projecter) {
        if (!is_done())
            t = projecter(*it);
    }

    Tuple const& ProjectIterator::dereference() const {
        return t;
    }

    void ProjectIterator::increment() {
        ++it;
        if (!is_done())
            t = projecter(*it);
    }

    bool ProjectIterator::is_done() const {
        return it.is_done();
    }

    void ProjectIterator::restart() {
        it.restart();
        if (!is_done())
            t = projecter(*it);
    }

    Project::Project(BaseOperator * const nextOp, Context const& ctx, SQL_AST::projections const& projections)
    :  nextOp(nextOp), ctx(ctx), projections(projections) {
        compile();
    }

    OperatorIterator Project::begin() const {
        return OperatorIterator(new ProjectIterator(nextOp->begin(), projecter));
    }

    void Project::compile() {
        unsigned int s = projections.size();
        vector<unsigned int> indices; indices.reserve(s);
        for (auto const& project : projections) {
            auto const att = project.attribute_;
            indices.push_back(ctx[att.relation_][att.column_]);
        }
        projecter = [indices,s](Tuple const& in) -> Tuple {
            Tuple out; out.reserve(s);
            for (auto const& indice : indices) {
                out.push_back(in[indice]);
            }
            return out;
        };
    }
}
