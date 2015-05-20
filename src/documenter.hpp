#ifndef TISSOT_DOCUMENTER_HPP
#define TISSOT_DOCUMENTER_HPP

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

#include <string>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

namespace boost { namespace geometry { namespace proj4converter
{

struct find_parameter
{
    find_parameter(std::string const& name)
        : m_name(name)
    {}

    bool operator()(parameter const& p)
    {
        return p.name == m_name;
    }

    std::string m_name;
};

class proj4_documenter
{
public :
    proj4_documenter(projection_properties& prop, std::string const& group)
    : m_prop(prop)
    , m_group(group)
    {
    }

    void analyze()
    {
        BOOST_FOREACH(derived& der, m_prop.derived_projections)
        {
            replace_double_quotes(der);
            parse_characteristics(der);
            scan_parameters(der);
        }
    }

    void create()
    {
        BOOST_FOREACH(derived& der, m_prop.derived_projections)
        {
            document_characteristics(der);
        }
    }


private :

    void replace_double_quotes(derived& der)
    {
        // Replace enclosing double quotes
        std::string::size_type begin = der.description.find('"');
        std::string::size_type end = der.description.rfind('"');
        if (begin != std::string::npos
            && end != std::string::npos
            && begin != end)
        {
            der.description.erase(end);
            der.description.erase(begin, 1);
        }
    }

    void parse_characteristics(derived& der)
    {
        std::string line = der.raw_characteristics;
        boost::replace_all(line, "\\t", " ");
        boost::replace_all(line, "\\n", ",");
        boost::replace_all(line, "\"", "");
//        boost::replace_all(line, ".", " ");
        boost::replace_all(line, "  ", " ");

        // Characteristics with spaces:
        boost::replace_all(line, "plus parameters for projection", "PLUS_PROJECTION_PARAMETERS");
        boost::replace_all(line, "no inverse", "NO_INVERSE");
        boost::replace_all(line, "no inv.", "NO_INVERSE");
        boost::replace_all(line, "new pole", "NEW_POLE");
        boost::replace_all(line, "Azi(mod)", "AZI_MOD");
        boost::replace_all(line, "Mod. Polyconic", "POLYCONIC_MOD");
        boost::replace_all(line, "Special for Madagascar", "MADAGASCAR");
        boost::replace_all(line, "For CH1903", "CH1903");
        boost::replace_all(line, "fixed Earth", "FIXED_EARTH");

        // Remove brackets
        boost::replace_all(line, "[", "");
        boost::replace_all(line, "]", "");
        boost::replace_all(line, "(", "");
        boost::replace_all(line, ")", "");

        // Replace combinations
        boost::replace_all(line, "&", " ");
        boost::replace_all(line, " and", " ");
        boost::replace_all(line, " or", " ");

        // Remove defaults (TODO: get them back somehow)
        boost::replace_all(line, "=0", "");

        std::vector<std::string> terms;
        split(line, terms, " =,;\n");
        BOOST_FOREACH(std::string& term, terms)
        {
            boost::trim(term);
            if (boost::ends_with(term, "."))
            {
                boost::replace_last(term, ".", "");
            }

            if (term == "NO_INVERSE")
            {
                der.parsed_characteristics.push_back("no inverse");
            }
            else if (term == "MADAGASCAR")
            {
                der.parsed_characteristics.push_back("Special for Madagascar");
            }
            else if (term == "CH1903")
            {
                der.parsed_characteristics.push_back("For CH1903");
            }
            else if (term == "FIXED_EARTH")
            {
                der.parsed_characteristics.push_back("Fixed Earth");
            }
            else if (term == "Azi" || term == "Azimuthal")
            {
                der.parsed_characteristics.push_back("Azimuthal");
            }
            else if (term == "AZI_MOD")
            {
                der.parsed_characteristics.push_back("Azimuthal (mod)");
            }
            else if (term == "PCyl" | term == "Pcyl")
            {
                der.parsed_characteristics.push_back("Pseudocylindrical");
            }
            else if (term == "Misc")
            {
                der.parsed_characteristics.push_back("Miscellaneous");
            }
            else if (term == "Cyl")
            {
                der.parsed_characteristics.push_back("Cylindrical");
            }
            else if (term == "Conic"
                    || term == "Polyconic")
            {
                der.parsed_characteristics.push_back(term);
            }
            else if (term == "POLYCONIC_MOD")
            {
                der.parsed_characteristics.push_back("Mod. Polyconic");
            }
            else if (term == "Ell" || term == "Ellps" || term == "Ellipsoid")
            {
                der.parsed_characteristics.push_back("Ellipsoid");
            }
            else if (term == "Sph" || term == "Spheroid")
            {
                der.parsed_characteristics.push_back("Spheroid");
            }
            else
            {
                parameter par;
                par.name = term;
                der.parsed_parameters.push_back(par);
            }
        }
    }

    void scan_parameters(derived& der)
    {
        BOOST_FOREACH(std::string const& line, der.constructor_lines)
        {
            std::string copy = line;
            while (scan_parameter(der, copy))
            {}
        }
    }

    bool scan_parameter(derived& der, std::string& line)
    {
        std::string const param("pj_param");
        std::string::size_type pos = line.find(param);
        if (pos == std::string::npos)
        {
            return false;
        }
        line.erase(0, pos + param.length());
        // Now leave line like this to possibly find another pj_param occurence

        std::string term = line;

        // Find opening/closing double quote
        pos = term.find('"');
        if (pos == std::string::npos)
        {
            return false;
        }
        term.erase(0, pos + 1);

        // They are all typed by type in the first character, remove that one too
        char ch = term[0];
        std::string type
            = ch == 'd' ? "real" // simple real input
            : ch == 'r' ? "degrees"
            : ch == 's' ? "string"
            : ch == 'b' ? "boolean"
            : ch == 'i' ? "integer"
            : ch == 't' ? ""  // test for presence
            : "?";

        term.erase(0, 1);

        pos = term.find('"');
        if (pos == std::string::npos)
        {
            return false;
        }
        term.erase(pos);

        // Find the parameter
        std::vector<parameter>::iterator it
                = std::find_if(der.parsed_parameters.begin(), der.parsed_parameters.end(), find_parameter(term));
        if (it == der.parsed_parameters.end())
        {
            parameter par;
            par.used = true;
            par.type = type;
            par.name = term;
            der.parsed_parameters.push_back(par);
        }
        else
        {
            // Already described, register usage and type
            parameter& par = *it;
            par.used = true;
            if (! boost::contains(par.type, type))
            {
                par.type += type;
            }
        }
        return true;
    }

    void document_characteristics(derived& der)
    {
        typedef std::map<std::string, std::string> map_type;
        map_type properties
            = boost::assign::map_list_of
                ("lat_0", "Latitude of origin")
                ("lat_1", "Latitude of first standard parallel")
                ("lat_2", "Latitude of second standard parallel")
                ("lat_ts", "Latitude of true scale")
                ("lon_0", "Central meridian")
                ("lon_1", "") // TODO
                ("lon_2", "")
                ("south", "Denotes southern hemisphere UTM zone")
                ("zone", "UTM Zone")

                ("alpha", "Alpha") // ob_tran, ocea, omerc, urm5
                ("gamma", "Gamma") // omerc
                ("lonc", "Longitude (only used if alpha (or gamma) is specified)")

                ("theta", "Theta") // oea
                ("azi", "Azimuth (or Gamma)") // isea, labrd, nsper
                ("tilt", "Tilt, or Omega") // nsper
                ("no_cut", "Do not cut at hemisphere limit") // airy
                ("no_off", "Only for compatibility with libproj, proj4") // omerc
                ("no_rot", "No rotation") // labrd, omerc
                ("sweep", "Sweep axis ('x' or 'y')") // geos
                ("h", "Height") // geos, nsper - probably in meters
                ("k", "Scale factor on the pseudo standard parallel") // Krovak
                ("k_0", "Scale factor") // (new name), gstmerc, not even used
                ("m", "") // gn_sinu, oea
                ("n", "") // fouc_s, gn_sinu, oea, urm5, urmfps
                ("q", "") // urm5
                ("M", "") // hammer - unknown
                ("W", "") // hammer, lagrng - unknown
        ;
        BOOST_FOREACH(parameter& p, der.parsed_parameters)
        {
            // Exceptions
            if (p.processed)
            {
                continue;
            }
            else if (p.name == "PLUS_PROJECTION_PARAMETERS")
            {
                p.name = "Plus projection parameters";
                continue;
            }
            else if (p.name == "NEW_POLE")
            {
                p.name = "New pole";
                continue;
            }

            map_type::const_iterator it = properties.find(p.name);
            if (it == properties.end()
                && boost::starts_with(p.name, "o_"))
            {
                it = properties.find(p.name.substr(2));
            }
            if (it == properties.end())
            {
                std::cerr << "Not found: projection: " << m_group << " Parameter: " << p.name << std::endl;
            }
            else
            {
                p.explanation = it->second;
            }
        }

    }

    projection_properties& m_prop;
    std::string m_group;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_DOCUMENTER_HPP
