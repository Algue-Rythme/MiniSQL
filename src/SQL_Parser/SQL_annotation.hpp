#ifndef SQL_ANNOTATION_HPP
#define SQL_ANNOTATION_HPP

#include <map>
#include "SQL_AST.hpp"

namespace SQL_Parser {
    struct annotation_base
    {
        template <typename T, typename Iterator, typename Context>
        inline void on_success(Iterator const& first, Iterator const& last
                               , T& ast, Context const& context);
    };

    template <typename T, typename Iterator, typename Context>
    inline void
    annotation_base::on_success(Iterator const& first, Iterator const& last
                                , T& ast, Context const& context)
    {
        auto& error_handler = x3::get<error_handler_tag>(context).get();
        error_handler.tag(ast, first, last);
    }
}

#endif
