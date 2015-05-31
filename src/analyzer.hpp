#ifndef TISSOT_ANALYZER_HPP
#define TISSOT_ANALYZER_HPP

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

#include <boost/foreach.hpp>


namespace boost { namespace geometry { namespace proj4converter
{


class proj4_analyzer
{
public :
    proj4_analyzer(projection_properties& prop, std::string const& group)
    : m_prop(prop)
    , m_group(group)
    {
    }

    void analyze()
    {
        BOOST_FOREACH(projection& proj, m_prop.projections)
        {
            proj.subgroup = m_group;
            assign_model(proj.direction, proj);
        }

        // Sort on: subgroup, model, direction
        std::sort(m_prop.projections.begin(), m_prop.projections.end());

        std::size_t n = m_prop.projections.size();
        for (size_t i = 0; i < n; i++)
        {
            projection& proj = m_prop.projections[i];
            if (proj.direction == "inverse" ||
                (i < n - 1 && m_prop.projections[i + 1].direction == "inverse"))
            {
                proj.has_inverse = true;
            }
            if (proj.model == "ellipsoid")
            {
                m_prop.has_ellipsoid = true;
            }
            else if (proj.model == "spheroid")
            {
                m_prop.has_spheroid = true;
            }
            else if (proj.model == "guam")
            {
                m_prop.has_guam = true;
            }
        }

        std::vector<model> models;
        std::string previous_model;
        BOOST_FOREACH(projection& proj, m_prop.projections)
        {
            if (proj.model != previous_model)
            {
                model m;
                m.name = proj.model;
                m.subgroup = proj.subgroup;
                m.has_inverse = proj.direction == "inverse";
                if (m_prop.has_spheroid && m_prop.has_ellipsoid)
                {
                    m.condition = m.name == "ellipsoid" ? "if (par.es)" : "else";
                }
                models.push_back(m);
            }
            else if (proj.direction == "inverse")
            {
                models.back().has_inverse = true;
            }
            previous_model = proj.model;
        }

        // Copy the models separately to all derived projections - some of them
        // might be deleted later if not supported
        BOOST_FOREACH(derived& der, m_prop.derived_projections)
        {
            der.models = models;
        }
    }


private :

    bool check_model(std::string term, std::string const& prefix,
            const std::string& direction, std::string const& model,
            projection& proj)
    {
        // Default case, e.g. e_forward, s_forward, etc
        if (term == prefix + "_" + direction)
        {
            proj.model = model;
            return true;
        }

        // e.g. for healpix, e_healpix_forward, etc
        if (term == prefix + "_" +  m_group + "_" + direction)
        {
            proj.model = model;
            proj.subgroup = m_group;
            return true;
        }

        // e.g. for healpix, e_healpix_forward, etc
        if (term == prefix + "_r" + m_group + "_" + direction)
        {
            proj.model = model;
            proj.subgroup = "r" + m_group;
            return true;
        }

        // For e.g. gaum ("e_gaum_fwd")
        if (boost::ends_with(term, "inv"))
        {
            boost::replace_last(term, "inv", "inverse");
        }
        if (boost::ends_with(term, "fwd"))
        {
           boost::replace_last(term, "fwd", "forward");
        }

        if (term == prefix + "_" + model + "_" + direction) // e.g. e_gaum_forward
        {
            proj.model = model;
            return true;
        }

        return false;
    }

    bool assign_model(std::string direction, projection& proj)
    {
        std::string term = proj.raw_model;
        boost::replace_all(term, boost::to_upper_copy(direction), "");
        if (direction == "special_factors")
        {
            boost::replace_all(term, "INVERSE", "");
            direction = "inverse";
        }
        boost::replace_all(term, "(", "");
        boost::replace_all(term, ")", "");
        boost::replace_all(term, ";", "");

        boost::trim(term);

        if (check_model(term, "e", direction, "ellipsoid", proj)
            || check_model(term, "s", direction, "spheroid", proj)
            || check_model(term, "o", direction, "oblique", proj)
            || check_model(term, "t", direction, "transverse", proj)
            || check_model(term, "e", direction, "guam", proj))
        {
            return true;
        }
        else if (term == direction)
        {
            proj.model = "other";
            return true;
        }

        std::cerr << "ERROR IN MODEL " << term << std::endl;
        proj.model = "ERROR";
        return false;
    }

    projection_properties& m_prop;
    std::string m_group;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_ANALYZER_HPP
