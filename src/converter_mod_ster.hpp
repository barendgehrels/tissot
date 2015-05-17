#ifndef TISSOT_CONVERTER_MOD_STER_HPP
#define TISSOT_CONVERTER_MOD_STER_HPP

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


class converter_cpp_bg_mod_ster : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_mod_ster(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    boost::replace_all(line,
                    "double den, rh, z, sinz, cosz, chi, phi",
                    "double den, rh = 0, z, sinz = 0, cosz = 0, chi, phi = 0");
                }
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_MOD_STER_HPP
