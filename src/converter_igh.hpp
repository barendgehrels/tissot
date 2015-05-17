#ifndef TISSOT_CONVERTER_IGH_HPP
#define TISSOT_CONVERTER_IGH_HPP

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


class converter_cpp_bg_igh : public converter_cpp_bg_default
{
public :
    converter_cpp_bg_igh(projection_properties& prop)
        : m_prop(prop)
    {
    }

    void convert()
    {
        m_prop.template_struct = "<Geographic, Cartesian>";
        m_prop.extra_proj_includes.insert("gn_sinu.hpp");
        m_prop.extra_proj_includes.insert("moll.hpp");

        BOOST_FOREACH(std::string& line, m_prop.proj_parameters)
        {
            boost::replace_all(line, "struct PJconsts*", "boost::shared_ptr<projection<Geographic, Cartesian> >");
        }

        BOOST_FOREACH(derived& d, m_prop.derived_projections)
        {
            update_setup(d.constructor_lines);
            update_proj_parm(d.constructor_lines, "mutable_");
        }
        update_inlined(m_prop.inlined_functions);

        update_proj_parm(m_prop.inlined_functions, "mutable_");

        BOOST_FOREACH(projection& proj, m_prop.projections)
        {
            update_projection(proj.lines);
            update_proj_parm(proj.lines, "");
        }

    }

private :

    void update_inlined(std::vector<std::string>& lines)
    {
        functor_trimmed_equals functor("C_NAMESPACE PJ", "*pj_sinu(PJ *), *pj_moll(PJ *);");
        lines.erase
            (
                std::remove_if(lines.begin(), lines.end(),functor),
                lines.end()
            );
    }

    void update_projection(std::vector<std::string>& lines)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            if (boost::contains(line, "lp = "))
            {
                boost::replace_all(line, "lp = ", "");
                boost::replace_all(line, "xy, this->m_proj_parm.pj[z-1]", "xy_x, xy_y, lp_lon, lp_lat");
            }
            if (boost::contains(line, "xy = "))
            {
                boost::replace_all(line, "xy = ", "");
                boost::replace_all(line, "lp, this->m_proj_parm.pj[z-1]", "lp_lon, lp_lat, xy_x, xy_y");
            }
        }
    }

    void update_proj_parm(std::vector<std::string>& lines, std::string const& prefix)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            if (boost::contains(line, "proj_parm")
                && ! boost::contains(line, "fwd")
                && ! boost::contains(line, "inv"))
            {
                boost::replace_all(line, "]->", std::string("]->") + prefix + "params().");
            }
        }
    }

    void update_setup(std::vector<std::string>& lines)
    {
        // Find macro
        functor_starts_with sw("#define SETUP");
        std::vector<std::string>::iterator start = std::find_if(lines.begin(), lines.end(), sw);
        if (start != lines.end())
        {
            std::vector<std::string>::iterator end = start;
            while (end != lines.end()
                && ! boost::starts_with(boost::replace_all_copy(*end, " ", ""), "proj_parm.pj[n-1]->lam0"))
            {
                ++end;
            }
            if (end != lines.end())
            {
                ++end;

                // Move define code to a templated inlined function

                m_prop.inlined_functions.push_back("");
                m_prop.inlined_functions.push_back(std::string("// Converted from ") + boost::replace_last_copy(*start, "\\", ""));
                m_prop.inlined_functions.push_back("template <template <typename, typename, typename> class Entry, typename Parameters, typename Geographic, typename Cartesian>");
                m_prop.inlined_functions.push_back("inline void do_setup(int n, Parameters const& par, par_igh<Geographic, Cartesian>& proj_parm, double x_0, double y_0, double lon_0)");
                m_prop.inlined_functions.push_back("{");
                m_prop.inlined_functions.push_back(tab1 + "Entry<Geographic, Cartesian, Parameters> entry;");
                m_prop.inlined_functions.push_back(tab1 + "proj_parm.pj[n-1].reset(entry.create_new(par));");

                for (std::vector<std::string>::const_iterator it = start + 1;
                    it != end; ++it)
                {
                    std::string line = *it;
                    boost::replace_last(line, "\\", "");
                    if (! boost::trim_copy(line).empty()
                        && ! boost::contains(line, "##"))
                    {
                        m_prop.inlined_functions.push_back(line);
                    }
                }
                m_prop.inlined_functions.push_back("}");

                // Now erase from constructor line
                lines.erase(start, end);
            }
        }

        BOOST_FOREACH(std::string& line, lines)
        {
            if (boost::contains(line, "moll") && boost::contains(line, "SETUP"))
            {
                boost::replace_first(line, "moll", "par, proj_parm");
                boost::replace_first(line, "SETUP", "do_setup<moll_entry>");
            }
            else if (boost::contains(line, "sinu") && boost::contains(line, "SETUP"))
            {
                boost::replace_first(line, "sinu", "par, proj_parm");
                boost::replace_first(line, "SETUP", "do_setup<sinu_entry>");
            }

            // Replace LP/XY with doubles
            boost::replace_all(line, "LP lp = { 0, d4044118 };", "double lp_lam = 0, lp_phi = d4044118;");
            boost::replace_all(line, "XY xy1;", "double xy1_x, xy1_y;");
            boost::replace_all(line, "XY xy3;", "double xy3_x, xy3_y;");
            boost::replace_all(line, "xy1 =", "");
            boost::replace_all(line, "xy3 =", "");

            boost::replace_all(line, "lp, proj_parm.pj[0]", "lp_lam, lp_phi, xy1_x, xy1_y");
            boost::replace_all(line, "lp, proj_parm.pj[2]", "lp_lam, lp_phi, xy3_x, xy3_y");
            boost::replace_all(line, ".y", "_y");
        }
    }

    projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_IGH_HPP
