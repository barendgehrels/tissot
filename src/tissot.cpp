// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <string>

#include "tissot_util.hpp"
#include "tissot_structs.hpp"
#include "tissot_parser.hpp"
#include "tissot_converter.hpp"
#include "tissot_summary_writer.hpp"
#include "tissot_bg_writer.hpp"

#include "analyzer.hpp"
#include "documenter.hpp"

#include "converter_aea.hpp"
#include "converter_aeqd.hpp"
#include "converter_aitoff.hpp"
#include "converter_cass.hpp"
#include "converter_cea.hpp"
#include "converter_chamb.hpp"
#include "converter_eqdc.hpp"
#include "converter_etmerc.hpp"
#include "converter_geos.hpp"
#include "converter_gn_sinu.hpp"
#include "converter_goode.hpp"
#include "converter_healpix.hpp"
#include "converter_igh.hpp"
#include "converter_imw_p.hpp"
#include "converter_isea.hpp"
#include "converter_lcca.hpp"
#include "converter_lsat.hpp"
#include "converter_mod_ster.hpp"
#include "converter_ob_tran.hpp"
#include "converter_omerc.hpp"
#include "converter_qsc.hpp"
#include "converter_robin.hpp"
#include "converter_rouss.hpp"
#include "converter_sconics.hpp"
#include "converter_sterea.hpp"
#include "converter_tmerc.hpp"

namespace boost { namespace geometry { namespace proj4converter
{

std::vector<epsg_entry> epsg_entries;

#include "epsg_entries.inc"


converter_cpp_bg_default* get_specific(std::string const& projection_group,
        projection_properties& prop)
{
    return projection_group == "aitoff" ? new converter_cpp_bg_aitoff(prop)
        :  projection_group == "aeqd" ? new converter_cpp_bg_aeqd(prop)
        :  projection_group == "aea" ? new converter_cpp_bg_aea(prop)
        :  projection_group == "cass" ? new converter_cpp_bg_cass(prop)
        :  projection_group == "cea" ? new converter_cpp_bg_cea(prop)
        :  projection_group == "chamb" ? new converter_cpp_bg_chamb(prop)
        :  projection_group == "eqdc" ? new converter_cpp_bg_eqdc(prop)
        :  projection_group == "etmerc" ? new converter_cpp_bg_etmerc(prop)
        :  projection_group == "geos" ? new converter_cpp_bg_geos(prop)
        :  projection_group == "gn_sinu" ? new converter_cpp_bg_gn_sinu(prop)
        :  projection_group == "goode" ? new converter_cpp_bg_goode(prop)
        :  projection_group == "healpix" ? new converter_cpp_bg_healpix(prop)
        :  projection_group == "igh" ? new converter_cpp_bg_igh(prop)
        :  projection_group == "imw_p" ? new converter_cpp_bg_imw_p(prop)
        :  projection_group == "isea" ? new converter_cpp_bg_isea(prop)
        :  projection_group == "lcca" ? new converter_cpp_bg_lcca(prop)
        :  projection_group == "lsat" ? new converter_cpp_bg_lsat(prop)
        :  projection_group == "mod_ster" ? new converter_cpp_bg_mod_ster(prop)
        :  projection_group == "ob_tran" ? new converter_cpp_bg_ob_tran(prop)
        :  projection_group == "omerc" ? new converter_cpp_bg_omerc(prop)
        :  projection_group == "qsc" ? new converter_cpp_bg_qsc(prop)
        :  projection_group == "robin" ? new converter_cpp_bg_robin(prop)
        :  projection_group == "rouss" ? new converter_cpp_bg_rouss(prop)
        :  projection_group == "sconics" ? new converter_cpp_bg_sconics(prop)
        :  projection_group == "sterea" ? new converter_cpp_bg_sterea(prop)
        :  projection_group == "tmerc" ? new converter_cpp_bg_tmerc(prop)
        :  new converter_cpp_bg_default();
}



}}} // namespace boost::geometry::proj4converter


int main (int argc, char** argv)
{
    using namespace boost::geometry::proj4converter;
    fill_epsg_entries();

    if (argc < 3)
    {
        std::cerr << "USAGE: " << argv[0] << " <source file> <group name>" << std::endl;
        return 1;
    }

    std::string filename(argv[1]);
    std::string projection_group(argv[2]);
    projection_properties projprop;

    try
    {
        std::cerr << "Convert " << projection_group << std::endl;
        proj4_parser(projprop, filename, projection_group, epsg_entries);

        proj4_analyzer analyzer(projprop, projection_group);
        analyzer.analyze();

        proj4_documenter documenter(projprop, projection_group);
        documenter.analyze();

        proj4_converter_cpp_bg converter(projprop);
        converter_cpp_bg_default* specific_converter = get_specific(projection_group, projprop);

        specific_converter->pre_convert();
        converter.convert();
        specific_converter->convert();
        converter.post_convert();
        converter.trim();
        converter.scan();

        // Afer parsing and possible modifications of specific converters:
        documenter.create();

        delete specific_converter;

        proj4_writer_cpp_bg writer(projprop, projection_group, epsg_entries, std::cout);
//        proj4_summary_writer writer(projprop, projection_group, std::cout);
        writer.write();
    }
    catch(std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
