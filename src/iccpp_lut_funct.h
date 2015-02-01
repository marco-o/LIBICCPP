#ifndef icc_lut_funct_H
#define icc_lut_funct_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "iccpp_function.h"
#include "iccpp_clut.h"

namespace iccpp
{
	/*
	 * Turn a given function into an equivalent one based on a lookup table
	 */
    template <class Y, class X, class I = interp_tetra_t>
	function_t<Y, X> make_lut(const function_t<Y, X> &f, const size_t *steps, const X &step)
	{
        using scalar_type = typename X::scalar_type;
        std::unique_ptr< lut_t<Y, X, I> > lut(new lut_t<Y, X, I>(steps, step));
		size_t size = lut->datasize();
		for (size_t i = 0; i < size; i++)
		{
			size_t components[X::dimension];
			for (size_t j = 0, t = i; j < X::dimension; j++)
			{
				components[j] = t % steps[j];
				t /= steps[j];
			}
			X x = lut_input_traits_t<X>::build(step, components);
			(*lut)[i] = f(x);
		}
		return function_t<Y, X>(lut.release());
	}

	template <class Y, class X, class I>
	function_t<Y, X> make_lut(const function_t<Y, X> &f, size_t steps)
	{
		using scalar_type = typename X::scalar_type;
		X dx(static_cast<scalar_type>(scalar_traits_t<scalar_type>::cube_size() / (steps - 1)));
		size_t gridsteps[X::dimension];

		for (size_t i = 0; i < X::dimension; i++)
			gridsteps[i] = steps;
		return make_lut<X, Y, I>(f, gridsteps, dx);
	}
} // namespace iccpp

#endif