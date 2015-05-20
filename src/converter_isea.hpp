#ifndef TISSOT_CONVERTER_ISEA_HPP
#define TISSOT_CONVERTER_ISEA_HPP

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


class converter_cpp_bg_isea : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_isea(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            m_prop.parstruct_first = false;

            // Remove M_PI and M_PI_2
            functor_equals_in_named_struct eq("M_PI", "M_PI_2");
            eq.add("ISEA_STATIC");

            m_prop.defined_consts.erase
                (
                    std::remove_if(m_prop.defined_consts.begin(),
                        m_prop.defined_consts.end(),eq),
                    m_prop.defined_consts.end()
                );

            std::vector<std::string>& lines = m_prop.inlined_functions;
            BOOST_FOREACH(std::string& line, lines)
            {
                boost::replace_all(line, "ISEA_STATIC", "static");

                // Structure not completely filled:
                boost::replace_all(line, "{0.0},", "{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},");
            }

            // Remove #ifndef/#endif structs (defining M_PI / static)
            {
                functor_starts_with sw("#ifndef");
                std::vector<std::string>::iterator it =  std::find_if(lines.begin(), lines.end(), sw);
                while (it != lines.end() && it + 1 != lines.end())
                {
                    if (boost::trim_copy(* (it + 1)) == "#endif")
                    {
                        lines.erase(it, it + 2);
                    }
                    else
                    {
                        break;
                    }
                    it = std::find_if(it + 1, lines.end(), sw);
                }
            }

            // Replace the *char with std::string
            BOOST_FOREACH(derived& d, m_prop.derived_projections)
            {
                BOOST_FOREACH(std::string& line, d.constructor_lines)
                {
                   boost::replace_all(line, "char *opt", "std::string opt");
                   boost::replace_all(line, "if (opt)", "if (! opt.empty())");
                   boost::replace_all(line, "!strcmp(opt,", "opt == std::string(");
                   boost::replace_all(line, "std::string( ", "std::string(");
                }
            }

            // Provide const-correctness, don't use/change parameters
            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                std::vector<std::string>& lines = proj.lines;
                std::vector<std::string>::iterator it
                    =  std::find_if(lines.begin(), lines.end(),
                            functor_starts_with("out = isea_forward"));
                if (it != lines.end())
                {
                    // First replace current line, then insert the decl before
                    boost::replace_all(*it, "&this->m_proj_parm.dgg", "&copy");
                    lines.insert(it, tab1 + "isea_dgg copy = this->m_proj_parm.dgg;");
                }
            }


            // TODO: we might replace all "struct XXX" usage with XXX (but not in definition of course)

        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_ISEA_HPP
