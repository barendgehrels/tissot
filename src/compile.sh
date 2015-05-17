# Tissot, converts projecton source code (Proj4) to Boost.Geometry
# (or potentially other source code)
#
# Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
#
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# Example compilation
# gcc should also work
# It should also be able to compile it with MSVC

clang -I . -I ~/git/boost/modular-boost/ -o ../bin/tissot tissot.cpp -lstdc++
