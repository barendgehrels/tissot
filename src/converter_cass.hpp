#ifndef TISSOT_CONVERTER_CASS_HPP
#define TISSOT_CONVERTER_CASS_HPP

// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "tissot_structs.hpp"
#include "tissot_common.hpp"
#include "tissot_util.hpp"
#include "converter_base.hpp"


namespace boost { namespace geometry { namespace proj4converter
{


struct remove_cass_parameters
{
    inline bool operator()(std::string const& item) const
    {
        bool keep = boost::contains(item, "m0")
            || boost::contains(item, "EN_SIZE");
        return !keep;
    }
private:
    std::vector<std::string> tokens;
};

class converter_cpp_bg_cass : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_cass(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            change_proj_parameters();

            // Fix usage of mutable variables
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                add_declarations(proj);
            }
        }

    private :

        void add_declarations(projection& proj)
        {
            bool a1 = false;
            bool a2 = false;
            bool dd = false;
            bool d2 = false;
            bool n = false;
            bool r = false;
            bool tn = false;
            bool t = false;
            bool c = false;

            if (proj.direction == "forward")
            {
                n = move_expression(proj, "n = sin(lp_lat)", "n");
                c = move_expression(proj, "c = cos(lp_lat)", "c");
            }
            else
            {
                dd = move_expression(proj, "dd = xy_y + this->m_par.phi0", "dd");
            }

            BOOST_FOREACH(std::string& line, proj.lines)
            {
                add_declaration(line, a1, "a1");
                add_declaration(line, a2, "a2");
                add_declaration(line, n, "n");
                add_declaration(line, r, "r");
                add_declaration(line, t, "t");
                add_declaration(line, tn, "tn");
                add_declaration(line, dd, "dd");
                add_declaration(line, d2, "d2");
            }
        }


        void change_proj_parameters()
        {
            // Changes proj parameters into local variables, all but m0 and en
            // Because we have fwd/inv as const methods

            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    // Save
                    boost::replace_all(line, "this->m_proj_parm.en", "PROJ.en");
                    boost::replace_all(line, "this->m_proj_parm.m0", "PROJ.m0");

                    // Replace
                    boost::replace_all(line, "this->m_proj_parm.", "");

                    // Restore
                    boost::replace_all(line, "PROJ.en", "this->m_proj_parm.en");
                    boost::replace_all(line, "PROJ.m0", "this->m_proj_parm.m0");
                }
            }

            m_prop.proj_parameters.erase
                (
                    std::remove_if(m_prop.proj_parameters.begin(),
                        m_prop.proj_parameters.end(), remove_cass_parameters()),
                    m_prop.proj_parameters.end()
                );
        }

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_CASS_HPP
