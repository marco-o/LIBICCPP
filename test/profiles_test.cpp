//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "iccpp_profile.h"

#ifdef HAVE_BOOST_TEST
#include <boost/test/auto_unit_test.hpp>
#else
#include "test_registrar.h"
#endif
using namespace iccpp;

BOOST_AUTO_TEST_CASE(icc_loader)
{
	std::unique_ptr<profile_t> handler(profile_t::create("profile.icc"));
    std::unique_ptr<profile_t> handler1(profile_t::create_sRGB());
	BOOST_CHECK(handler);
	handler->list_tags(std::cout);
	handler->load_tag(tag_signature_t::BToA1Tag);
//	handler->load_all();

    lab_t lab;

    // to make this case work one needs to add proper type at the end of the chain...
    rgb_t<double> x = { 0.3764, 0.5019, 0.5647 };
    function_t<rgb_t<double>, lab_t> f = handler->pcs2device<rgb_t<double>, lab_t>();
    function_t<lab_t, rgb_t<double>> g = handler1->device2pcs<lab_t, rgb_t<double>>();

    lab = g(x);
    rgb_t<double> y = f(lab);
    (void)y;
    BOOST_CHECK(f);
    BOOST_CHECK(g);

    // now check that an extra conversion is possible
    rgb_t<unsigned char> xc = { 30, 60, 90 };
    function_t<rgb_t<unsigned char>, lab_t> fc = handler->pcs2device<rgb_t<unsigned char>, lab_t>();
    function_t<lab_t, rgb_t<unsigned char>> gc = handler1->device2pcs<lab_t, rgb_t<unsigned char>>();
    lab = gc(xc);
    rgb_t<unsigned char> yc = fc(lab);
    (void)yc; 
}

BOOST_AUTO_TEST_CASE(icc_srgb)
{
	std::unique_ptr<profile_t> handler(profile_t::create_sRGB());
	rgb_t<double> rgb = {0.3764, 0.5019, 0.5647};
	function_t<xyz_t, rgb_t<double>> f = handler->device2pcs<xyz_t, rgb_t<double>>();
	xyz_t xyz = f(rgb);

	function_t<rgb_t<double>, xyz_t> g = handler->pcs2device<rgb_t<double>, xyz_t>();
	rgb_t<double> res = g(xyz);
    res.red   -= rgb.red;
    res.green -= rgb.green;
    res.blue  -= rgb.blue;
    double err = res.red * res.red + res.green * res.green + res.blue * res.blue;
    BOOST_CHECK(err<1e-6);
}