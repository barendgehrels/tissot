#ifndef TISSOT_CONVERTER_OMERC_HPP
#define TISSOT_CONVERTER_OMERC_HPP

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


class converter_cpp_bg_omerc : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_omerc(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            // Initialize variable (for gcc warning)
            // (actually there are many more uninitialized - we should possibly
            // initialize all of them, TODO)
            BOOST_FOREACH(derived& der, m_prop.derived_projections)
            {
                BOOST_FOREACH(std::string& line, der.constructor_lines)
                {
                    boost::replace_all(line, ", alpha_c;", ", alpha_c=0.0;");
                }
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_OMERC_HPP
