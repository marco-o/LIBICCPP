//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//

/**	@file: function_test.cpp

@brief Basic function composition test with some simple concrete functions
*/

#ifdef HAVE_BOOST_TEST
#define BOOST_TEST_MODULE FunctionsUnitTest
#include <boost/test/auto_unit_test.hpp>
#else
#include "test_registrar.h"
#endif

#include <iostream>

#include "iccpp_image.h"
#include "iccpp_function.h"
#include "iccpp_profile.h"
#include "iccpp_clut.h"
#include "iccpp_color_spaces.h"


namespace iccpp
{
    template <class P>
    class brighten_t : public algo_t<P, P>
    {
    public:
        brighten_t(double value)
        {
            for (int i = 0; i < 256; i++)
                lut_[i] = static_cast<unsigned char>(255.0 * pow(i / 255., value) + .5);
        }
        virtual P eval(const P &pixel) const override
        {
            P result(pixel);
            result.red = lut_[pixel.red];
            result.green = lut_[pixel.green];
            result.blue = lut_[pixel.blue];
            return result;
        }
        virtual brighten_t<P> *clone(void) const override
        {
            return new brighten_t(*this);
        }
    private:
        unsigned char lut_[256];
    };

}
using namespace iccpp;


#pragma region real_cases
BOOST_AUTO_TEST_CASE(function_identity_eval) {
    identity_t<int> id;
    BOOST_CHECK(1 == id.eval(1));
    function_t<int, int> funct(new identity_t<int>);
    BOOST_CHECK(1 == funct(1));
}


BOOST_AUTO_TEST_CASE(image_compile)
{
    unsigned char data[4] = { 0, 1, 2, 3 };
    point_t size = { 2, 2 };
    image_t<unsigned char> img(data, size);
    BOOST_CHECK(img[0][0] == 0);
    BOOST_CHECK(img[0][1] == 1);
    BOOST_CHECK(img[1][0] == 2);
    BOOST_CHECK(img[1][1] == 3);

}

BOOST_AUTO_TEST_CASE(image_brighten)
{
    using pixel_t = iccpp::rgba_t<unsigned char> ;
    pixel_t data[4] = { 0, 0, 0, 0, 64, 64, 64, 64, 2, 2, 2, 2, 3, 3, 3, 3 };
    point_t size = { 2, 2 };
    image_t<pixel_t> img(data, size);
    function_t<pixel_t, pixel_t>  f(new brighten_t<pixel_t>(0.5));
    img.apply(f);
    BOOST_CHECK(img[0][1].red == 128);
}

//b test on xyz <--> lab conversions

BOOST_AUTO_TEST_CASE(color_conversions)
{
    xyz_t xyz = { 0.3, 0.2, 0.1 };

    function_t<lab_t, xyz_t>  xyz2lab(new xyz2lab_t(xyz_t::D50()));
    function_t<xyz_t, lab_t>  lab2xyz(new lab2xyz_t(xyz_t::D50()));
    lab_t lab = xyz2lab(xyz);
    xyz_t clr = lab2xyz(lab);
    BOOST_CHECK(fabs(clr.x - xyz.x) < 1e-3);

    function_t<rgb_t<double>, xyz_t>  xyz2rgb(new xyz2rgb_t);
    function_t<xyz_t, rgb_t<double>>  rgb2xyz(new rgb2xyz_t);
    rgb_t<double> rgb = xyz2rgb(xyz);
    xyz_t clr1 = rgb2xyz(rgb);
    BOOST_CHECK(fabs(clr1.x - xyz.x) < 1e-3);

}

template <int N>
bool check_permutations(void)
{
    size_t data[N];
    for (size_t i = 0; i < N; i++)
        data[i] = i;
    permutation_t<N, false> perm;
    do
    {
        delta_item_t<size_t> deltas[N];
        for (size_t i = 0; i < N; i++)
        {
            deltas[i].value = data[i];
            deltas[i].offset = i;
        }
        perm.indexes(deltas);
        for (size_t j = 0; j < N - 1; j++)
            if (deltas[j] < deltas[j + 1]) // input is sorted in decreasing order...
                return false;
    } while (std::next_permutation(data, data + N));
    return true;
}

// simple test on a component for n-dimensionl look-up table 
BOOST_AUTO_TEST_CASE(icc_permutation)
{
    size_t data[][4] = 
    {
        {0, 1, 2, 0x7}, // last number if the result
        {1, 0, 2, 0x6},
        {2, 1, 0, 0x0},
        {1, 2, 0, 0x1}
    };
    using perm3_t = permutation_t<3, false>;
    perm3_t perm ;
    for (size_t i = 0; i < sizeof(data) / sizeof(size_t[4]); i++)
        BOOST_CHECK(perm3_t::signature(data[i]) == data[i][3]);
    BOOST_CHECK(check_permutations<3>());
    BOOST_CHECK(check_permutations<4>());
    BOOST_CHECK(check_permutations<5>());

    delta_item_t<double> deltas[3] = { { 0.2, 0 }, { 0.4, 1 }, { 0.1, 2 } };
    perm.signature(deltas);
}



