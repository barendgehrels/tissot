#ifndef TISSOT_CONVERTER_AITOFF_HPP
#define TISSOT_CONVERTER_AITOFF_HPP

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


class converter_cpp_bg_aitoff : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_aitoff(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            // Remove M_PI and M_PI_2
            if (m_prop.inlined_functions.size() == 4u
                && m_prop.inlined_functions.front() == "#ifndef M_PI")
            {
                m_prop.inlined_functions.clear();
            }
            functor_equals_in_named_struct eq("M_PI", "M_PI_2");

            m_prop.defined_consts.erase
                (
                    std::remove_if(m_prop.defined_consts.begin(),
                        m_prop.defined_consts.end(),eq),
                    m_prop.defined_consts.end()
                );

            // Replace return (lp), we don't return values in fwd/inv
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    boost::replace_all(line, "return (lp)", "return");
                }
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_AITOFF_HPP
