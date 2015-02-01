#ifndef iccpp_streamio_H
#define iccpp_streamio_H
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
	template <class stream_t>
	stream_t &operator<<(stream_t &ost, const xyz_t &xyz)
	{
		return ost << xyz.x << ", " << xyz.y << ", " << xyz.z ;
	}

	template <class stream_t>
	stream_t &operator<<(stream_t &ost, const lab_t lab)
	{
		return ost << lab.L << ", " << lab.a << ", " << lab.b;
	}

	template <class stream_t, class channel_t>
	stream_t &operator<<(stream_t &ost, const rgb_t<channel_t> &rgb)
	{
		return ost << rgb.r << ", " << rgb.g << ", " << rgb.b ;
	}

	template <class stream_t, class channel_t, int N>
	stream_t &operator<<(stream_t &ost, const vector_t<channel_t, N> &v)
	{
		for (size_t i = 0; i < N; i++)
			ost << v[i] << (i == N - 1 ? "" : ", ");
		return ost;
	}
}
#endif