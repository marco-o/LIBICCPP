//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//

//
// Test of the N-dimensional lookup table
//
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

// initializes a lut given the steps, the inut space dimension and the test function defined above
// for integer T steps-1 must divide 255
template <class T, int N, class I>
bool test_lut(size_t steps)
{
	using vector = vector_t<T, N>;
    iccpp::function_t<vector, vector> seed(new test_funct_t<T, N>);
    iccpp::function_t<vector, vector> funct(make_lut<vector, vector, I>(seed, steps));
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

// test of a floating point lut given a few dimensions 
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

