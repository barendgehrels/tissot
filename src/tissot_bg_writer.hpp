#ifndef TISSOT_WRITER_HPP
#define TISSOT_WRITER_HPP

// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/foreach.hpp>
#include <sstream>


namespace boost { namespace geometry { namespace proj4converter
{



class proj4_writer_cpp_bg
{
    public :
        proj4_writer_cpp_bg(projection_properties& projpar
                , std::string const& group
                , std::vector<epsg_entry> const& epsg_entries
                , std::ostream& str)
            : m_projpar(projpar)
            , stream(str)
            , m_epsg_entries(epsg_entries)
            , projection_group(group)
            , hpp("BOOST_GEOMETRY_PROJECTIONS_" + boost::to_upper_copy(projection_group) + "_HPP")
        {
        }

        void write()
        {
            stream << "#ifndef " << hpp << std::endl
                << "#define " << hpp << std::endl
                << std::endl;

            write_copyright();
            write_header();
            write_begin_impl();
            {
                write_consts();
                write_extra_structs();
                if (m_projpar.parstruct_first)
                {
                    write_proj_par_struct();
                }
                write_prefix();
                if (! m_projpar.parstruct_first)
                {
                    write_proj_par_struct();
                }
                write_impl_classes();
                write_postfix();
                write_setup();
            }
            write_end_impl();

            write_classes();

            write_wrappers();
            write_end();
        }

    private :

        std::ostream& stream;

        void write_copyright()
        {
            std::ifstream cr_file ("../src/tissot_bg_copyright_header.txt");
            if (cr_file.is_open())
            {
                while (! cr_file.eof() )
                {
                    std::string line;
                    std::getline(cr_file, line);

                    stream << line << std::endl;
                }
                cr_file.close();
            }
            stream << std::endl;
        }
        void write_header()
        {
            BOOST_FOREACH(std::string const& s, m_projpar.extra_includes)
            {
                stream << "#include <" << s << ">"  << std::endl;
            }
            if (! m_projpar.extra_includes.empty())
            {
                stream << std::endl;
            }

            stream
                << include_projections << "/impl/base_static.hpp>" << std::endl
                << include_projections << "/impl/base_dynamic.hpp>" << std::endl
                << include_projections << "/impl/projects.hpp>" << std::endl
                << include_projections << "/impl/factory_entry.hpp>" << std::endl
                ;

            BOOST_FOREACH(std::string const& s, m_projpar.extra_proj_includes)
            {
                stream << include_projections << "/proj/" << s << ">"  << std::endl;
            }

            if (projection_group == "robin"
                || projection_group == "tmerc")
            {
                stream << include_projections << "/impl/function_overloads.hpp>" << std::endl;
            }

            BOOST_FOREACH(std::string const& s, m_projpar.extra_impl_includes)
            {
                stream << include_projections << "/impl/" << s << ">"  << std::endl;
            }

            if (use_epsg())
            {
                stream << std::endl;
                stream << include_projections << "/epsg_traits.hpp>" << std::endl;
            }

            stream << std::endl;

            stream << "namespace boost { namespace geometry { namespace projections" << std::endl
                << "{" << std::endl;

            if (! m_projpar.forward_declarations.empty())
            {
                stream << std::endl << tab1 << m_projpar.forward_declarations << std::endl << std::endl;
            }
        }

        void write_begin_impl()
        {

            stream
                << tab1 << "#ifndef DOXYGEN_NO_DETAIL" << std::endl
                << tab1 << "namespace detail { namespace " << projection_group << "{" << std::endl;
        }

        void write_extra_structs()
        {
            BOOST_FOREACH(std::string const& s, m_projpar.extra_structs)
            {
                stream << tab3 << s << std::endl;
            }
            if (! m_projpar.extra_structs.empty())
            {
                stream << std::endl;
            }

            std::string ts = m_projpar.template_struct;
            if (! ts.empty())
            {
                stream << tab3 << "template <";
                if (ts == "<Cartesian>")
                {
                    stream  << "typename Cartesian";
                }
                else if (ts == "<Geographic, Cartesian>")
                {
                    stream << "typename Geographic, typename Cartesian";
                }
                else if (ts == "<Geographic, Cartesian, Parameters>")
                {
                    stream << "typename Geographic, typename Cartesian, typename Parameters";
                }
                stream << ">" << std::endl;
            }
        }

        void write_proj_par_struct()
        {
            if (! m_projpar.proj_parameters.empty())
            {
                stream
                    << tab3 << "struct par_" << projection_group << std::endl
                    << tab3 << "{" << std::endl;
                for (std::vector<std::string>::const_iterator it = m_projpar.proj_parameters.begin();
                    it != m_projpar.proj_parameters.end(); 
                    ++it)
                {
                    stream << tab4  << *it << std::endl;
                }
                stream << tab3 << "};" << std::endl << std::endl;
            }
        }

        void write_consts()
        {
            BOOST_FOREACH(macro_or_const const& con, m_projpar.defined_consts)
            {
                std::string type = "double";

                // Check if it is a real integer
                {
                    int value = atoi(con.value.c_str());
                    std::ostringstream out;
                    out << value;
                    if (out.str() == con.value)
                    {
                        type = "int";
                    }
                }
                stream << tab3 << "static const " << type << " " << con.name << " = " << con.value << ";" << std::endl;
            }
            stream << std::endl;

            BOOST_FOREACH(macro_or_const const& macro, m_projpar.defined_macros)
            {
                stream << tab3 << "#define " << macro.name << " " << macro.value << std::endl;
            }
            if (! m_projpar.defined_macros.empty())
            {
                stream << std::endl;
            }
        }

        std::string preceded(std::string const& tab, std::string const& line) const
        {
            if (line.empty())
            {
                return line;
            }
            return tab + line;
        }

        void write_prefix()
        {
            BOOST_FOREACH(std::string const& line, m_projpar.inlined_functions)
            {
                stream << preceded(tab3, line) << std::endl;
            }
            if (! m_projpar.inlined_functions.empty())
            {
                stream << std::endl;
            }
        }

        void write_postfix()
        {
            if (m_projpar.setup_functions.size() > 0 && ! m_projpar.setup_function_line.empty() )
            {
                if (! m_projpar.proj_parameters.empty())
                {
                    // Modify the setup function: add project parameter
                    std::string tag = "Parameters& par";
                    std::string::size_type loc = m_projpar.setup_function_line.find(tag);
                    if (loc != std::string::npos)
                    {
                        m_projpar.setup_function_line.insert(loc + tag.length(),
                                ", par_" + projection_group + "& proj_parm");
                    }
                }

                stream
                    << tab3 << "template <typename Parameters>" << std::endl
                    << tab3 << m_projpar.setup_function_line << std::endl
                    << tab3 << "{" << std::endl;

                BOOST_FOREACH(std::string const& line, m_projpar.setup_functions)
                {
                    stream << preceded(tab3, line) << std::endl;
                }
                stream << std::endl << std::endl;
            }
        }

        void write_impl_classes()
        {
            std::string current_model;
            std::string current_subgroup;
            for (size_t i = 0; i < m_projpar.projections.size(); i++)
            {
                projection const& proj = m_projpar.projections[i];


                if (proj.model != current_model || proj.subgroup != current_subgroup)
                {
                    std::string name = "base_" + proj.subgroup + "_" + proj.model;

                    std::string tbase = "base_t_f";
                    if (proj.has_inverse)
                    {
                        tbase += "i"; // base_fi
                    }

                    tbase += "<" + name + "<Geographic, Cartesian, Parameters>,"
                        + "\n" + tab5 + " Geographic, Cartesian, Parameters>";


                    std::string mut;
                    if (projection_group == "lcc"
                        || projection_group == "aea"
                        || projection_group == "cass"
                        || projection_group == "eqdc"
                        )
                    {
                        mut = "mutable ";
                    }

                    stream
                        << tab3 << "// template class, using CRTP to implement forward/inverse" << std::endl
                        << tab3 << "template <typename Geographic, typename Cartesian, typename Parameters>" << std::endl
                        << tab3 << "struct " << name << " : public " << tbase
                        << std::endl
                        << tab3 << "{" << std::endl << std::endl;

                    // for GCC (probably standard) typedefs again are necessary
                    stream
                        //<< tab4 << "typedef typename " << tbase << "::geographic_type geographic_type;" << std::endl
                        //<< tab4 << "typedef typename " << tbase << "::cartesian_type cartesian_type;" << std::endl
                        << tab4 << " typedef double geographic_type;" << std::endl
                        << tab4 << " typedef double cartesian_type;" << std::endl
                        << std::endl;


                    // optional project specific parameter variable
                    if (! m_projpar.proj_parameters.empty())
                    {
                        stream << tab4 << mut << "par_" << projection_group
                            << m_projpar.template_struct << " m_proj_parm;" << std::endl;
                    }

                    stream << std::endl
                        // constructor
                        << tab4 << "inline " << name << "(const Parameters& par)" << std::endl
                        << tab5 << ": " << tbase << "(*this, par)";

                    BOOST_FOREACH(std::string const& s,
                        m_projpar.extra_member_initialization_list)
                    {
                        stream << ", " << s;
                    }
                    stream << " {}" << std::endl << std::endl;
                }

                if (proj.model != current_model)
                {
                    BOOST_FOREACH(std::string const& s, proj.preceding_lines)
                    {
                        stream << tab4 << s << std::endl;
                    }
                    if (! proj.preceding_lines.empty())
                    {
                        stream << std::endl;
                    }
                }

                if (proj.direction == "special_factors")
                {
                    stream << tab4 << "#ifdef SPECIAL_FACTORS_NOT_CONVERTED" << std::endl;
                }

                stream << tab4 << "inline void ";
                if (proj.direction == "forward")
                {
                    stream << "fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y";
                }
                else if (proj.direction == "inverse")
                {
                    stream << "inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat";
                }
                else if (proj.direction == "special_factors")
                {
                    stream
                        << "fac(Geographic lp, Factors &fac";
                }
                else
                {
                    stream << proj.direction << "(";
                }

                stream << ") const" << std::endl
                    << tab4 << "{" << std::endl;

                for (size_t j = 0; j < proj.lines.size(); j++)
                {
                    stream << preceded(tab4, proj.lines[j]) << std::endl;
                }
                stream << tab4 << "}" << std::endl;

                BOOST_FOREACH(std::string const& line, proj.trailing_lines)
                {
                    stream << preceded(tab4, line) << std::endl;
                }

                if (proj.direction == "special_factors")
                {
                    stream << tab4 << "#endif" << std::endl;
                }

                // End of class
                if (i == m_projpar.projections.size() - 1 || m_projpar.projections[i + 1].model != m_projpar.projections[i].model)
                {
                    stream << tab3 << "};" << std::endl;
                }
                stream << std::endl;

                current_model = proj.model;
                current_subgroup = proj.subgroup;
            }
        }


        bool use_epsg() const
        {
            BOOST_FOREACH(derived const& der, m_projpar.derived_projections)
            {
                BOOST_FOREACH(epsg_entry const& entry, m_epsg_entries)
                {
                    if (entry.prj_name == der.name)
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        void write_setup()
        {
            BOOST_FOREACH(derived const& der, m_projpar.derived_projections)
            {

                stream
                    << tab3 << "// " << der.description << std::endl
                    << tab3 << "template <";
                std::string ts = m_projpar.template_struct;
                if (! ts.empty())
                {
                    stream << "typename ";
                    if (ts == "<Cartesian>") stream << "Cartesian";
                    else if (ts == "<Geographic, Cartesian>" || ts == "<Geographic, Cartesian, Parameters>") stream << "Geographic, typename Cartesian";
                    stream << ", ";
                }

                stream << "typename Parameters>" << std::endl
                    << tab3
                    << (m_projpar.setup_return_type.empty() ? "void" : m_projpar.setup_return_type)
                    << " setup_" << der.name << "(Parameters& par";

                if (! m_projpar.proj_parameters.empty())
                {
                    stream << ", par_" << projection_group << m_projpar.template_struct << "& proj_parm";
                }
                stream << m_projpar.setup_extra_parameters << ")" << std::endl
                    << tab3 << "{" << std::endl;

                BOOST_FOREACH(std::string const& s, der.constructor_lines)
                {
                    std::string line = s;
                    if (boost::starts_with(boost::trim_copy(line), "setup")
                        && ! m_projpar.proj_parameters.empty())
                    {
                        // Insert second parameter if necessary.
                        boost::replace_all(line, "par,", "par, proj_parm,");
                        boost::replace_all(line, "par)", "par, proj_parm)");
                        if (boost::starts_with(line, "setup"))
                        {
                            line = tab1 + line;
                        }
                    }
                    stream << preceded(tab3, line) << std::endl;
                }
                stream << tab3 << "}" << std::endl << std::endl;
            }
        }

        void write_end_impl()
        {
            stream << tab2 << "}} // namespace detail::" << projection_group << std::endl
                << tab1 << "#endif // doxygen" << std::endl
                << std::endl;
        }
        void write_end()
        {
            stream
                << "}}} // namespace boost::geometry::projections" << std::endl << std::endl
                << "#endif // " << hpp << std::endl << std::endl;
        }

        void write_classes()
        {
            BOOST_FOREACH(derived const& der, m_projpar.derived_projections)
            {
                BOOST_FOREACH(model const& mod, der.models)
                {
                    std::string name = der.name + "_" + mod.name;

                    if (m_projpar.valid)
                    {
                        std::string base = "detail::" + projection_group
                            + "::base_" + projection_group + "_" + mod.name + "<Geographic, Cartesian, Parameters>";

                        // Doxygen comments
                        stream
                            << tab1 << "/*!" << std::endl
                            << tab2 << "\\brief " << der.description << " projection" << std::endl
                            << tab2 << "\\ingroup projections" << std::endl
                            << tab2 << "\\tparam Geographic latlong point type" << std::endl
                            << tab2 << "\\tparam Cartesian xy point type" << std::endl
                            << tab2 << "\\tparam Parameters parameter type" << std::endl
                        ;

                        if (! der.parsed_characteristics.empty())
                        {
                            stream << tab2 << "\\par Projection characteristics" << std::endl;
                            BOOST_FOREACH(std::string const& ch, der.parsed_characteristics)
                            {
                                 stream << tab2 << " - " << ch << std::endl;
                            }
                        }
                        if (! der.parsed_parameters.empty())
                        {
                            stream << tab2 << "\\par Projection parameters" << std::endl;
                            BOOST_FOREACH(parameter const& p, der.parsed_parameters)
                            {
                                 stream << tab2 << " - " << p.name;
                                 if (! p.explanation.empty())
                                 {
                                     stream << ": " << p.explanation;
                                 }
                                 if (! p.type.empty())
                                 {
                                     stream << " (" << p.type << ")";
                                 }
                                 stream << std::endl;
                            }
                        }

                        stream
                            << tab2 << "\\par Example" << std::endl
                            << tab2 << "\\image html ex_" << der.name << ".gif" << std::endl
                            << tab1 << "*/" << std::endl;

                        // Class itself
                        stream
                            << tab1 << "template <typename Geographic, typename Cartesian, typename Parameters = parameters>" << std::endl
                            << tab1 << "struct " << name
                            << " : public " << base << std::endl
                            << tab1 << "{"  << std::endl
                            << tab2 << "inline " << name << "(const Parameters& par) : " << base << "(par)" << std::endl
                            << tab2 << "{" << std::endl
                            << tab3 << "detail::" << projection_group << "::setup_" << der.name << "(this->m_par";
                        if (! m_projpar.proj_parameters.empty())
                        {
                            stream << ", this->m_proj_parm";
                        }
                        stream << ");" << std::endl
                            << tab2 << "}" << std::endl
                            << tab1 << "};" << std::endl
                            << std::endl;
                    }
                }
            }
        }

        void write_wrappers()
        {
            stream
                << tab1 << "#ifndef DOXYGEN_NO_DETAIL" << std::endl
                << tab1 << "namespace detail" << std::endl
                << tab1 << "{" << std::endl << std::endl;

            std::string templates = "template <typename Geographic, typename Cartesian, typename Parameters>";


            // TEMP: get model from below
            // TODO: get model from epsg-parameter
            std::string epsg_model = "";

            // Create factory entries
            // This complicated piece has
            // - to decide to take either "ellipsoid" or "spheroid" based on the input parameter
            // - to decide if it is the forward or forward/reverse model

            stream << tab2 << "// Factory entry(s)" << std::endl;
            BOOST_FOREACH(derived const& der, m_projpar.derived_projections)
            {
                stream << tab2 << templates << std::endl
                    << tab2 << "class " << der.name << "_entry : public detail::factory_entry<Geographic, Cartesian, Parameters>" << std::endl
                    << tab2 << "{" << std::endl
                    << tab3 << "public :" << std::endl
                    << tab4 << "virtual projection<Geographic, Cartesian>* create_new(const Parameters& par) const" << std::endl
                    << tab4 << "{" << std::endl;

                if (! m_projpar.setup_extra_code.empty())
                {
                    BOOST_FOREACH(std::string const& s, m_projpar.setup_extra_code)
                    {
                        stream << preceded(tab5, s) << std::endl;
                    }
                }

                std::string tab = tab5;
                // Add tab and check of models are correct (TODO: move to separate check source)
                if (der.models.size() > 1u)
                {
                    tab += tab1;
                    BOOST_FOREACH(model const& mod, der.models)
                    {
                        BOOST_ASSERT(! mod.condition.empty());
                    }
                }
                else
                {
                    BOOST_FOREACH(model const& mod, der.models)
                    {
                        BOOST_ASSERT(mod.condition.empty());
                    }
                }

                BOOST_FOREACH(model const& mod, der.models)
                {
#if 0
                    if (m_projpar.has_spheroid && m_projpar.has_ellipsoid && m_projpar.has_guam)
                    {
                        stream << tab5;
                        switch(m)
                        {
                            case 0 : stream << "if (! par.es)"; break;
                            case 1 : stream << "else if (pj_param(par.params, \"bguam\").i)"; break;
                            case 2 : stream << "else"; break;
                        }
                        stream << std::endl;
                    }
                    else if (m == 1 && do_else)
                    {
                        stream << tab5 << "else" << std::endl;
                    }
#endif

                    if (! mod.condition.empty())
                    {
                        stream << tab5 << mod.condition << std::endl;
                    }

                    std::string base = "base_v_f";
                    if (mod.has_inverse)
                    {
                        base += "i";
                    }
                    std::string name = der.name + "_" + mod.name;
                    stream << tab << "return new " << base
                        << "<" << name << "<Geographic, Cartesian, Parameters>, Geographic, Cartesian, Parameters>(par);" << std::endl;

                    if (epsg_model.empty())
                    {
                        epsg_model = mod.name;
                    }
                }
                stream
                    << tab4 << "}" << std::endl
                    << tab2 << "};" << std::endl << std::endl;

            }

            // Create "PRJ_init" function for registration at factory
            stream << tab2 << templates << std::endl
                << tab2 << "inline void " << projection_group << "_init(detail::base_factory<Geographic, Cartesian, Parameters>& factory)" << std::endl
                << tab2 << "{" << std::endl;
            BOOST_FOREACH(derived const& der, m_projpar.derived_projections)
            {
                stream << tab3 << "factory.add_to_factory(\"" << der.name << "\", new "
                    << der.name << "_entry<Geographic, Cartesian, Parameters>);" << std::endl;
            }
            stream << tab2 << "}" << std::endl << std::endl;

            stream << tab1 << "} // namespace detail" << std::endl;


            // Create EPSG specializations
            if (use_epsg())
            {
                stream << tab1 << "// Create EPSG specializations" << std::endl
                    << tab1 << "// (Proof of Concept, only for some)" << std::endl
                    << std::endl;
                BOOST_FOREACH(derived const& der, m_projpar.derived_projections)
                {
                    BOOST_FOREACH(epsg_entry const& entry, m_epsg_entries)
                    {
                        if (entry.prj_name == der.name)
                        {
                            stream << tab1 << "template<typename LatLongRadian, typename Cartesian, typename Parameters>" << std::endl
                                << tab1 << "struct epsg_traits<" << entry.epsg_code << ", LatLongRadian, Cartesian, Parameters>" << std::endl
                                << tab1 << "{" << std::endl
                                // TODO, model, see above
                                << tab2 << "typedef " << der.name << "_" << epsg_model << "<LatLongRadian, Cartesian, Parameters> type;" << std::endl
                                << tab2 << "static inline std::string par()" << std::endl
                                << tab2 << "{" << std::endl
                                << tab3 << "return \"" << entry.parameters << "\";" << std::endl
                                << tab2 << "}" << std::endl
                                << tab1 << "};" << std::endl
                                << std::endl
                                << std::endl;
                        }
                    }

                }
            }

            stream
                << tab1 << "#endif // doxygen" << std::endl
                << std::endl;


        }

        projection_properties& m_projpar;
        std::vector<epsg_entry> const& m_epsg_entries;

        std::string projection_group;
        std::string hpp;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_WRITER_HPP
