#ifndef TISSOT_CONVERTER_LSAT_HPP
#define TISSOT_CONVERTER_LSAT_HPP

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


class converter_cpp_bg_lsat : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_lsat(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                boost::replace_all(line, "Parameters& par)", "Parameters& par, par_lsat& proj_parm)");

                // Set translation (done in converter) to proj_parm, but not for .params and .es
                boost::replace_all(line, "par.", "proj_parm.");
                boost::replace_all(line, "this->m_proj_parm.", "proj_parm.");

                boost::replace_all(line, "proj_parm.params", "par.params");
                boost::replace_all(line, "proj_parm.es", "par.es");
            }

            // See also imw_p/sconics
            BOOST_FOREACH(derived& d, m_prop.derived_projections)
            {
                BOOST_FOREACH(std::string& line, d.constructor_lines)
                {
                    if (boost::contains(line, "seraz0"))
                    {
                        boost::replace_last(line, "P", "par, proj_parm");
                    }
                }
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_LSAT_HPP
