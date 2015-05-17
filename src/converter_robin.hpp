#ifndef TISSOT_CONVERTER_ROBIN_HPP
#define TISSOT_CONVERTER_ROBIN_HPP

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


class converter_cpp_bg_robin : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_robin(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            m_prop.extra_impl_includes.insert("function_overloads.hpp");

            // Replace two macro's by inline functions (below)
            // They have to be replaced because V is used in Boost Macro's

            if (m_prop.defined_macros.size() == 2u)
            {
                m_prop.defined_macros.clear();
            }

//            functor_equals_in_define eq("V(C,z)", "DV(C,z)");

//            m_prop.defined_consts.erase
//                (
//                    std::remove_if(m_prop.defined_consts.begin(),
//                        m_prop.defined_consts.end(),eq),
//                    m_prop.defined_consts.end()
//                );



            BOOST_FOREACH(std::string& line, m_prop.inlined_functions)
            {
                boost::replace_all(line, "float c0", "double c0");
            }
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    boost::replace_all(line, "floor", "int_floor");
                }
                if (proj.direction == "forward")
                {
                    proj.preceding_lines.push_back("inline double  V(COEFS const& C, double z) const");
                    proj.preceding_lines.push_back("{ return (C.c0 + z * (C.c1 + z * (C.c2 + z * C.c3))); }");
                    proj.preceding_lines.push_back("inline double DV(COEFS const& C, double z) const");
                    proj.preceding_lines.push_back("{ return (C.c1 + z * (C.c2 + C.c2 + z * 3. * C.c3)); }");
                }
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_ROBIN_HPP
