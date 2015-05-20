#ifndef TISSOT_CONVERTER_CHAMB_HPP
#define TISSOT_CONVERTER_CHAMB_HPP

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


class converter_cpp_bg_chamb : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_chamb(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            // Remove old definition of struct VECT (we could omit that too!)
            std::vector<std::string>& lines = m_prop.inlined_functions;
            lines.erase
                (
                    std::remove_if(lines.begin(), lines.end(),
                            functor_starts_with("typedef struct")),
                    lines.end()
                );

            m_prop.extra_structs.push_back("// specific for 'chamb'");
            m_prop.extra_structs.push_back("struct VECT { double r, Az; };");
            m_prop.extra_structs.push_back("struct XY { double x, y; };");

            // Fix assignment of xy to xy_x, xy_y
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    std::string term = boost::trim_copy(line);
                    if (boost::starts_with(term, "xy ="))
                    {
                        boost::replace_first(term, "xy =", "");
                        boost::replace_last(term, ";", "");
                        boost::trim(term);
                        line = tab2 + "{ xy_x = " + term + ".x; xy_y = " + term + ".y; }";
                    }
                }
            }

            adapt_documentation();
        }

    private :

        void adapt_documentation()
        {
            BOOST_FOREACH(derived& der, m_prop.derived_projections)
            {
                BOOST_FOREACH(parameter& p, der.parsed_parameters)
                {
                    // lat_1 -> Latitude of control point 1
                    if (boost::starts_with(p.name, "lat_")
                     || boost::starts_with(p.name, "lon_"))
                    {
                        p.explanation = p.name;
                        boost::replace_all(p.explanation , "lat", "Latitude");
                        boost::replace_all(p.explanation , "lon", "Longitude");
                        boost::replace_all(p.explanation, "_", " of control point ");
                        p.processed = true;
                        p.used = true;
                        p.type = "degrees";
                    }
                }
            }
        }

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_CHAMB_HPP

