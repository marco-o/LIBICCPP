#ifndef iccpp_typelist_h
#define iccpp_typelist_h
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "iccpp_color_spaces.h"

namespace iccpp
{
    struct type_list_end_t {};

    template <class U, class V>
    struct type_list_t
    {
        typedef U head;
        typedef V tail;
    };

    template <class X, class K>
    struct visitor_domain_chain_t {};

    template <class H, class T, class K>
    struct visitor_domain_chain_t<type_list_t<H, T>, K>
    {
        class type : virtual public visitor_domain_t<H>,
                     virtual public visitor_domain_chain_t<T, K>::type
        {
        public:
            virtual void visit_domain(algo_domain_t<H> &algo) 
            {
                this->domain(algo);
            }
        };
    };

    template <class K>
    struct visitor_domain_chain_t<type_list_end_t, K>
    {
        typedef K type;
    };

    template <class X, class K>
    struct visitor_range_chain_t {};

    template <class H, class T, class K>
    struct visitor_range_chain_t<type_list_t<H, T>, K>
    {
        class type : virtual public visitor_range_t<H>,
                     virtual public visitor_range_chain_t<T, K>::type
        { 
        public:
            virtual void visit_range(algo_range_t<H> &algo)
            {
                this->range(algo);
            }
        };
    };

    template <class K>
    struct visitor_range_chain_t<type_list_end_t, K>
    {
        typedef K type;
    };

    typedef type_list_t < xyz_t,
            type_list_t < lab_t,
            type_list_t<rgb_t<double>, type_list_end_t >> > color_space_list_t;


}

#endif
