#ifndef TISSOT_CONVERTER_HPP
#define TISSOT_CONVERTER_HPP

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

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <sstream>

namespace boost { namespace geometry { namespace proj4converter
{


class proj4_converter_cpp_bg
{
public :
    proj4_converter_cpp_bg(projection_properties& prop)
    : m_prop(prop)
    {
    }

    void convert()
    {
        for_each_line(&proj4_converter_cpp_bg::replace_macros);
        for_each_line(&proj4_converter_cpp_bg::remove_fwd_inv);
        replace_setup();
        replace_parameters();
        replace_parameter_usage();
        replace_return();

        fix_lastlines();
        replace_apple_macros();
        replace_ctx();
        replace_struct_parameters();
        determine_const_types();

        for_each_line(&proj4_converter_cpp_bg::replace_exceptions);
        trim(); // we do this twice
    }

    // Called after converting specific projections
    void post_convert()
    {
        replace_all_functions();
    }

    void trim()
    {
        for_each_line(&proj4_converter_cpp_bg::remove_empty_preceding_lines);
        for_each_line(&proj4_converter_cpp_bg::remove_empty_trailing_lines);
        for_each_line(&proj4_converter_cpp_bg::remove_redundant_returns);
        for_each_line(&proj4_converter_cpp_bg::trim_right);
    }

    void scan()
    {
        check_unused_parameters();
        for_each_line(&proj4_converter_cpp_bg::scan_includes);
    }

private :

    void for_each_line(void (proj4_converter_cpp_bg::*ptr)(std::vector<std::string>& lines))
    {
        (this->*ptr)(m_prop.proj_parameters);
        (this->*ptr)(m_prop.inlined_functions);
        (this->*ptr)(m_prop.setup_functions);
        BOOST_FOREACH(derived& der, m_prop.derived_projections)
        {
            (this->*ptr)(der.constructor_lines);
        }
        BOOST_FOREACH(projection& proj, m_prop.projections)
        {
            (this->*ptr)(proj.lines);
        }
    }

    void replace_all_functions()
    {
        for_each_line(&proj4_converter_cpp_bg::replace_functions);
        BOOST_FOREACH(macro_or_const& mc, m_prop.defined_macros)
        {
            replace_functions_in_line(mc.value);
        }
        BOOST_FOREACH(macro_or_const& mc, m_prop.defined_consts)
        {
            replace_functions_in_line(mc.value);
        }
    }

    void check_unused_parameters()
    {
        std::vector<std::string>& lines = m_prop.setup_functions;
        if (lines.empty())
        {
            return;
        }

        bool par_used = false, proj_parm_used = false;
        BOOST_FOREACH(std::string& line, lines)
        {
            std::string trimmed = boost::trim_copy(line);
            if (! boost::starts_with(trimmed, "//"))
            {
                if (boost::contains(line, "par."))
                {
                    par_used = true;
                }
                if (boost::contains(line, "proj_parm."))
                {
                    proj_parm_used = true;
                }
            }
        }
        if (! proj_parm_used && ! m_prop.proj_parameters.empty())
        {
            lines.insert(lines.begin(), tab1 + "boost::ignore_unused(proj_parm);");
        }
        if (! par_used)
        {
            lines.insert(lines.begin(), tab1 + "boost::ignore_unused(par);");
        }
    }

    void scan_includes(std::vector<std::string>& lines)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            if (boost::contains(line, "aasin")
                || boost::contains(line, "aacos"))
            {
                m_prop.extra_impl_includes.insert("aasincos.hpp");
            }
            if (boost::contains(line, "enfn"))
            {
                m_prop.extra_impl_includes.insert("pj_mlfn.hpp");
            }
            if (boost::contains(line, "pj_auth"))
            {
                m_prop.extra_impl_includes.insert("pj_auth.hpp");
            }
            if (boost::contains(line, "mdist"))
            {
                m_prop.extra_impl_includes.insert("proj_mdist.hpp");
            }
            if (boost::contains(line, "pj_phi2"))
            {
                m_prop.extra_impl_includes.insert("pj_phi2.hpp");
            }
            if (boost::contains(line, "pj_msfn"))
            {
                m_prop.extra_impl_includes.insert("pj_msfn.hpp");
            }
            if (boost::contains(line, "pj_qsfn"))
            {
                m_prop.extra_impl_includes.insert("pj_qsfn.hpp");
            }
            if (boost::contains(line, "pj_tsfn"))
            {
                m_prop.extra_impl_includes.insert("pj_tsfn.hpp");
            }
            if (boost::contains(line, "pj_zpoly"))
            {
                m_prop.extra_impl_includes.insert("pj_zpoly1.hpp");
            }

            // Boost
            if (boost::contains(line, "hypot"))
            {
                m_prop.extra_includes.insert("boost/math/special_functions/hypot.hpp");
            }
            if (boost::contains(line, "ignore_unused"))
            {
                m_prop.extra_includes.insert("boost/core/ignore_unused.hpp");
            }
            if (boost::contains(line, "shared_ptr"))
            {
                m_prop.extra_includes.insert("boost/shared_ptr.hpp");
            }

            // Std
            if (boost::contains(line, "sprintf"))
            {
                m_prop.extra_includes.insert("cstdio");
            }
        }
    }

    void remove_empty_preceding_lines(std::vector<std::string>& lines)
    {
        while(! lines.empty() && boost::trim_copy(lines.front()).empty())
        {
            lines.erase(lines.begin());
        }
    }

    void remove_empty_trailing_lines(std::vector<std::string>& lines)
    {
        while(! lines.empty() && boost::trim_copy(lines.back()).empty())
        {
            lines.erase(lines.begin() + lines.size());
        }
    }

    void remove_fwd_inv(std::string& line, std::string const& direction)
    {
        std::string::size_type const pos = line.find("P->" + direction);
        if (pos == std::string::npos)
        {
            return;
        }
        std::string::size_type const pos2 = line.find(";", pos);
        if (pos2 != std::string::npos)
        {
            line.erase(pos, pos2 + 1);
        }
    }

    void remove_fwd_inv(std::vector<std::string>& lines)
    {
        // Remove the par.fwd; and par.inv statements

        std::vector<int> to_erase;
        int index = 0;
        BOOST_FOREACH(std::string& line, lines)
        {
            if (boost::contains(line, "P->"))
            {
                remove_fwd_inv(line, "fwd");
                remove_fwd_inv(line, "inv");
                remove_fwd_inv(line, "spc");
                boost::trim_right(line);
                if (boost::trim_copy(line).empty())
                {
                    // remove it
                    to_erase.push_back(index);
                }
            }
            index++;
        }

        // Remove stored empty lines, backwards
        for (std::vector<int>::const_reverse_iterator it =
            to_erase.rbegin(); it != to_erase.rend(); ++it)
        {
            int const index = *it;
            lines.erase(lines.begin() + index);
        }
    }

    void replace_exceptions(std::vector<std::string>& lines)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            boost::replace_all(line, "F_ERROR", "throw proj_exception();");
            boost::replace_all(line, "I_ERROR", "throw proj_exception();");
            boost::replace_all(line, "E_ERROR_0", "throw proj_exception(0)");
            boost::replace_all(line, "exit(EXIT_FAILURE)", "throw proj_exception()");

            if (boost::contains(line, "E_ERROR"))
            {
                boost::replace_all(line, "E_ERROR", "throw proj_exception");
                if (boost::ends_with(line, ")"))
                {
                    boost::replace_last(line, ")", ");");
                }
            }
            if (boost::contains(line, "pj_ctx_set_errno"))
            {
                boost::replace_all(line, "this->m_par.ctx,", "");
                boost::replace_all(line, "pj_ctx_set_errno", "throw proj_exception");
            }
        }
    }

    void replace_parameter_usage(std::vector<std::string>& lines, std::string const& prefix)
    {
        std::vector<std::string> proj_parameter_names = extract_names(m_prop.proj_parameters);
        BOOST_FOREACH(std::string& line, lines)
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
    }

    void replace_parameter_usage()
    {
        replace_parameter_usage(m_prop.setup_functions, "");
        BOOST_FOREACH(derived& der, m_prop.derived_projections)
        {
            replace_parameter_usage(der.constructor_lines, "");
        }

        replace_parameter_usage(m_prop.inlined_functions, "this->m_");
        BOOST_FOREACH(projection& proj, m_prop.projections)
        {
            replace_parameter_usage(proj.lines, "this->m_");
        }
    }

    inline void replace_return(std::string& line, std::string const& var)
    {
        std::string copy(line);
        boost::replace_all(copy, "return", "");
        boost::trim_left_if(copy, boost::is_any_of(" ("));
        boost::trim_right_if(copy, boost::is_any_of(" );"));
        if (copy == var)
        {
            line = tab3 + "return;";
        }
    }

    void replace_return(std::vector<std::string>& lines, std::string const& var)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            replace_return(line, var);
        }
    }

    void replace_return()
    {
        replace_return(m_prop.setup_functions, "P");
        BOOST_FOREACH(derived& der, m_prop.derived_projections)
        {
            replace_return(der.constructor_lines, "P");
        }

        BOOST_FOREACH(projection& proj, m_prop.projections)
        {
            replace_return(proj.lines, "xy");
            replace_return(proj.lines, "lp");
        }
    }

    void replace_setup()
    {
        boost::replace_all(m_prop.setup_function_line, "PJ *P", "Parameters& par");
        boost::replace_all(m_prop.setup_function_line, "static PJ *", "void");
        /*libproject*/boost::replace_all(m_prop.setup_function_line, "PROJ *P", "Parameters& par");
        /*libproject*/boost::replace_all(m_prop.setup_function_line, "static PROJ *", "void");

        // add void (in most/all cases 'static PJ *' was on the previous line)
        if (! boost::starts_with(m_prop.setup_function_line, "void"))
        {
            m_prop.setup_function_line = "void " + m_prop.setup_function_line;
        }
    }

    void replace_macros(std::vector<std::string>& lines)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            // Replace all "defines" containing -> with the defined constant
            for (std::vector<macro_or_const>::const_iterator it = m_prop.defined_parameters.begin();
            it != m_prop.defined_parameters.end();
            ++it)
            {
                boost::replace_all(line, it->name, it->value);
            }
        }
    }

    void trim_right(std::vector<std::string>& lines)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            boost::trim_right(line);
        }
    }

    void replace_functions_in_line(std::string& line)
    {
        boost::replace_all(line, "M_PI_2", "(2.0 * boost::math::constants::pi<double>())");
        boost::replace_all(line, "M_PI", "boost::math::constants::pi<double>()");

        boost::replace_all(line, "hypot", "boost::math::hypot");
        // BEGIN libproject:
        boost::replace_all(line, "proj_asin", "std::asin");
        boost::replace_all(line, "proj_acos", "std::acos");
        // END libproject
    }

    void replace_functions(std::vector<std::string>& lines)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            replace_functions_in_line(line);
        }
    }

    void replace_parameters()
    {
        std::vector<std::string>& lines = m_prop.inlined_functions;

        for (std::vector<std::string>::iterator it = lines.begin();
            it != lines.end(); ++it)
        {
            if (boost::contains(*it, "PJ *P"))
            {
                boost::replace_all(*it, "PJ *P", "Parameters& par");
                std::vector<std::string>::iterator start = it;
                std::string trimmed = boost::trim_copy(*(start - 1));
                if (boost::starts_with(trimmed, "inline ")
                    || boost::starts_with(trimmed, "static "))
                {
                    --start;
                }
                lines.insert(start, "template <typename Parameters>");
                ++it;
                if (it == lines.end())
                {
                    break;
                }
            }
        }
        BOOST_FOREACH(std::string& line, lines)
        {
            boost::replace_all(line, "P->", "par.");
        }
        BOOST_FOREACH(derived& der, m_prop.derived_projections)
        {
            BOOST_FOREACH(std::string& line, der.constructor_lines)
            {
                boost::replace_first(line, "setup(P", "setup(par");
            }
        }
    }

    void fix_lastlines()
    {
        // Implementations contain a last line including "}", redundant returns, etc

        BOOST_FOREACH(projection& proj, m_prop.projections)
        {
            // Erase single last closing curly brace (might be located before any comments)
            bool found = false;
            std::size_t index = proj.lines.size() - 1;
            for (std::vector<std::string>::reverse_iterator it = proj.lines.rbegin();
            it != proj.lines.rend(); ++it, index--)
            {
                if (boost::trim_copy(*it) == "}")
                {
                    proj.lines.erase(proj.lines.begin() + index);
                    found = true;
                    break;
                }
            }
            if (! found)
            {
                std::cerr << "Warning: no closing brace found" << std::endl;
            }

            remove_empty_trailing_lines(proj.lines);

            // Move last comments (if any)
            while(! proj.lines.empty()
            && (boost::starts_with(boost::trim_copy(proj.lines.back()), "/*")
            || boost::ends_with(boost::trim_copy(proj.lines.back()), "*/")
            || boost::starts_with(boost::trim_copy(proj.lines.back()), "*"))) // fragile!
            {
                proj.trailing_lines.push_front(proj.lines.back());
                proj.lines.erase(proj.lines.begin() + proj.lines.size());
            }
        }
    }

    void remove_redundant_returns(std::vector<std::string>& lines)
    {
        // Erase last return statement (if any)
        if(! lines.empty() && boost::trim_copy(lines.back()) == "return;")
        {
            lines.erase(lines.begin() + lines.size());
            return;
        }
        // Erase return statement before last curly brace
        std::size_t const n = lines.size();
        if (n > 2
            &&  boost::trim_copy(lines[n - 2]) == "return;"
            &&  boost::trim_copy(lines[n - 1]) == "}")
        {
            lines.erase(lines.begin() + n - 2);
        }
    }

    void remove_pair_of_brackets(std::string& line)
    {
        while (boost::contains(line, "(!(") && boost::contains(line, ")))"))
        {
            // Remove one pair of brackets
            boost::replace_all(line, "(!(", "(!");
            boost::replace_all(line, ")))", "))");
        }
    }

    void pass_parameter_instead_of_return(std::vector<std::string>& lines)
    {
        BOOST_FOREACH(std::string& line, lines)
        {
            if (boost::contains(line, "pj_enfn")
                || boost::contains(line, "proj_mdist_ini"))
            {
                // Pass "en" as parameter instead
                boost::replace_all(line, "proj_parm.en = ", "");
                boost::replace_all(line, "par.es", "par.es, proj_parm.en");
                remove_pair_of_brackets(line);
            }
            if (boost::contains(line, "pj_authset"))
            {
                // Pass "apa" as parameter instead
                boost::replace_all(line, "proj_parm.apa = ", "");
                boost::replace_all(line, "par.es", "par.es, proj_parm.apa");
                remove_pair_of_brackets(line);
            }
        }
    }

    void replace_struct_parameters()
    {
        // For all projections using EN/APA (enfn, auth, mdist)
        BOOST_FOREACH(std::string& line, m_prop.proj_parameters)
        {
            if (boost::contains(line, "double") && boost::contains(line, "*en"))
            {
                boost::replace_all(line, "*en", "en[EN_SIZE]");
            }
            if (boost::contains(line, "double") && boost::contains(line, "*apa"))
            {
                boost::replace_all(line, "*apa", "apa[APA_SIZE]");
            }
        }
        for_each_line(&proj4_converter_cpp_bg::pass_parameter_instead_of_return);
    }

    void determine_const_types()
    {
        BOOST_FOREACH(macro_or_const& con, m_prop.defined_consts)
        {
            // By default:
            con.type = "double";

            // Check if integer:
            {
                int value = atoi(con.value.c_str());
                std::ostringstream out;
                out << value;
                if (out.str() == con.value)
                {
                    con.type = "int";
                }
            }
        }
    }


    void replace_ctx(std::string const& function, std::string& line)
    {
        // remove space after
        boost::replace_all(line, "ctx, ", "ctx,");

        // remove space after opening bracket
        boost::replace_all(line, function + "( ", function + "(");

        std::string const replacement = function + "(";
        boost::replace_all(line, function + "(P->ctx,", replacement);
        boost::replace_all(line, function + "(par.ctx,", replacement);
        boost::replace_all(line, function + "(projCtx ctx,", replacement);
        boost::replace_all(line, function + "(proj_parm.ctx,", replacement);
        boost::replace_all(line, function + "(this->proj_parm.ctx,", replacement);
        boost::replace_all(line, function + "(this->m_proj_parm.ctx,", replacement);
        boost::replace_all(line, function + "(this->m_par.ctx,", replacement);
        boost::replace_all(line, function + "(ctx,", replacement);
    }

    void replace_ctx(std::string& line)
    {
        if (boost::contains(line, "ctx"))
        {
            replace_ctx("aasin", line);
            replace_ctx("aacos", line);
            replace_ctx("vect", line);
            replace_ctx("lc", line);
            replace_ctx("pj_inv_mlfn", line);
            replace_ctx("pj_param", line);
            replace_ctx("pj_phi2", line);
            replace_ctx("proj_inv_mdist", line);
            replace_ctx("proj_exception", line);
        }
    }

    void replace_ctx(std::vector<std::string>& lines)
    {
        // NEW since 4.8 or later, removes ctx from aasin/aacos because we throw an exception there
        BOOST_FOREACH(std::string& line, lines)
        {
            replace_ctx(line);
        }
    }

    void replace_ctx()
    {
        BOOST_FOREACH(derived& der, m_prop.derived_projections)
        {
            functor_nospaces_equals functor("pj.ctx=par.ctx;", "proj_parm.link->ctx=par.ctx;");
            std::vector<std::string>& lines = der.constructor_lines;
            lines.erase
                (
                    std::remove_if(lines.begin(), lines.end(), functor),
                    lines.end()
                );
            BOOST_FOREACH(std::string& line, lines)
            {
                replace_ctx(line);
            }
        }
        for_each_line(&proj4_converter_cpp_bg::replace_ctx);
    }

    void replace_apple_macros()
    {
        // CS/CN are internally used by Apple/XCode. We add an underscore

        for (std::vector<macro_or_const>::iterator it = m_prop.defined_consts.begin();
        it != m_prop.defined_consts.end(); ++it)
        {
            if (it->name == "CS") { it->name = "CS_"; }
            if (it->name == "CN") { it->name = "CN_"; }
        }

        BOOST_FOREACH(projection& proj, m_prop.projections)
        {
            BOOST_FOREACH(std::string& line, proj.lines)
            {
                std::vector<std::string> splitted;
                boost::split(splitted, line, boost::is_any_of(" ,:+-?;"), boost::token_compress_on);

                boost::replace_all(line, " CS ", " CS_ ");
                boost::replace_all(line, " CN ", " CN_ ");
                boost::replace_all(line, " CS)", " CS_)");
                boost::replace_all(line, " CN)", " CN_)");
                boost::replace_all(line, "(CS ", "(CS_ ");
                boost::replace_all(line, "(CN ", "(CN_ ");
            }
        }
    }

    projection_properties& m_prop;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_HPP
