#ifndef TISSOT_CONVERTER_OB_TRAN_HPP
#define TISSOT_CONVERTER_OB_TRAN_HPP

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
#include <boost/assert.hpp>


namespace boost { namespace geometry { namespace proj4converter
{


class converter_cpp_bg_ob_tran : public converter_cpp_bg_default
{
    public :
        converter_cpp_bg_ob_tran(projection_properties& prop)
            : m_prop(prop)
        {
        }

        void convert()
        {
            // ob_tran is a projection forwarding to another projection
            m_prop.template_struct = "<Geographic, Cartesian>";
            m_prop.forward_declarations = "template <typename Geographic, typename Cartesian, typename Parameters> class factory;";

            m_prop.setup_return_type = "double";
            m_prop.setup_extra_parameters = ", bool create = true";

            m_prop.setup_extra_code.push_back("detail::ob_tran::par_ob_tran<Geographic, Cartesian> proj_parm;");
            m_prop.setup_extra_code.push_back("Parameters p = par;");
            m_prop.setup_extra_code.push_back("double phip = setup_ob_tran(p, proj_parm, false);");
            m_prop.setup_extra_code.push_back("");

            BOOST_FOREACH(derived& der, m_prop.derived_projections)
            {
                std::vector<std::string>& lines = der.constructor_lines;
                BOOST_FOREACH(std::string& line, lines)
                {
                    boost::replace_all(line, "proj_parm.link->", "pj.");
                    boost::replace_all(line, "char *name, *s;", "Parameters pj;");
                    if (boost::contains(line, "so_proj"))
                    {
                        // Replace assignment completely
                        line = tab1 + "pj.name = pj_param(par.params, \"so_proj\").s;";
                    }
                }

                // Remove some lines comparing projection name
                functor_starts_with functor("int i;");
                functor.add("for (i = 0;")
                    .add("(s = pj_list[i].id")
                    .add("++i)")
                    .add("if (!s ||")
                    .add("freeup(P);")
                    .add("return 0;")
                    .add("par.inv =") // with a conditional expression
                ;
                lines.erase
                    (
                        std::remove_if(lines.begin(), lines.end(),functor),
                        lines.end()
                    );

                // Replace creation part
                {
                    functor_starts_with functor("if (!(proj_parm.link");
                    std::vector<std::string>::iterator it
                        = std::find_if(lines.begin(), lines.end(), functor);
                    if (it != lines.end())
                    {
                        std::vector<std::string> piece;

                        piece.push_back("");
                        piece.push_back(tab1 + "if (create)");
                        piece.push_back(tab1 + "{");
                        piece.push_back(tab2 + "factory<Geographic, Cartesian, Parameters> fac;");
                        piece.push_back(tab2 + "proj_parm.link.reset(fac.create_new(pj));");
                        piece.push_back(tab2 + "if (! proj_parm.link.get()) throw proj_exception(-26);");
                        // closing curly brace is already there

                        // Replace the original expression with this piece
                        lines.insert(it, piece.begin(), piece.end());

                        // Find expression again to erase it
                        it = std::find_if(lines.begin(), lines.end(), functor);
                        lines.erase(it);
                    }
                }

                lines.push_back(tab1 + "// return phip to choose model");
                lines.push_back(tab1 + "return phip;");


                if (der.models.size() == 2u)
                {
                    BOOST_ASSERT(der.models[0].name == "oblique");
                    der.models[0].condition = "if (fabs(phip) > detail::ob_tran::TOL)";

                    BOOST_ASSERT(der.models[1].name == "transverse");
                    der.models[1].condition = "else";
                }
            }

            BOOST_FOREACH(projection& proj, m_prop.projections)
            {
                functor_nospaces_equals functor("(void)xy;");
                proj.lines.erase
                    (
                        std::remove_if(proj.lines.begin(), proj.lines.end(), functor),
                        proj.lines.end()
                    );

                BOOST_FOREACH(std::string& line, proj.lines)
                {
                    boost::replace_all(line, ", this->m_proj_parm.link", "");

                    std::string s = boost::trim_copy(line);
                    if (boost::starts_with(s, "lp ="))
                    {
                        boost::replace_all(line, "this->", "");
                        boost::replace_all(line, "lp = ", "");
                        boost::replace_all(line, "xy", "xy_x, xy_y, lp_lon, lp_lat");
                    }
                    else if (boost::starts_with(s, "return (this->"))
                    {
                        boost::replace_all(line, "return (this->", "");
                        boost::replace_last(line, ")", "");
                        boost::replace_all(line, "lp", "lp_lon, lp_lat, xy_x, xy_y");
                    }
                }
            }

            BOOST_FOREACH(std::string& line, m_prop.proj_parameters)
            {
                boost::replace_all(line, "struct PJconsts *", "boost::shared_ptr<projection<Geographic, Cartesian> > ");
            }
        }

    private :

        projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_OB_TRAN_HPP
