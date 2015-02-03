#ifndef iccpp_pixel_traits_H
#define iccpp_pixel_traits_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//

/// @file iccpp_pixel_traits.h
/// 
/// @brief traits required to define a vector_t<T, N> based N-dimensional lookup table
///

#include "iccpp_pixel.h"

namespace iccpp
{
    template <class T, int Dim>
    struct lut_input_traits_t<vector_t<T, Dim>>
    {
        typedef T scalar_type;
        typedef typename scalar_traits_t<T>::weight_type weight_type;
        static const scalar_type *scalar(const vector_t<T, Dim> &x) { return x.data(); }
        static void split(const vector_t<T, Dim> &x,
            const vector_t<T, Dim> &step,
            delta_item_t<weight_type> *deltas,
            const size_t *size1)
        {
            iteration_t<T, Dim>::split(x.data(), step.data(), deltas, size1);
        }
        static vector_t<T, Dim> build(const vector_t<T, Dim> &base, const size_t *step)
        {
            vector_t<T, Dim> result;
            iteration_t<T, Dim>::build(base.data(), step, result.data());
            return result;
        }
    };
}

#endif
