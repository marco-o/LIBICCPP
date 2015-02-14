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

/**	@file: profiles_test.cpp

    @brief Testing of functions that load a profile
*/

#ifdef HAVE_BOOST_TEST
#include <boost/test/auto_unit_test.hpp>
#else
#include "test_registrar.h"
#endif
using namespace iccpp;

#if 0
// that's atest case used mainly for early development
std::unique_ptr<profile_t> handler(profile_t::create("profile.icc"));
std::unique_ptr<profile_t> handler1(profile_t::create_sRGB());
BOOST_CHECK(handler);
if (!handler)
return;
handler->list_tags(std::cout);
handler->load_tag(tag_signature_t::BToA1Tag);
//	handler->load_all();
#endif

BOOST_AUTO_TEST_CASE(icc_loader)
{
	std::unique_ptr<profile_t> handler(profile_t::create("profile.icc"));
    std::unique_ptr<profile_t> handler1(profile_t::create_sRGB());
	BOOST_CHECK(handler);
    if (!handler)
        return ;

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
    BOOST_CHECK(fc);
    BOOST_CHECK(gc);
    lab = gc(xc);
    rgb_t<unsigned char> yc = fc(lab);
    (void)yc; 
}

/// @brief test of the sRGB built-in profile
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


BOOST_AUTO_TEST_CASE(icc_cmyk)
{
    using cmyk_t = vector_t<double, 4>;
    std::unique_ptr<profile_t> handler(profile_t::create("cmyk.icc"));
    BOOST_CHECK(handler);
    if (!handler)
        return;
    function_t<lab_t, cmyk_t> f = handler->device2pcs<lab_t, cmyk_t>();
    BOOST_CHECK(f);
    function_t<xyz_t, cmyk_t> h = handler->device2pcs<xyz_t, cmyk_t>();
    BOOST_CHECK(h);
    function_t<cmyk_t, lab_t> g = handler->pcs2device<cmyk_t, lab_t>();
    BOOST_CHECK(g);
    function_t<cmyk_t, xyz_t> l = handler->pcs2device<cmyk_t, xyz_t>();
    BOOST_CHECK(l);
    // this one shoud fail t load silently, so is OK that result is nullptr
    function_t<cmyk_t, rgb_t<double>> s = handler->pcs2device<cmyk_t, rgb_t<double>>();
    BOOST_CHECK(s.get() == nullptr);

    using cmyku_t = vector_t<unsigned char, 4>;
    function_t<lab_t, cmyku_t> fu = handler->device2pcs<lab_t, cmyku_t>();
    BOOST_CHECK(fu);
    function_t<cmyku_t, lab_t> gu = handler->pcs2device<cmyku_t, lab_t>();
    BOOST_CHECK(gu);
}
