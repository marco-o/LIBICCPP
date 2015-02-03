#ifndef iccpp_rgb_traits_H
#define iccpp_rgb_traits_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//

/// @file iccpp_rgb_traits.h
/// 
/// @brief traits required to define a RGB based 3-dimensional lookup table
///

#include "iccpp_rgb.h"

namespace iccpp
{
    //
    // This is the stuff required to make a rgb_t<T> 3-dimensional lookup table work
    //

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
            split_0(x.red, step.red, deltas, size1);
            split_0(x.green, step.green, deltas + 1, size1 + 1);
            split_0(x.blue, step.blue, deltas + 2, size1 + 2);
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
            deltas->value = x * scalar_traits_t<T>::cube_size() / step - deltas->offset * scalar_traits_t<T>::cube_size();
        }

    };

}
#endif