#ifndef TISSOT_STRUCTS_HPP
#define TISSOT_STRUCTS_HPP

// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <deque>
#include <set>
#include <string>
#include <vector>

namespace boost { namespace geometry { namespace proj4converter
{


const std::string include_projections = "#include <boost/geometry/extensions/gis/projections";


struct epsg_entry
{
    int epsg_code;
    std::string prj_name;
    std::string parameters;
    epsg_entry(int code, std::string const& name, std::string const& pars)
        : epsg_code(code)
        , prj_name(name)
        , parameters(pars)
    {}
};

struct projection
{
    bool has_inverse;
    std::string raw_model; // as in proj4: e_forward, etc
    std::string direction; // inverse/forward/factors
    std::string model; // spheroid/ellipsoid/oblique/transverse/Guam-elliptical
    std::string subgroup; // TODO: healpix/rhealpix
    std::vector<std::string> lines;
    std::vector<std::string> preceding_lines;
    std::deque<std::string> trailing_lines;

    // Sort on subgroup, then on model, then on direction
    inline bool operator<(projection const& other) const
    {
        return subgroup != other.subgroup ? subgroup < other.subgroup
            : model != other.model ? model < other.model
            : direction < other.direction
            ;
    }
    projection()
        : has_inverse(false)
    {}
};

struct macro_or_const
{
    std::string type;
    std::string name;
    std::string value;
};

struct model
{
    std::string name;
    std::string condition;
    bool has_inverse;
    model()
        : has_inverse(false)
    {}
};

struct parameter
{
    bool used;
    bool processed;
    std::string name;
    std::string explanation;
    std::string type;

    parameter()
        : used(false)
        , processed(false)
    {}
};

struct derived
{
    std::string name;
    std::string description;
    std::string raw_characteristics;
    std::vector<std::string> parsed_characteristics;
    std::vector<parameter> parsed_parameters;
    std::vector<std::string> constructor_lines;
    std::vector<model> models; // filled during analyze
};


struct projection_properties
{
    bool valid;
    bool parstruct_first;
    bool has_ellipsoid;
    bool has_spheroid;
    bool has_guam;
    std::string forward_declarations;
    std::string template_struct;
    std::string setup_return_type; // If empty, then "void" assumed
    std::string setup_extra_parameters;
    std::vector<std::string> setup_extra_code;

    std::set<std::string> extra_includes;
    std::set<std::string> extra_impl_includes;
    std::set<std::string> extra_proj_includes;

    std::vector<std::string> extra_member_initialization_list;
    std::vector<std::string> extra_structs;

    std::vector<projection> projections;
    std::vector<derived> derived_projections;

    std::vector<macro_or_const> defined_consts;
    std::vector<macro_or_const> defined_macros;
    std::vector<macro_or_const> defined_parameters;

    // contain functions for use in projections / common code for constructors etc
    std::vector<std::string> inlined_functions;
    std::vector<std::string> setup_functions;
    std::vector<std::string> proj_parameters; // will be written as struct
    std::string setup_function_line;

    projection_properties()
        : valid(true)
        , parstruct_first(true)
        , has_ellipsoid(false)
        , has_spheroid(false)
        , has_guam(false)
    {}

};


}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_STRUCTS_HPP

