#ifndef test_registrar_H
#define test_registrar_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

#include "iccpp_config.h"

namespace test
{
    template <class F>
    class registrar_t
    {
    public:
#ifdef _MSC_VER
        struct leak_detector_t
        {
            ~leak_detector_t(void)
            {
                _CrtDumpMemoryLeaks();
            }
        };
 #endif
       using entry_t = std::pair < std::string, F>;
        typedef std::vector<entry_t> entries_vect_t;
        registrar_t(F f, const std::string &key)
        {
            entries().push_back(entry_t(key, f));
        }
        static bool get(const std::string &key, F &f)
        {
            auto it = std::find_if(entries().begin(), entries().end(),
                            [&key](entry_t &entry){ return entry.first == key; });
            if (it == entries().end())
                return false;
            f = it->second;
            return true;
        }
        static entries_vect_t &entries(void)
        {
#ifdef _MSC_VER
            static leak_detector_t dummy;
#endif
            static entries_vect_t the_entries;
            return the_entries;
        }
    };

} // namespace test

typedef test::registrar_t<void(*)(void)> funct_registrar_t;

#define BOOST_AUTO_TEST_CASE(x)  void x(void) ; funct_registrar_t reg##x(x, #x) ; void x(void)
#define BOOST_CHECK(x) if (!(x)) std::cout << __FILE__ << ":" << __LINE__ << ": Test " << #x << " failed." <<std::endl ;

#endif
