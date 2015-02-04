#ifndef iccpp_traits_h
#define iccpp_traits_h
//----------------------------------------------------------------------------
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
    /// @brief This trait is useful to make the same work for both 
    ///        integer and floating point types
    ///
    template <class T>
    struct scalar_traits_t
    {
        typedef T scalar_type;
        typedef T weight_type;
        static T one(void) { return 1; }
    };

    template <>
    struct scalar_traits_t<unsigned char>
    {
        typedef int weight_type;
        typedef unsigned char scalar_type;
        static int          one(void) { return 255; }
    };

    template <>
    struct scalar_traits_t<int>
    {
        typedef int weight_type;
        typedef int scalar_type;
        static int          one(void) { return 255; }
    };

    // if X is smaller (false) then there is a down scaling
    template <class X, class Y, bool>
    struct scaling_unit
    {
        enum { value = 1 };
    };

    template <class X, class Y>
    struct scaling_unit<X, Y, false>
    {
        // should be scalar_traits_t<X>::one, but cannot use a function to init an enum...
        enum { value = 255 };
    };

}


#endif
