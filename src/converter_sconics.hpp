#ifndef TISSOT_CONVERTER_SCONICS_HPP
#define TISSOT_CONVERTER_SCONICS_HPP

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


class converter_cpp_bg_sconics : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_sconics(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                boost::replace_all(line, "Parameters& par,", "Parameters& par, par_sconics& proj_parm,");

                // Set translation (done in converter) to proj_parm, but not for .params and .es
                boost::replace_all(line, "par.", "proj_parm.");
                boost::replace_all(line, "this->m_proj_parm.", "proj_parm.");

                boost::replace_all(line, "proj_parm.params", "par.params");
                boost::replace_all(line, "proj_parm.es", "par.es");
            }

            // See also imw_p/lsat
            fix_calls(m_prop.setup_functions);
            BOOST_FOREACH(derived& d, m_prop.derived_projections)
            {
                fix_calls(d.constructor_lines);
            }
        }

    private :

        void fix_calls(std::vector<std::string>& lines)
        {
            BOOST_FOREACH(std::string& line, lines)
            {
                if (boost::contains(line, "phi12"))
                {
                    boost::replace_last(line, "P", "par, proj_parm");
                }
            }
        }

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_SCONICS_HPP
