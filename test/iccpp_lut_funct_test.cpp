//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//

/**	@file: iccpp_lut_funct_test.cpp

    @brief Test of the N-dimensional lookup table
*/

#ifdef HAVE_BOOST_TEST
#include <boost/test/auto_unit_test.hpp>
#else
#include "test_registrar.h"
#endif
#include "iccpp_lut_funct.h"
#include "iccpp_utils.h"

using namespace iccpp;

// ==============================================================================
// LUT building Test Cases
// ==============================================================================

template <class S, int N>
class test_funct_t : public algo_t<vector_t<S, N>, vector_t<S, N>>
{
public:
    using vector = vector_t<S, N>;
    virtual vector eval(const vector &x) const override
    {
        vector result;
        for (size_t i = 0; i < N; i++)
            result[i] = static_cast<S>(x[i] / (i + 1));
        return result;
    }
    virtual test_funct_t<S, N> *clone(void) const override
    {
        return new test_funct_t<S, N>;
    }
};

/// @brief Initializes a lut given the number of steps (assumes equal on all dimensions),
//         the input space dimension and the test function defined above.
///        For integer T steps-1 must divide 255
template <class T, int N, class I>
bool test_lut(size_t steps)
{
	using vector = vector_t<T, N>;
    function_t<vector, vector> seed(new test_funct_t<T, N>);
    function_t<vector, vector> funct(make_lut<vector, vector, I>(seed, steps));
	vector x ;
    for (size_t k = 0; k < 100; k++)
    {
	    for (size_t i = 0; i < N; i++)
		    x[i] = static_cast<T>(scalar_traits_t<T>::one() * (std::rand() % 237) / static_cast<T>(236));
	    vector y = funct(x);
        vector d = seed(x) - y;
        BOOST_CHECK(d.module2() < scalar_traits_t<T>::one() * scalar_traits_t<T>::one() / 500);
    }
    return true;
}

// test case for a floating point lut given a few dimensions 
BOOST_AUTO_TEST_CASE(lut_double)
{
    test_lut<double, 2, interp_tetra_t>(6);
    test_lut<double, 3, interp_tetra_t>(6);
    test_lut<double, 4, interp_tetra_t>(6);
    test_lut<double, 8, interp_tetra_t>(4);
    test_lut<double, 12, interp_tetra_t>(3);
}

// same as above but using unsigned char as scalar type
template <int N>
bool test_lut_uchar(size_t steps = 6)
{
    return test_lut<unsigned char, N, interp_tetra_t>(steps);
}

BOOST_AUTO_TEST_CASE(lut_uchar)
{
	BOOST_CHECK(test_lut_uchar<2>());
	BOOST_CHECK(test_lut_uchar<3>());
	BOOST_CHECK(test_lut_uchar<4>());
	BOOST_CHECK(test_lut_uchar<8>(4));
	BOOST_CHECK(test_lut_uchar<12>(4));
}

// another flavor, this time is multi linear interpolation
template <int N>
bool test_multi_double(size_t steps = 6)
{
    return test_lut<double, N, interp_multi_t>(steps);
}

BOOST_AUTO_TEST_CASE(lut_multi)
{
    BOOST_CHECK(test_multi_double<2>());
    BOOST_CHECK(test_multi_double<3>());
    BOOST_CHECK(test_multi_double<4>());
    BOOST_CHECK(test_multi_double<8>(4));
    BOOST_CHECK(test_multi_double<12>(4));
}

namespace iccpp
{
    template <class T>
    struct lut_input_traits_t<rgb_t<T>>
    {
        typedef T scalar_type;
        typedef typename scalar_traits_t<T>::weight_type weight_type;
        //static const scalar_type *scalar(const rgb_t<T> &x) { return &x.red; }
        static void split(const rgb_t<T> &x,
                          const rgb_t<T> &step,
                          delta_item_t<weight_type> *deltas,
                          const size_t *size1)
        {
            split_0(x.red,   step.red,   deltas,     size1);
            split_0(x.green, step.green, deltas + 1, size1 + 1);
            split_0(x.blue,  step.blue,  deltas + 2, size1 + 2);
        }
        static rgb_t<T> build(const rgb_t<T> &base, const size_t *step)
        {
            return rgb_t<T>(static_cast<T>(base.red   * step[0]), 
                            static_cast<T>(base.green * step[1]),
                            static_cast<T>(base.blue  * step[2]));
        }
        static void split_0(T x, T step, delta_item_t<weight_type> *deltas, const size_t *size1)
        {
            T div = x / step;
            deltas->offset = static_cast<size_t>(div);
            // when this happens we are at the extreme end of a hypercube, ending
            // up in a space not initialized. In this case the code adds an elements 
            // with weight 0 but pointing to an unintitalized memory
            // This is he 'teoric' code, below the one that avoids the if
            // if (deltas->offset >= size1[0])
            //     deltas->offset = size1[0] - 1;
            deltas->offset -= deltas->offset / size1[0];
           // deltas->value = x - deltas->offset * step;
            deltas->value = x * scalar_traits_t<T>::one() / step - deltas->offset * scalar_traits_t<T>::one();
        }

    };

}

template <class T>
bool test_lut_rgb(size_t steps)
{
    using pixel_t = rgb_t<T> ;
    T step1 = static_cast<T>(scalar_traits_t<T>::cube_size() / (steps - 1));
    pixel_t delta = { step1, step1, step1 };
    size_t steps1[] = { steps, steps, steps };
    function_t<pixel_t, pixel_t> seed(new identity_t<pixel_t>);
    function_t<pixel_t, pixel_t> funct(make_lut<pixel_t, pixel_t, interp_tetra_t>(seed, steps1, delta));
    pixel_t x(static_cast<T>(step1 *0.75), static_cast<T>(step1 * 1.2), static_cast<T>(step1 * 2.3));
    pixel_t y = funct(x);
    T diff = (x.blue - y.blue) * (x.blue - y.blue) + 
             (x.green - y.green) * (x.green - y.green) + 
             (x.red - y.red) * (x.red - y.red);
    return diff < 1;
}

BOOST_AUTO_TEST_CASE(lut_rgb)
{
    test_lut_rgb<unsigned char>(6);
}