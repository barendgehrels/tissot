#ifndef TISSOT_COMMON_HPP
#define TISSOT_COMMON_HPP

// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "tissot_structs.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>


namespace boost { namespace geometry { namespace proj4converter
{

struct remove_rho_parameter
{
    inline bool operator()(std::string const& item) const
    {
        // Remove rho (and not rho0)
        return boost::contains(item, "rho;");
    }
private:
    std::vector<std::string> tokens;
};

inline void adapt_rho_parameter(projection_properties& prop)
{
    // Changes proj parameters into local variables, all but m0 and en
    // Because we have fwd/inv as const methods

    BOOST_FOREACH(projection& proj, prop.projections)
    {
        bool has_rho = false;
        BOOST_FOREACH(std::string& line, proj.lines)
        {
            // Save
            boost::replace_all(line, "this->m_proj_parm.rho0", "PROJ.rho0");

            // Replace
            if (boost::contains(line, "this->m_proj_parm.rho"))
            {
                boost::replace_all(line, "this->m_proj_parm.rho", "rho");
                has_rho = true;
            }

            // Restore
            boost::replace_all(line, "PROJ.rho0", "this->m_proj_parm.rho0");
        }
        if (has_rho)
        {
            // We don't move assignments as they are complex and multi-lined here
            proj.lines.insert(proj.lines.begin(), tab1 + "double rho = 0.0;");
        }
    }

    prop.proj_parameters.erase
        (
            std::remove_if(prop.proj_parameters.begin(),
                prop.proj_parameters.end(), remove_rho_parameter()),
            prop.proj_parameters.end()
        );


}

inline void add_declaration(std::string& line, bool& assigned,
            std::string const& var)
{
    if (! assigned)
    {
        std::string expression = var + " =";
        if (boost::contains(line, " " + expression + " "))
        {
            boost::replace_all(line, expression, "double " + expression);
            assigned = true;
        }
    }
}

inline bool move_expression(projection& proj, std::string const& expr, std::string const& var)
{
    for (std::vector<std::string>::iterator it = proj.lines.begin();
        it != proj.lines.end();
        ++it)
    {
        std::string& line = *it;
        if (boost::contains(line, expr))
        {
            boost::replace_all(line, expr, var);
            proj.lines.insert(it, tab1 + "double " + expr + ";");
            return true;
        }
        if (boost::contains(line, "double " + var))
        {
            // already assigned somehow
            return true;
        }
    }
    return false;
}


inline void insert_ignore_unused_function(projection_properties& prop,
        std::string const& function)
{
    BOOST_FOREACH(derived& der, prop.derived_projections)
    {
        // In first setup-function, ignore specified function
        der.constructor_lines.insert(der.constructor_lines.begin(),
            tab1 + "boost::ignore_unused(" + function + ");");
        return;
    }
}


}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_COMMON_HPP

