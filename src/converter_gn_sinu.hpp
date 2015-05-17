#ifndef TISSOT_CONVERTER_GN_SINU_HPP
#define TISSOT_CONVERTER_GN_SINU_HPP

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



class converter_cpp_bg_gn_sinu : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_gn_sinu(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            // Remove ellipsoid model from all but "sinu"
            // because in all other methods, spherical is assumed
            // (setup(P) is unconditionally called and 'for spheres, only'

            BOOST_FOREACH(derived& der, m_prop.derived_projections)
            {
                if (der.name != "sinu")
                {
                    functor_equals_in_named_struct eq("ellipsoid");

                    der.models.erase
                        (
                            std::remove_if(der.models.begin(),
                                der.models.end(),eq),
                            der.models.end()
                        );

                    // Check and clear condition
                    BOOST_ASSERT(der.models.size() == 1u);
                    der.models.front().condition.clear();
                }
            }

        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_GN_SINU_HPP

