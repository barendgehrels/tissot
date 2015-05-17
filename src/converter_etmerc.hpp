#ifndef TISSOT_CONVERTER_ETMERC_HPP
#define TISSOT_CONVERTER_ETMERC_HPP

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

namespace boost { namespace geometry { namespace proj4converter
{


class converter_cpp_bg_etmerc : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_etmerc(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            remove_conditional_code(m_prop.inlined_functions);
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                remove_conditional_code(proj.lines);
            }
            BOOST_FOREACH(derived& d, m_prop.derived_projections)
            {
                remove_conditional_code(d.constructor_lines);
            }

            // Make const correct
            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                boost::replace_all(line, "gatg(double *p1", "gatg(const double *p1");
                boost::replace_all(line, "double *p;", "const double *p;");

                boost::replace_all(line, "clens(double *a", "clens(const double *a");
                boost::replace_all(line, "*p, r, hr", "r, hr");
                boost::replace_all(line, "clenS(double *a", "clenS(const double *a");
                boost::replace_all(line, "*p, r, i", "r, i");
                boost::replace_all(line, "p = a + size;", "const double* p = a + size;");
            }

        }

        void remove_conditional_code(std::vector<std::string>& lines)
        {
            typedef std::vector<std::string>::iterator it_type;
            // Remove #ifndef/#endif structs
            functor_starts_with sw_ifdef("#ifdef _GNU");
            functor_starts_with sw_else("#else");
            functor_starts_with sw_endif("#endif");
            it_type it_ifdef = std::find_if(lines.begin(), lines.end(), sw_ifdef);
            while (it_ifdef != lines.end())
            {
                it_type it_endif = std::find_if(it_ifdef, lines.end(), sw_endif);
                if (it_endif == lines.end())
                {
                    return;
                }
                // Get optional else in between
                it_type it_else = std::find_if(it_ifdef, it_endif, sw_else);
                if (it_else != it_endif)
                {
                    lines.erase(it_endif);
                    lines.erase(it_ifdef, it_else + 1);
                }
                else
                {
                    lines.erase(it_ifdef, it_endif + 1);
                }

                it_ifdef = std::find_if(lines.begin(), lines.end(), sw_ifdef);
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_ETMERC_HPP
