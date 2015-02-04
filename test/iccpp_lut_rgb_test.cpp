//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//

/**	@file: iccpp_lut_rgb_test.cpp

    @brief Test of the rgb<T> based 3-dimensional lookup table
*/
#ifdef HAVE_BOOST_TEST
#include <boost/test/auto_unit_test.hpp>
#else
#include "test_registrar.h"
#endif
#include "iccpp_lut_funct.h"
#include "iccpp_rgb_traits.h"

using namespace iccpp;

template <class T>
bool test_lut_rgb(size_t steps)
{
    using pixel_t = rgb_t<T>;
    T step1 = static_cast<T>(scalar_traits_t<T>::one() / (steps - 1));
    pixel_t delta = { step1, step1, step1 };
    size_t steps1[] = { steps, steps, steps };
    function_t<pixel_t, pixel_t> seed(new identity_t<pixel_t>);
    function_t<pixel_t, pixel_t> funct(make_lut<pixel_t, pixel_t, interp_tetra_t>(seed, steps1, delta));
    pixel_t x(static_cast<T>(step1 *0.75), static_cast<T>(step1 * 1.2), static_cast<T>(step1 * 2.3));
    pixel_t y = funct(x);
    T diff = (x.blue - y.blue) * (x.blue - y.blue) +
        (x.green - y.green) * (x.green - y.green) +
        (x.red - y.red) * (x.red - y.red);
    return diff <= 3;
}

// (steps - 1) must divide 255
BOOST_AUTO_TEST_CASE(lut_rgb_uchar)
{
    BOOST_CHECK(test_lut_rgb<unsigned char>(6));
    BOOST_CHECK(test_lut_rgb<unsigned char>(18));
}

BOOST_AUTO_TEST_CASE(lut_rgb_double)
{
    BOOST_CHECK(test_lut_rgb<double>(5));
    BOOST_CHECK(test_lut_rgb<double>(9));
    BOOST_CHECK(test_lut_rgb<double>(17));
}