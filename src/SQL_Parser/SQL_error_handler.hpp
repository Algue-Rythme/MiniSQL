#ifndef SQL_ERROR_HANDLER_HPP
#define SQL_ERROR_HANDLER_HPP

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <map>

#include "query.hpp"

namespace SQL_Parser {
    namespace x3 = boost::spirit::x3;

    template <typename Iterator>
    using error_handler = x3::error_handler<Iterator>;

    struct error_handler_tag;

    struct error_handler_base
    {
        error_handler_base();

        template <typename Iterator, typename Exception, typename Context>
        x3::error_handler_result on_error(
            Iterator& first, Iterator const& last
          , Exception const& x, Context const& context);

        std::map<std::string, std::string> id_map;
    };

    inline error_handler_base::error_handler_base() {
        id_map["query"] = "valid MiniSQL query";
        id_map["\"select\""] = "SELECT keyword";
        id_map["\"from\""] = "FROM keyword";
        id_map["\"where\""] = "WHERE keyword";
        id_map["projections"] = "valid attribute name";
        id_map["project_rename"] = "valid attribute name";
        id_map["attribute"] = "valid attribute name";
        id_map["carthesian_product"] = "valid attribute name";
        id_map["relation"] = "valid relation name ";
        id_map["or_conditions"] = "valid MiniSQL condition";
        id_map["and_conditions"] = "valid MiniSQL condition";
        id_map["atomic_condition"] = "valid MiniSQL condition";
    }

    template <typename Iterator, typename Exception, typename Context>
    inline x3::error_handler_result
    error_handler_base::on_error(
        Iterator& first, Iterator const& last
      , Exception const& x, Context const& context)
    {
        std::string which = x.which();
        auto iter = id_map.find(which);
        if (iter != id_map.end())
            which = iter->second;

        std::string message = "Error! Expecting " + which + " here:";
        auto& error_handler = x3::get<error_handler_tag>(context).get();
        error_handler(x.where(), message);
        return x3::error_handler_result::fail;
    }
}

#endif
