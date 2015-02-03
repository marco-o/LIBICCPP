#ifndef iccpp_rgb_H
#define iccpp_rgb_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
namespace iccpp
{

    template <class Channel>
    struct rgba_t
    {
        using scalar_type = Channel;
        scalar_type red;
        scalar_type green;
        scalar_type blue;
        scalar_type alpha;
    };

    template <class Ch>
    struct rgb_t
    {
        using scalar_type = Ch;
        enum { dimension = 3 };
        scalar_type red;
        scalar_type green;
        scalar_type blue;
        template <class X>
        rgb_t(const rgb_t<X> &lhs) :
            red(static_cast<scalar_type>(lhs.red / scaling_unit<Ch, X, sizeof(Ch) >= sizeof(X)>::value)),
            green(static_cast<scalar_type>(lhs.green / scaling_unit<Ch, X, sizeof(Ch) >= sizeof(X)>::value)),
            blue(static_cast<scalar_type>(lhs.blue / scaling_unit<Ch, X, sizeof(Ch) >= sizeof(X)>::value)) { }
        rgb_t(Ch r, Ch g = 0, Ch b = 0) : red(r), green(g), blue(b) {}
        rgb_t(void) {}
    };

    template <class T, class X>
    rgb_t<typename std::common_type<T, X>::type> operator*(const rgb_t<T> &lhs, X k)
    {
        rgb_t<typename std::common_type<T, X>::type> result = { lhs.red * k, lhs.green * k, lhs.blue * k };
        return result;
    }

    template <class T, class X>
    rgb_t<typename std::common_type<T, X>::type> operator+(const rgb_t<T> &lhs, const rgb_t<X> &rhs)
    {
        rgb_t<typename std::common_type<T, X>::type>
            result = { lhs.red + rhs.red, lhs.green + rhs.green, lhs.blue + rhs.blue };
        return result;
    }

    template <class Channel>
    struct bgr_t
    {
        using scalar_type = Channel;
        enum { dimension = 3 };
        scalar_type blue;
        scalar_type green;
        scalar_type red;
    };
}
#endif
