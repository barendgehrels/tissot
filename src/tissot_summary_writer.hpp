#ifndef TISSOT_SUMMARY_HPP
#define TISSOT_SUMMARY_HPP

// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/foreach.hpp>
#include "tissot_structs.hpp"


namespace boost { namespace geometry { namespace proj4converter
{



class proj4_summary_writer
{
    public :
        proj4_summary_writer(projection_properties& projpar
                , std::string const& group
                , std::ostream& str)
            : m_prop(projpar)
            , stream(str)
            , projection_group(group)
        {
        }

        void write()
        {
//            stream << projection_group << std::endl;
            std::set<std::string> properties;
            BOOST_FOREACH(projection const& p, m_prop.projections)
            {
                properties.insert(p.model);
                properties.insert(p.direction);
//                stream << "   projection: " << p.model << " " << p.direction << std::endl;
            }

            BOOST_FOREACH(derived const& d, m_prop.derived_projections)
            {
                stream << d.name;
                BOOST_FOREACH(std::string const& s, properties)
                {
                    stream << " " << s;
                }
                stream << std::endl;

//                stream << "   derived: " << d.name << " ";
//                BOOST_FOREACH(std::string const& s, d.parsed_characteristics)
//                {
//                    stream << " " << s;
//                }
//                stream << std::endl;
            }
//            stream << std::endl;
        }

    private :

        std::ostream& stream;
        projection_properties& m_prop;

        std::string projection_group;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_SUMMARY_HPP
