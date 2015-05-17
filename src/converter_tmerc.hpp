#ifndef TISSOT_CONVERTER_TMERC_HPP
#define TISSOT_CONVERTER_TMERC_HPP

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


class converter_cpp_bg_tmerc : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_tmerc(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            m_prop.extra_impl_includes.insert("function_overloads.hpp");

            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    boost::replace_all(line, "floor", "int_floor");
                }
            }
            BOOST_FOREACH(derived& d, m_prop.derived_projections)
            {
                BOOST_FOREACH(std::string& line, d.constructor_lines)
                {
                    boost::replace_all(line, "floor", "int_floor");
                }

                // Remove the throw-clause for UTM to enable the spherical model
                // (otherwise it is specified, but invalid)
                // TODO: reconsider this. Is UTM with sphere valid? For distances, it gives the correct result
                std::vector<std::string>& lines = d.constructor_lines;
                lines.erase
                    (
                        std::remove_if(lines.begin(), lines.end(),
                                functor_starts_with("if (!par.es) throw")),
                        lines.end()
                    );
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_TMERC_HPP
