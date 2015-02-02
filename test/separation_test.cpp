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

@brief Basic function composition test.
*/
#ifdef HAVE_BOOST_TEST
#include <boost/test/auto_unit_test.hpp>
#else
#include "test_registrar.h"
#endif

#include "iccpp_function.h"
#include "iccpp_color_spaces.h"
#include "iccpp_converters.h"
#include "iccpp_typelist.h"


/**	@file: sepration_test.cpp

    @brief Functions that adds domain adptation and range adaptation
*/
using namespace iccpp;

BOOST_AUTO_TEST_CASE(color_conv_adapter)
{
	function_t<rgb_t<double>, xyz_t> f1(new xyz2rgb_t);

    std::unique_ptr<algo_base_t> algo(adapt_domain<lab_t>(f1.get()));
    BOOST_CHECK(algo.get() != nullptr);
    //std::cout << "Result = " << typeid(*algo).name() << std::endl;

    std::unique_ptr<algo_base_t> algo1(adapt_range<rgb_t<unsigned char >>(algo.get()));
    BOOST_CHECK(algo1.get() != nullptr);
    //std::cout << "Result = " << typeid(*algo1).name() << std::endl;

    std::unique_ptr<algo_base_t> algo2(adapt_range<rgb_t<double>>(algo.get()));
    BOOST_CHECK(algo2.get() != nullptr);
    //std::cout << "Result = " << typeid(*algo2).name() << std::endl;

    std::unique_ptr<algo_base_t> algo3(adapt_domain2<rgb_t<unsigned char >, lab_t>(algo1.get()));
    BOOST_CHECK(algo3.get() != nullptr);
    //std::cout << "Result = " << typeid(*algo3).name() << std::endl;

    std::unique_ptr<algo_base_t> algo4(adapt_domain<rgb_t<float>>(f1.get()));
    BOOST_CHECK(algo4.get() != nullptr);
    std::unique_ptr<algo_base_t> algo5(adapt_range2<rgb_t<unsigned char >, rgb_t<float>>(algo4.get()));
    BOOST_CHECK(algo5.get() != nullptr);
}

/// @brief Test of transform_t::create function
BOOST_AUTO_TEST_CASE(middle_adapter)
{
    function_t<rgb_t<double>, xyz_t> output(new xyz2rgb_t);
    function_t<xyz_t, rgb_t<double>> i1(new rgb2xyz_t);
    function_t<lab_t, rgb_t<double>> input = function_t<lab_t, xyz_t>(new xyz2lab_t) * i1;
    function_t<rgb_t<unsigned char>, 
               rgb_t<unsigned char>> transform = transform_t::create<rgb_t<unsigned char>, 
                                                                     rgb_t<unsigned char>>(output.get(), input.get());

    BOOST_CHECK(transform.get() != nullptr); 
}

