#ifndef TISSOT_CONVERTER_AEQD_HPP
#define TISSOT_CONVERTER_AEQD_HPP

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


class converter_cpp_bg_aeqd : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_aeqd(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            BOOST_FOREACH(derived& der, m_prop.derived_projections)
            {
                if (der.models.size() == 3)
                {
                    m_prop.setup_extra_code.push_back("bool const guam = pj_param(par.params, \"bguam\").i;");
                    m_prop.setup_extra_code.push_back("");

                    // Still ordered alphabetically
                    BOOST_ASSERT(der.models[0].name == "ellipsoid");
                    der.models[0].condition = "if (par.es && ! guam)";

                    BOOST_ASSERT(der.models[1].name == "guam");
                    der.models[1].condition = "else if (par.es && guam)";

                    BOOST_ASSERT(der.models[2].name == "spheroid");
                    der.models[2].condition = "else";
                }
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_AEQD_HPP
