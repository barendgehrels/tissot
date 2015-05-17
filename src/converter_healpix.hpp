#ifndef TISSOT_CONVERTER_HEALPIX_HPP
#define TISSOT_CONVERTER_HEALPIX_HPP

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


class converter_cpp_bg_healpix : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_healpix(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            replace_defines();
            replace_std_functions();
            correct_enumerations();
            replace_lp_xy();
        }

    private :

        void replace_defines()
        {
            // Convert defines to just code, because R1, etc are also used as
            // variable names in other projections
            std::string r1, r2, r3, ident, rot;

            BOOST_FOREACH(macro_or_const const& def, m_prop.defined_macros)
            {
                if (def.name == "R1") { r1 = def.value; }
                else if (def.name == "R2") { r2 = def.value; }
                else if (def.name == "R3") { r3 = def.value; }
                else if (def.name == "IDENT") { ident = def.value; }
                else if (def.name == "ROT") { rot = def.value; }
            }
            boost::replace_all(rot, "R1", r1);
            boost::replace_all(rot, "R2", r2);
            boost::replace_all(rot, "R3", r3);
            boost::replace_all(rot, "IDENT", ident);

            m_prop.defined_macros.clear();

            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                boost::replace_all(line, "ROT", rot);
            }
        }

        void replace_std_functions()
        {
            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                boost::replace_all(line, "MIN", "std::min");
                boost::replace_all(line, "MAX", "std::max");
            }
        }

        void correct_enumerations()
        {
            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                if (boost::contains(line, ".region"))
                {
                    boost::replace_all(line, "north", "CapMap::north");
                    boost::replace_all(line, "south", "CapMap::south");
                    boost::replace_all(line, "equatorial", "CapMap::equatorial");
                }
            }
        }

        void replace_lp_xy()
        {
            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                std::string trimmed = boost::trim_copy(line);
                if (boost::starts_with(trimmed, "XY ") && boost::contains(trimmed, "("))
                {
                    boost::replace_all(line, "LP lp", "double const& lp_lam, double const& lp_phi, double& xy_x, double& xy_y");
                    boost::replace_first(line, "XY", "void");
                }
                else if (boost::starts_with(trimmed, "LP ") && boost::contains(trimmed, "("))
                {
                    boost::replace_all(line, "XY xy", "double const& xy_x, double const& xy_y, double& lp_lam, double& lp_phi");
                    boost::replace_first(line, "LP", "void");
                }
                boost::replace_all(line, "lp.lam", "lp_lam");
                boost::replace_all(line, "lp.phi", "lp_phi");

                boost::replace_all(line, "xy.x", "xy_x");
                boost::replace_all(line, "xy.y", "xy_y");

                boost::replace_all(line, "return xy;", "");
                boost::replace_all(line, "return lp;", "");
                boost::replace_all(line, "XY xy;", "");
                boost::replace_all(line, "LP lp;", "");

//                TODO:
//                starts with xy = ---> replace parameters
            }
        }

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_HEALPIX_HPP
