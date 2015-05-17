#ifndef TISSOT_CONVERTER_LCCA_HPP
#define TISSOT_CONVERTER_LCCA_HPP

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


class converter_cpp_bg_lcca : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_lcca(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            BOOST_FOREACH(derived& d, m_prop.derived_projections)
            {
                d.constructor_lines.push_back(tab1 + "boost::ignore_unused(tan20);");
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_LCCA_HPP
