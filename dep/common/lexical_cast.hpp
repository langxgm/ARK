// C++ 11实现的lexical_cast，为了剥离boost
//代码来自: https://github.com/qicosmos/cosmos

#ifndef CPP_11_LEXICAL_CAST_HPP
#define CPP_11_LEXICAL_CAST_HPP

#include <type_traits>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <cstring>
using namespace std;

namespace detail {
static const char* strue = "true";
static const char* sfalse = "false";

template<typename To, typename From>
struct Converter
{
};

// to numeric
template<typename From>
struct Converter<uint8_t, From>
{
    static uint8_t convert(const string& from)
    {
        return std::atoi(from.c_str());
    }

    static uint8_t convert(const char* from)
    {
        return std::atoi(from);
    }
};

template<typename From>
struct Converter<int16_t, From>
{
    static int16_t convert(const string& from)
    {
        return std::atoi(from.c_str());
    }

    static int16_t convert(const char* from)
    {
        return std::atoi(from);
    }
};

template<typename From>
struct Converter<uint16_t, From>
{
    static uint16_t convert(const string& from)
    {
        return std::atoi(from.c_str());
    }

    static uint16_t convert(const char* from)
    {
        return std::atoi(from);
    }
};

template<typename From>
struct Converter<int32_t, From>
{
    static int32_t convert(const string& from)
    {
        return std::atoi(from.c_str());
    }

    static int32_t convert(const char* from)
    {
        return std::atoi(from);
    }
};

template<typename From>
struct Converter<uint32_t, From>
{
    static uint32_t convert(const string& from)
    {
        return std::atoi(from.c_str());
    }

    static uint32_t convert(const char* from)
    {
        return std::atoi(from);
    }
};

template<typename From>
struct Converter<long, From>
{
    static long convert(const string& from)
    {
        return std::atol(from.c_str());
    }

    static long convert(const char* from)
    {
        return std::atol(from);
    }
};

template<typename From>
struct Converter<long long, From>
{
    static long long convert(const string& from)
    {
        return std::atoll(from.c_str());
    }

    static long long convert(const char* from)
    {
        return std::atoll(from);
    }
};

template<typename From>
struct Converter<uint64_t, From>
{
    static uint64_t convert(const string& from)
    {
        return std::atoll(from.c_str());
    }

    static uint64_t convert(const char* from)
    {
        return std::atoll(from);
    }
};

template<typename From>
struct Converter<double, From>
{
    static double convert(const string& from)
    {
        return std::atof(from.c_str());
    }

    static double convert(const char* from)
    {
        return std::atof(from);
    }
};

template<typename From>
struct Converter<float, From>
{
    static float convert(const string& from)
    {
        return (float)std::atof(from.c_str());
    }

    static float convert(const char* from)
    {
        return (float)std::atof(from);
    }
};

// to bool
template<typename From>
struct Converter<bool, From>
{
    static typename std::enable_if<std::is_integral<From>::value, bool>::type convert(From from)
    {
        return !!from;
    }
};

static bool checkbool(const char* from, const size_t len, const char* s)
{
    for (size_t i = 0; i < len; i++)
    {
        if (from[i] != s[i])
        {
            return false;
        }
    }

    return true;
}

static bool convert(const char* from)
{
    size_t len = strlen(from);

    bool r = true;
    if (len == 4)
    {
        //"true"
        r = checkbool(from, len, strue);

        if (r)
            return true;
    }
    else if (len == 5)
    {
        //"false"
        r = checkbool(from, len, sfalse);

        if (r)
            return false;
    }
    else
    {
        // 数字转为bool
        int value = Converter<int, const char*>::convert(from);
        return (value > 0);
    }

    throw std::invalid_argument("argument is invalid");
}

template<>
struct Converter<bool, string>
{
    static bool convert(const string& from)
    {
        return detail::convert(from.c_str());
    }
};

template<>
struct Converter<bool, const char*>
{
    static bool convert(const char* from)
    {
        return detail::convert(from);
    }
};

template<>
struct Converter<bool, char*>
{
    static bool convert(char* from)
    {
        return detail::convert(from);
    }
};

template<unsigned N>
struct Converter<bool, const char[N]>
{
    static bool convert(const char (&from)[N])
    {
        return detail::convert(from);
    }
};

template<unsigned N>
struct Converter<bool, char[N]>
{
    static bool convert(const char (&from)[N])
    {
        return detail::convert(from);
    }
};

// to string
template<typename From>
struct Converter<string, From>
{
    static string convert(const From& from)
    {
        return std::to_string(from);
    }
};
} // namespace detail

template<typename To, typename From>
typename std::enable_if<!std::is_same<To, From>::value, To>::type lexical_cast(const From& from)
{
    return detail::Converter<To, From>::convert(from);
}

template<typename To, typename From>
typename std::enable_if<std::is_same<To, From>::value, To>::type lexical_cast(const From& from)
{
    return from;
}

template<typename ValueTYPE>
bool ValueFromString(const std::string& strValue, ValueTYPE& nValue)
{
    try
    {
        nValue = lexical_cast<ValueTYPE>(strValue);
        return true;
    }
    catch (...)
    {
        return false;
    }

    return false;
}

template<typename ValueTYPE>
bool ValueToString(const ValueTYPE& nValue, std::string& strData)
{
    try
    {
        strData = lexical_cast<std::string>(nValue);
        return true;
    }
    catch (...)
    {
        return false;
    }

    return false;
}
#endif //! CPP_11_LEXICAL_CAST_HPP