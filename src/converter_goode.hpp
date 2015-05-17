#ifndef TISSOT_CONVERTER_GOODE_HPP
#define TISSOT_CONVERTER_GOODE_HPP

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


class converter_cpp_bg_goode  : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_goode(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            // Goode is a projection combining moll/sinu
            m_prop.template_struct = "<Geographic, Cartesian, Parameters>";
            m_prop.extra_proj_includes.insert("moll.hpp");
            m_prop.extra_proj_includes.insert("gn_sinu.hpp");

            m_prop.extra_member_initialization_list.push_back("m_proj_parm(par)");

            // Basically we replace most of it here...
            m_prop.proj_parameters.clear();
            m_prop.proj_parameters.push_back("sinu_ellipsoid<Geographic, Cartesian, Parameters>    sinu;");
            m_prop.proj_parameters.push_back("moll_spheroid<Geographic, Cartesian, Parameters>    moll;");
            m_prop.proj_parameters.push_back("");
            m_prop.proj_parameters.push_back("par_goode(const Parameters& par) : sinu(par), moll(par) {}");

            m_prop.inlined_functions.clear();

            // 4 cases will become "m_proj_parm.sinu.inv(xy_x, xy_y, lp_lon, lp_lat);" or similar
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    std::string s = boost::trim_copy(line);

                    if (boost::starts_with(s, "xy ="))
                    {
                        // Forward projection
                        boost::replace_all(line, "->fwd", ".fwd");
                        boost::replace_all(line, "xy = ", "");
                        boost::replace_all(line, "lp, ", "lp_lon, lp_lat, xy_x, xy_y");
                        boost::replace_last(line, "this->m_proj_parm.sinu", "");
                        boost::replace_last(line, "this->m_proj_parm.moll", "");
                    }
                    else if (boost::starts_with(s, "lp ="))
                    {
                        // Inverse projection
                        boost::replace_all(line, "->inv", ".inv");
                        boost::replace_all(line, "lp = ", "");
                        boost::replace_all(line, "xy, ", "xy_x, xy_y, lp_lon, lp_lat");
                        boost::replace_last(line, "this->m_proj_parm.sinu", "");
                        boost::replace_last(line, "this->m_proj_parm.moll", "");
                    }
                }
            }

            if (m_prop.derived_projections.size() == 1)
            {
                std::vector<std::string>& lines
                        = m_prop.derived_projections.front().constructor_lines;

                functor_starts_with functor("throw proj", "if (!(proj_parm.sinu", "proj_parm.sinu", "proj_parm.moll");
                // Will be:
                // starts_with functor("if (!(P->sinu", "E_ERROR_0");

                lines.erase
                    (
                        std::remove_if(lines.begin(), lines.end(),functor),
                        lines.end()
                    );
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_GOODE_HPP
