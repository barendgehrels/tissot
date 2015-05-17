#ifndef TISSOT_CONVERTER_IMW_P_HPP
#define TISSOT_CONVERTER_IMW_P_HPP

// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "tissot_structs.hpp"
#include "tissot_util.hpp"
#include "converter_base.hpp"
#include <boost/algorithm/string/replace.hpp>


namespace boost { namespace geometry { namespace proj4converter
{


class converter_cpp_bg_imw_p : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_imw_p(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void replace_if_found(std::vector<std::string>& lines, std::string const& line, std::string const& term, std::string const& replacement)
        {
            std::vector<std::string>::iterator it =
                std::find_if(lines.begin(), lines.end(),
                    functor_trimmed_equals(line));

            if (it != lines.end())
            {
                boost::replace_all(*it, term, replacement);
            }
        }

        void pre_convert()
        {
            // Move the free function xy, after inverse, to inlined_functions
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                std::vector<std::string>::iterator it
                    = std::find_if(proj.lines.begin(), proj.lines.end(),
                                           functor_trimmed_equals("static void"));
                if (it != proj.lines.end())
                {
                    // Move all further lines
                    for (std::vector<std::string>::iterator fit = it; fit != proj.lines.end(); ++fit)
                    {
                        m_prop.inlined_functions.push_back(*fit);
                    }
                    proj.lines.erase(it, proj.lines.end());
                }
            }
        }


        void convert()
        {
            m_prop.extra_structs.push_back("struct XY { double x, y; }; // specific for IMW_P");

            // First adapt projections and move xy to inlined_functions
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                replace_if_found(proj.lines, "double yc;", ";", " = 0;");
                replace_if_found(proj.lines, "xy = loc_for(lp, P, &yc);", "xy =", "XY xy =");
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    boost::replace_all(line, "lp, P", "lp_lon, lp_lat, this->m_par, m_proj_parm");
                }
                if (proj.direction == "forward")
                {
                    proj.lines.push_back(tab1 + "xy_x = xy.x; xy_y = xy.y;");
                }
            }

            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                boost::replace_all(line, "LP lp", "double const& lp_lam, double const& lp_phi");
                boost::replace_all(line, "lp.lam", "lp_lam");
                boost::replace_all(line, "lp.phi", "lp_phi");
                boost::replace_all(line, "Parameters& par,", "Parameters& par, par_imw_p& proj_parm,");
                if (boost::contains(line, "Parameters")
                    && (boost::contains(line, "loc_for") || boost::contains(line, "xy")))
                {
                    // Make parameters const in this function definition
                    boost::replace_all(line, "& par", " const& par");
                    boost::replace_all(line, "& proj_parm", " const& proj_parm");
                }

                // Set translation (done in converter) to proj_parm, but not for .params and .es
                boost::replace_all(line, "par.", "proj_parm.");
                boost::replace_all(line, "this->m_proj_parm.", "proj_parm.");

                boost::replace_all(line, "proj_parm.params", "par.params");
                boost::replace_all(line, "proj_parm.es", "par.es");
            }

            // See also lsat/sconics
            BOOST_FOREACH(derived& d, m_prop.derived_projections)
            {
                BOOST_FOREACH(std::string& line, d.constructor_lines)
                {
                    boost::replace_last(line, "(P,", "(par, proj_parm,");
                }
            }

        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_IMW_P_HPP
