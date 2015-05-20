#ifndef TISSOT_PARSER_HPP
#define TISSOT_PARSER_HPP

// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

#include "tissot_util.hpp"

namespace boost { namespace geometry { namespace proj4converter
{

class proj4_parser
{
    public :
        proj4_parser(projection_properties& prop
                , std::string const& filename
                , std::string const& group
                , std::vector<epsg_entry> const& epsg_entries)
            : m_prop(prop)
            , m_epsg_entries(epsg_entries)
            , projection_group(group)
            , skip_for_setup(false)
            , skip_lineno(0)
            , lineno(0)
        {
            parse(filename);
        }


    private :

        // TODO: replacements will go to tissot_converter.hpp
        // Replaces "P->" with our expression "par."
        void replace_pars(std::string& line, std::string const& prefix = "")
        {
            if (boost::contains(line, "->"))
            {
                boost::replace_all(line, "fac->", prefix + "fac.");
                boost::replace_all(line, "P->", prefix + "par.");
                boost::replace_all(line, "P -> ", prefix + "par.");
                // Refer to project-specific parameters
                for (std::vector<std::string>::const_iterator it = proj_parameter_names.begin();
                    it != proj_parameter_names.end();
                    ++it)
                {
                    boost::replace_all(line, "par." + *it, "proj_parm." + *it);
                }
                // If the projection-parameter was a "p", it has replaced par.params/par.phi0 as well... Correct this
                boost::replace_all(line, "proj_parm.params", "par.params");
                boost::replace_all(line, "proj_parm.phi0", "par.phi0");
                // Same for lam0
                boost::replace_all(line, "proj_parm.lam0", "par.lam0");
            }

        }

        // TODO: replacements will go to tissot_converter.hpp
        bool replace_in_entry(std::string& line)
        {
            replace_pars(line);
            replace_return(line, "P");

            return true;
        }

        void replace_macros(std::string& line)
        {
            // Replace all "defines" containing -> with the defined constant
            // TODO: replacements will go to tissot_converter.hpp
            for (std::vector<macro_or_const>::const_iterator it = m_prop.defined_parameters.begin();
                it != m_prop.defined_parameters.end();
                ++it)
            {
                boost::replace_all(line, it->name, it->value);
            }
        }


        void insert_into_setup(std::string line, std::vector<std::string>& setup)
        {
            replace_macros(line);
            if (replace_in_entry(line))
            {
                setup.push_back(line);
            }
        }

        std::string get_raw_model(std::string const& line)
        {
            std::string result = line;
            strip_comments(result);
            boost::trim(result);
            return result;
        }

        void parse(std::string const& filename)
        {
            std::ifstream cpp_file(filename.c_str());

            bool in_projection = false;
            bool in_constructor = false;
            bool in_prefix = true;
            bool in_postfix = false;
            bool in_define = false;
            bool in_comment = false;
            bool in_proj_params = false;

            static std::string PROJ_HEAD("PROJ_HEAD(");

            std::vector<projection>::iterator proj_it;
            std::vector<derived>::iterator derived_it;
            std::string line2; // keeping macro value of line2

            int proj_lineno = -1;


            if (cpp_file.is_open())
            {
                while (! cpp_file.eof() )
                {

                    std::string line;
                    std::getline(cpp_file, line);
                    lineno++;

                    // Avoid tabs
                    boost::replace_all(line, "\t", tab1);

                    // Replace degree-sign
                    std::string deg = "";
                    deg += static_cast<unsigned char>(176); // or 248, but in Krovak it is 175
                    boost::replace_all(line, deg, " DEG");

                    boost::trim_right(line);

                    std::string trimmed = boost::trim_copy(line);
                    std::string nospaced = boost::replace_all_copy(line, " ", "");

                    std::string raw_model, direction;
                    std::vector<std::string> extra_entries;

                    bool started = false;

                    std::string::size_type loc;

                    if (find(line, PROJ_HEAD, loc)
                        || (lineno == proj_lineno + 1
                            && (boost::starts_with(trimmed, "\"") || boost::starts_with(trimmed, "LINE2"))
                            )

                        )
                    {
                        if (loc == std::string::npos
                            && lineno == proj_lineno + 1 && ! m_prop.derived_projections.empty())
                        {
                            // If macro LINE2 is defined and used, we replace it
                            boost::replace_all(trimmed, "LINE2", line2);
                            m_prop.derived_projections.back().raw_characteristics += " " + trimmed;
                        }
                        else
                        {
                            loc += PROJ_HEAD.length();
                            std::string::size_type comma = line.find(',');
                            std::string::size_type end = line.find("\")", comma);
                            if (comma != std::string::npos && end != std::string::npos)
                            {
                                derived der;
                                der.name = line.substr(loc, comma - loc);
                                der.description = boost::trim_copy(line.substr(comma + 1, end - comma));
                                der.raw_characteristics = line.substr(end + 2);

                                m_prop.derived_projections.push_back(der);
                            }
                        }
                        proj_lineno = lineno;
                    }
                    else if (boost::starts_with(trimmed, "FORWARD"))
                    {
                        in_prefix = false;
                        in_projection = true;
                        started = true;
                        direction = "forward";
                        raw_model = get_raw_model(trimmed);
                    }
                    else if (boost::starts_with(trimmed, "INVERSE"))
                    {
                        in_prefix = false;
                        in_projection = true;
                        started = true;
                        direction = "inverse";
                        raw_model = get_raw_model(trimmed);
                    }
                    else if (boost::starts_with(trimmed, "SPECIAL"))
                    {
                        in_prefix = false;
                        in_projection = true;
                        started = true;
                        direction = "special_factors";  // pseudo. Is in lcc and in eqdc

                        // do not change the model
                        raw_model = proj_it->raw_model;
                    }
                    else if (boost::starts_with(trimmed, "FREEUP"))
                    {
                        in_projection = false;
                        started = true;
                    }
                    else if (boost::starts_with(trimmed, "ENTRYX"))
                    {
                        // sterea contains (per proj 4.8) ENTRYA/ENTRYX which is an if/then construct.
                        // We ignore the X, so all is initialized
                        trimmed.clear();
                        line.clear();
                    }
                    else if (boost::starts_with(trimmed, "ENTRY"))
                    {
                        in_constructor = true;
                        in_postfix = false;
                        started = true;

                        std::string entry = trimmed.substr(6);
                        std::string name = entry;

                        // Some files contain all initialization in one line. So split it here
                        if (boost::contains(entry, "ENDENTRY"))
                        {
                            // Insert extra ; to split easier
                            std::string::size_type pos = entry.find(')');
                            if (pos != std::string::npos)
                            {
                                entry.insert(pos + 1, ";");
                            }
                            //boost::split(extra_entries, entry, boost::is_any_of(";"));
                            split(entry, extra_entries, ";");

                            if (extra_entries.size() > 0)
                            {
                                name = extra_entries[0];
                                extra_entries.erase(extra_entries.begin());
                            }
                            started = false;
                            in_constructor = false;
                        }

                        boost::replace_all(name, "(", "");
                        boost::replace_all(name, ")", "");
                        boost::replace_all(name, ";", "");
                        // ENTRY1 entries have a comma plus "en". Remove that.
                        std::string::size_type loc = name.find(",");
                        if (loc != std::string::npos)
                        {
                            name.erase(loc);
                        }

                        // Find corresponding projection
                        derived_it = m_prop.derived_projections.begin();
                        while (derived_it != m_prop.derived_projections.end() && derived_it->name != name)
                        {
                            ++derived_it;
                        }
                        if (derived_it == m_prop.derived_projections.end())
                        {
                            std::cerr << "WARNING: " << "ENTRY: " << name << " not found" << std::endl;
                        }
                    }
                    else if (boost::starts_with(trimmed, "ENDENTRY"))
                    {
                        // Some files have setup(...) here
                        std::string parameters = end_entry(trimmed);
                        if (boost::contains(parameters, "setup") && derived_it != m_prop.derived_projections.end())
                        {
                            boost::replace_all(trimmed, "ENDENTRY(", "");
                            boost::replace_last(trimmed, ")", "");
                            boost::replace_first(trimmed, "setup(P", "setup(par");
                            derived_it->constructor_lines.push_back(trimmed + ";");
                        }
                        in_constructor = false;
                    }
                    else if (boost::starts_with(trimmed, "setup") && m_prop.setup_function_line.empty())
                    {
                        in_postfix = true;
                        m_prop.setup_function_line = trimmed;
                        boost::replace_all(m_prop.setup_function_line, "PJ *P", "Parameters& par");
                        boost::replace_all(m_prop.setup_function_line, "static PJ *", "void");
                        /*libproject*/boost::replace_all(m_prop.setup_function_line, "PROJ *P", "Parameters& par");
                        /*libproject*/boost::replace_all(m_prop.setup_function_line, "static PROJ *", "void");
                        boost::replace_all(m_prop.setup_function_line, "{", "");

                        // add void (in most/all cases 'static PJ *' was on the previous line)
                        if (! boost::starts_with(m_prop.setup_function_line, "void"))
                        {
                            m_prop.setup_function_line = "void " + m_prop.setup_function_line;
                        }

                    }
                    else if (boost::contains(trimmed, "PROJ_PARMS"))
                    {
                        in_proj_params = true;
                    }
                    else if (boost::starts_with(nospaced, "#define"))
                    {
                        in_proj_params = false;
                        if (boost::ends_with(trimmed, "\\"))
                        {
                            in_define = true;
                        }
                        else
                        {
                            boost::replace_first(trimmed, "#", "");
                            boost::replace_first(trimmed, "define", "");
                            boost::trim(trimmed);
                            std::string::size_type space = trimmed.find(' ');
                            if (space != std::string::npos)
                            {
                                // Split it
                                macro_or_const mc;
                                mc.name = trimmed.substr(0, space);
                                mc.value = trimmed.substr(space);
                                boost::trim(mc.name);
                                boost::trim(mc.value);
                                if (mc.name == "LINE2")
                                {
                                    // LINE2 is a macro defined for projection characteristis
                                    line2 = trimmed;
                                    boost::replace_all(line2, "LINE2", "");
                                    boost::replace_all(line2, "\"", "");
                                    boost::trim(line2);
                                }
                                else if (mc.value.empty())
                                {
                                    // Skip
                                    std::cerr << "Ignored: " << mc.name << " in " << line << std::endl;
                                }
                                else
                                {
                                    if ((boost::contains(mc.name, "(") && boost::contains(mc.name, ")"))
                                        || (boost::contains(mc.value, "{") && boost::contains(mc.value, "}")))
                                    {
                                        m_prop.defined_macros.push_back(mc);
                                    }
                                    else if (boost::contains(mc.value, "->"))
                                    {
                                        m_prop.defined_parameters.push_back(mc);
                                    }
                                    else
                                    {
                                        m_prop.defined_consts.push_back(mc);
                                    }
                                }
                            }
                        }
                    }
                    else if (in_proj_params)
                    {
                        std::string par = trimmed;
                        boost::trim_right_if(par, boost::is_any_of("\\ "));
                        boost::replace_all(par, "\t", " ");

                        std::string par_constructor;

                        m_prop.proj_parameters.push_back(par);

                        if (! boost::contains(trimmed, "\\"))
                        {
                            in_proj_params = false;
                        }
                        proj_parameter_names = extract_names(m_prop.proj_parameters);

                        if (! par_constructor.empty())
                        {
                            m_prop.proj_parameters.push_back("");
                            m_prop.proj_parameters.push_back(par_constructor);
                        }


                    }
                    else if (in_prefix
                        && ! boost::contains(trimmed, "*/")
                        && (boost::starts_with(trimmed, "/***")
                            || boost::starts_with(trimmed, "/* PROJ.4")
                            || trimmed == "/*"
                            )
                        )
                    {
                        in_comment = true;
                    }
                    else if (in_comment && boost::contains(trimmed, "*/"))
                    {
                        in_comment = false;
                    }
                    else if (in_prefix
                        && ! in_comment
                        && ! in_define
                        && ! boost::starts_with(nospaced, "#include")
                        && ! boost::contains(line, "SCCSID[]")
                        && ! boost::contains(line, "PJ_CVSID")
                        && ! boost::contains(line, "RCS_ID[]")
                        && ! boost::contains(line, "LIBPROJ_ID")
                        && trimmed != std::string("static const char") // libproj_id often splitted on two lines
                        && ! (boost::starts_with(trimmed, "\"") && boost::ends_with(trimmed, "\";")) // after projhead
                        && ! (boost::starts_with(trimmed, "\"") && boost::ends_with(trimmed, "\""))) // after projhead
                    {
                        m_prop.inlined_functions.push_back(line);
                    }
                    else if (in_postfix)
                    {
                        insert_into_setup(line, m_prop.setup_functions);
                    }
                    else
                    {
                        in_proj_params = false;
                    }

                    if (! boost::ends_with(trimmed, "\\"))
                    {
                        in_define = false;
                    }

                    // Process projection functions
                    if (in_projection)
                    {
                        if (started)
                        {
                            projection proj;
                            proj.raw_model = raw_model;
                            proj.direction = direction;
                            m_prop.projections.push_back(proj);

                            proj_it = m_prop.projections.begin() + (m_prop.projections.size() - 1);

                        }
                        else if (proj_it != m_prop.projections.end())
                        {
                            // Goes in "projection.lines"

                            replace_macros(line);
                            replace_pars(line, "this->m_");

                            replace_return(line, "xy");
                            replace_return(line, "lp");

                            boost::replace_all(line, "xy.x", "xy_x");
                            boost::replace_all(line, "xy.y", "xy_y");
                            boost::replace_all(line, "lp.lam", "lp_lon");
                            boost::replace_all(line, "lp.phi", "lp_lat");

                            // TODO: create something like: replace whole words, case sensitive:
                            boost::replace_all(line, " cosl,", " cosl_,");
                            boost::replace_all(line, " cosl)", " cosl_)");
                            boost::replace_all(line, " cosl ", " cosl_ ");

                            proj_it->lines.push_back(line);
                        }
                    }
                    else if (in_constructor)
                    {
                        if (! started && derived_it != m_prop.derived_projections.end())
                        {
                            // Replace call to "setup(P)" with appropriate
                            if (boost::starts_with(trimmed, "setup(P"))
                            {
                                    boost::replace_first(line, "(P", "(par");
                            }
                            insert_into_setup(line, derived_it->constructor_lines);
                        }
                    }
                    else if (extra_entries.size() > 0 && derived_it != m_prop.derived_projections.end())
                    {
                        for (std::vector<std::string>::iterator it = extra_entries.begin(); it != extra_entries.end(); it++)
                        {
                            if (boost::contains(*it, "ENDENTRY"))
                            {
                                std::string par = end_entry(*it);
                                if (! par.empty())
                                {
                                    derived_it->constructor_lines.push_back(par + ";");
                                }
                            }
                            else
                            {
                                replace_in_entry(*it);
                                derived_it->constructor_lines.push_back(tab1 + boost::trim_copy(*it + ";"));
                            }
                        }
                    }

                }
                cpp_file.close();
            }
        }

        // Class - member variables
        projection_properties& m_prop;
        std::vector<epsg_entry> const& m_epsg_entries;

        std::string projection_group;

        std::vector<std::string> proj_parameter_names; // same but only names

        bool skip_for_setup;
        unsigned int skip_lineno;
        unsigned int lineno;
};


}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_PARSER_HPP

