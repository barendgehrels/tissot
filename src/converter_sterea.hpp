#ifndef TISSOT_CONVERTER_STEREA_HPP
#define TISSOT_CONVERTER_STEREA_HPP

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


class converter_cpp_bg_sterea : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_sterea(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            // Sterea uses a named parameter "en" instead of a void*
            m_prop.extra_impl_includes.insert("pj_gauss.hpp");

            BOOST_FOREACH(std::string& line, m_prop.proj_parameters)
            {
                if (line == "void *en;")
                {
                    line = "gauss::GAUSS en;";
                }
            }
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    if (boost::starts_with(boost::trim_copy(line), "lp = pj_gauss("))
                    {
                        line = tab1 + "detail::gauss::gauss(m_proj_parm.en, lp_lon, lp_lat);";
                    }
                    else if (boost::contains(line, "return(pj_inv_gauss"))
                    {
                        line = tab1 + "detail::gauss::inv_gauss(m_proj_parm.en, lp_lon, lp_lat);";
                    }
                }
            }



            if (m_prop.derived_projections.size() == 1)
            {
                std::vector<std::string>& lines
                        = m_prop.derived_projections.front().constructor_lines;

                functor_trimmed_equals functor("proj_parm.en=0;");
                lines.erase
                    (
                        std::remove_if(lines.begin(), lines.end(),functor),
                        lines.end()
                    );

                BOOST_FOREACH(std::string& line, lines)
                {
                    if (boost::contains(line, "en = pj_gauss_ini"))
                    {
                        line = tab1 + "proj_parm.en = detail::gauss::gauss_ini(par.e, par.phi0, proj_parm.phic0, R);";
                    }
                }

            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_STEREA_HPP
