#ifndef iccpp_utils_H
#define iccpp_utils_H
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
#include "iccpp_pixel.h"

namespace iccpp
{
	template <class X>
	class identity_t : public algo_t<X, X>
	{
	public:
		virtual X eval(const X &x) const override
		{
			return x;
		}
		virtual identity_t<X> *clone(void) const override
		{
			return new identity_t();
		}
	};

	template <class P>
	class brighten_t : public algo_t<P, P>
	{
	public:
		brighten_t(double value)
		{
			for (int i = 0; i < 256; i++)
				lut_[i] = static_cast<unsigned char>(255.0 * pow(i / 255., value) + .5);
		}
		virtual P eval(const P &pixel) const override
		{
			P result(pixel);
			result.red = lut_[pixel.red];
			result.green = lut_[pixel.green];
			result.blue = lut_[pixel.blue];
			return result;
		}
		virtual brighten_t<P> *clone(void) const override
		{
			return new brighten_t(*this);
		}
	private:
		unsigned char lut_[256];
	};
}
#endif
