#ifndef icpp_image_H
#define icpp_image_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <cstring>
#include "iccpp_function.h"

namespace iccpp
{
	struct point_t
	{
		int x;
		int y;
	};

	template <class P>
	class image_t
	{
	public:
		typedef P pixel_type;
		image_t(const point_t &size) : size_(size), owned_(true)
		{
			line_size_ = size.x * sizeof(pixel_type);
			data_ = new pixel_type[size.x * size.y];
		}
	    image_t(pixel_type *data, const point_t &size, int linesize = 0) : data_(data), size_(size), owned_(false)
		{
			if (linesize == 0)
				line_size_ = size.x * sizeof(pixel_type);
			else
				line_size_ = linesize;
		}
		~image_t(void)
		{
			if (owned_)
				delete[] data_;
		}
		pixel_type *operator[](int line) 
		{
			return reinterpret_cast<pixel_type *>(reinterpret_cast<char *>(data_) + line_size_ * line);
		}
		const pixel_type *operator[](int line) const
		{
			return reinterpret_cast<const pixel_type *>(reinterpret_cast<const char *>(data_) + line_size_ * line);
		}
		point_t size(void) const { return size_; }
		void copy(const image_t<pixel_type> &src)
		{
			int length = sizeof(pixel_type)* size_.x;
			for (int i = 0; i < size_.y; i++)
                std::memcpy(this->operator[](i), src[i], length);
		}

        template <class P1>
		void apply(const function_t<P, P1> &f, const image_t<P1> &img)
		{			
			for (int i = 0; i < size_.y; i++)
			{
				pixel_type *dest = this->operator[](i);
                const typename image_t<P1>::pixel_type *src = img[i];
				for (int j = 0; j < size_.x; j++)
					dest[j] = f(src[j]);					
			}
		}
		template <class F>
		void apply(const F &f)
		{
			apply(f, *this);
		}
	private:
		pixel_type *data_;
		point_t     size_;
		int         line_size_;
		bool        owned_;
	};
}

#endif
