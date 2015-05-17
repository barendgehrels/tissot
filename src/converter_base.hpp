#ifndef TISSOT_CONVERTER_BASE_HPP
#define TISSOT_CONVERTER_BASE_HPP

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


namespace boost { namespace geometry { namespace proj4converter
{


class converter_cpp_bg_default
{
public :

    virtual void pre_convert() {}
    virtual void convert() {}
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_CONVERTER_BASE_HPP
