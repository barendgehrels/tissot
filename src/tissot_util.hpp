#ifndef TISSOT_UTIL_HPP
#define TISSOT_UTIL_HPP

// Tissot, converts projecton source code (Proj4) to Boost.Geometry
// (or potentially other source code)
//
// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/tokenizer.hpp>

namespace boost { namespace geometry { namespace proj4converter
{


const std::string tab1 = "    ";
const std::string tab2 = tab1 + tab1;
const std::string tab3 = tab2 + tab1;
const std::string tab4 = tab3 + tab1;
const std::string tab5 = tab4 + tab1;


inline bool find(std::string const& line, std::string const& tag, std::string::size_type& loc)
{
    loc = line.find(tag);
    return loc != std::string::npos;
}

inline void strip_comments(std::string& line)
{
    std::string::size_type loc = line.find("/*");
    if (loc == std::string::npos)
    {
        loc = line.find("//");
    }
    if (loc != std::string::npos)
    {
        line.erase(loc);
        boost::trim(line);
    }
}

inline void split(std::string const& line, std::vector<std::string>& subs, std::string const& seps1, std::string const& seps2 = "")
{
    typedef boost::tokenizer<boost::char_separator<char> > TOK;
    TOK tokens(line, boost::char_separator<char>(seps1.c_str(), seps2.c_str()));
    for (TOK::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        if (*it == seps2 && subs.size() > 0)
        {
            subs[subs.size() - 1] += *it;
        }
        else
        {
            subs.push_back(*it);
        }

    }
}

inline std::vector<std::string> extract_names(std::vector<std::string> const& parameters)
{
    std::string pars;
    for (std::vector<std::string>::const_iterator it = parameters.begin(); it != parameters.end(); it++)
    {
        std::string line = *it;
        strip_comments(line);
        std::string::size_type loc = line.find("[");
        if (loc != std::string::npos)
        {
            line.erase(loc);
            line += ";";
        }
        pars += line;
    }
    typedef boost::tokenizer<boost::char_separator<char> > TOK;
    std::vector<std::string> retval;
    TOK tokens(pars, boost::char_separator<char>(" ,;*"));
    for (TOK::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        if (! (*it == "double" || *it == "int" || *it == "void"))
        {
            retval.push_back(*it);
        }
    }
    return retval;
}

inline std::string end_entry(std::string const& line)
{
    // Process it
    std::string parameters = boost::trim_copy(line);
    boost::replace_all(parameters, "ENDENTRY", "");
    boost::replace_first(parameters, "(", "");
    boost::replace_last(parameters, ")", "");
    boost::replace_first(parameters, "(P", "(par");
    if (parameters == "P")
    {
        parameters.clear();
    }
    return parameters;
}


struct functor_starts_with
{
    explicit functor_starts_with(std::string const& line)
    {
        tokens.push_back(line);
    }
    functor_starts_with(std::string const& line1, std::string const& line2)
    {
        tokens.push_back(line1);
        tokens.push_back(line2);
    }
    functor_starts_with(std::string const& line1, std::string const& line2, std::string const& line3)
    {
        tokens.push_back(line1);
        tokens.push_back(line2);
        tokens.push_back(line3);
    }
    functor_starts_with(std::string const& line1, std::string const& line2, std::string const& line3, std::string const& line4)
    {
        tokens.push_back(line1);
        tokens.push_back(line2);
        tokens.push_back(line3);
        tokens.push_back(line4);
    }
    functor_starts_with& add(std::string const& s)
    {
        tokens.push_back(s);
        return *this;
    }

    inline bool operator()(std::string const& line) const
    {
        std::string const& trimmed = boost::trim_copy(line);
        for (std::vector<std::string>::const_iterator it = tokens.begin();
            it != tokens.end(); ++it)
        {
            if (boost::starts_with(trimmed, *it))
            {
                return true;
            }
        }
        return false;
    }
private:
    std::vector<std::string> tokens;
};


struct functor_trimmed_equals
{
    explicit functor_trimmed_equals(std::string const& line)
    {
        tokens.push_back(line);
    }
    functor_trimmed_equals(std::string const& line1, std::string const& line2)
    {
        tokens.push_back(line1);
        tokens.push_back(line2);
    }

    inline bool operator()(std::string const& line) const
    {
        std::string const& trimmed = boost::trim_copy(line);
        for (std::vector<std::string>::const_iterator it = tokens.begin();
            it != tokens.end(); ++it)
        {
            if (boost::equals(trimmed, *it))
            {
                return true;
            }
        }
        return false;
    }
private:
    std::vector<std::string> tokens;
};

struct functor_nospaces_equals
{
    explicit functor_nospaces_equals(std::string const& line)
    {
        tokens.push_back(line);
    }
    functor_nospaces_equals(std::string const& line1, std::string const& line2)
    {
        tokens.push_back(line1);
        tokens.push_back(line2);
    }
    functor_nospaces_equals& add(std::string const& s)
    {
        tokens.push_back(s);
        return *this;
    }

    inline bool operator()(std::string const& line) const
    {
        std::string trimmed = boost::trim_copy(line);
        boost::replace_all(trimmed, " ", "");
        for (std::vector<std::string>::const_iterator it = tokens.begin();
            it != tokens.end(); ++it)
        {
            if (boost::equals(trimmed, *it))
            {
                return true;
            }
        }
        return false;
    }
private:
    std::vector<std::string> tokens;
};

struct functor_equals_in_named_struct
{
    explicit functor_equals_in_named_struct(std::string const& line)
    {
        tokens.push_back(line);
    }
    functor_equals_in_named_struct(std::string const& line1, std::string const& line2)
    {
        tokens.push_back(line1);
        tokens.push_back(line2);
    }
    functor_equals_in_named_struct& add(std::string const& s)
    {
        tokens.push_back(s);
        return *this;
    }

    template <typename T>
    inline bool operator()(T const& item) const
    {
        for (std::vector<std::string>::const_iterator it = tokens.begin();
            it != tokens.end(); ++it)
        {
            if (item.name == *it)
            {
                return true;
            }
        }
        return false;
    }
private:
    std::vector<std::string> tokens;
};

}}} // namespace boost::geometry::proj4converter


#endif // TISSOT_UTIL_HPP

