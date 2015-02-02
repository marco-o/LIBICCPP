#ifndef iccpp_pixel_H
#define iccpp_pixel_H
//---------------------------------------------------------------------------------
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
    // a bit of metaprogramming t make the 
    // unsigned char case works.
    // (event auto declaration won't work)
    template <class X, class Y, bool> // select the first as default
    struct largest_scalar_imp_t
    {
        typedef X type;
    };

    template <class X, class Y>
    struct largest_scalar_imp_t<X, Y, false>
    {
        typedef Y type;
    };

    template <class X, class Y>
    struct largest_scalar_t
    {
        typedef typename largest_scalar_imp_t<X, Y, (sizeof(X) > sizeof(Y)) >::type type ;
    };

    template <class X, class Y, bool>
    struct scaling_unit
    {
        enum {value = 1};
    };

    template <class X, class Y>
    struct scaling_unit<X, Y, false>
    {
        enum { value = 256 };
    };
    // some metaprogramming stuff
    template <class T, int N>
    struct arithmetic_t
    {
        template <class X>
        static void add(T *dest, const X *src)
        {
            dest[0] += src[0];
            arithmetic_t<T, N - 1>::add(dest + 1, src + 1);
        }
        template <class X>
        static void add(typename largest_scalar_t<X, T>::type *dest, const T *lhs, const X *rhs)
        {
            dest[0] = lhs[0] + rhs[0];
            arithmetic_t<T, N - 1>::add(dest + 1, lhs + 1, rhs + 1);
        }
        static T dot(const T *lhs, const T *rhs)
        {
            return lhs[0] * rhs[0] + arithmetic_t<T, N - 1>::dot(lhs + 1, rhs + 1);
        }
        static void assign(T *dest, T t)
        {
            dest[0] = t;
            arithmetic_t<T, N - 1>::assign(dest + 1, t);
        }
        template <class X>
        static void assign(T *dest, const X *x)
        {
            dest[0] = x[0];
            arithmetic_t<T, N - 1>::assign(dest + 1, x + 1);
        }
        template <class X>
        static void assign_scaled(T *dest, const X *x)
        {
            dest[0] = static_cast<T>(x[0] / scaling_unit<T, X, sizeof(T) >= sizeof(X)>::value);
            arithmetic_t<T, N - 1>::assign_scaled(dest + 1, x + 1);
        }
        template <class X>
        static void multiply(T *dest, X x)
        {
            dest[0] *= x ;
            arithmetic_t<T, N - 1>::template multiply<X>(dest + 1, x);
        }
        template <class X>
        static void multiply(typename largest_scalar_t<X,T>::type *dest, const T *src, X t)
        {
            dest[0] = src[0] * t;
            arithmetic_t<T, N - 1>::template multiply<X>(dest + 1, src + 1, t);
        }
        template <class X>
        static void multiply(typename largest_scalar_t<X, T>::type *dest, const T *lhs, const X *rhs)
        {
            dest[0] = lhs[0] * rhs[0];
            arithmetic_t<T, N - 1>::template multiply<X>(dest + 1, lhs + 1, rhs + 1);
        }
        template <class X>
        static void subtract(T *dest, const X *src)
        {
            dest[0] -= src[0];
            arithmetic_t<T, N - 1>::subtract(dest + 1, src + 1);
        }
        template <class X>
        static void subtract(typename largest_scalar_t<X, T>::type *dest, const T *lhs, const X *rhs)
        {
            dest[0] = lhs[0] - rhs[0];
            arithmetic_t<T, N - 1>::template subtract<X>(dest + 1, lhs + 1, rhs + 1);
        }
    };

    template <class T>
    struct arithmetic_t<T, 0>
    {
        template <class X>
        static void add(T *, const X *) {}
        template <class X>
        static void add(typename largest_scalar_t<X, T>::type *, const T *, const X *){}
        static T dot(const T *, const T *) { return 0; }
        static void assign(T *, T) {}
        template <class X>
        static void assign(T *, const X *) {}
        template <class X>
        static void assign_scaled(T *, const X *) {}
        template <class X>
        static void multiply(typename largest_scalar_t<X, T>::type *, X) {}
        template <class X>
        static void multiply(typename largest_scalar_t<X, T>::type *, const T *, X) {}
        template <class X>
        static void multiply(typename largest_scalar_t<X, T>::type *, const T *, const X *){}
        template <class X>
        static void subtract(T *, const X *) {}
        template <class X>
        static void subtract(typename largest_scalar_t<X, T>::type *, const T *, const X *) {}
    };

    // this is a definition of a compile time vector that could work ss pixel
    // let's start with the easy way, without the integer pomotion business
    template <class T, int N>
    class vector_t
    {
    public:
        enum { dimension = N };
        typedef T scalar_type;
        vector_t(T value = 0)
        {
            arithmetic_t<T, N>::assign(value_, value);
        }
        vector_t(const T *value)
        {
            arithmetic_t<T, N>::assign(value_, value);
        }
        template <class X>
        vector_t(const vector_t<X, N> &value)
        {
            arithmetic_t<T, N>::template assign_scaled<X>(value_, value.data());
        }
        const T *data(void) const { return value_; }
        T *data(void) { return value_; }
        T &operator[](int index) { return value_[index]; }
        const T &operator[](int index) const { return value_[index]; }
        template <class X>
        vector_t<T, N> &operator+=(const vector_t<X, N> &rhs)
        {
            arithmetic_t<T, N>::add(value_, rhs.data());
            return *this;
        }
        vector_t<T, N> &operator-=(const vector_t<T, N> &rhs)
        {
            arithmetic_t<T, N>::subtract(value_, rhs.data());
            return *this;
        }
        vector_t<typename largest_scalar_t<T, scalar_type>::type, N> &operator*=(scalar_type k)
        {
            arithmetic_t<T, N>::template multiply<scalar_type>(value_, k);
            return *this;
        }
        T module2(void) const
        {
            return arithmetic_t<T, N>::dot(value_, value_);
        }
    private:
        scalar_type value_[N];
    };

    template <class T, int N, class X>
    vector_t<typename largest_scalar_t<T, X>::type, N> operator+(const vector_t<T, N> &lhs, const vector_t<X, N> &rhs)
    {
        vector_t<typename largest_scalar_t<T, X>::type, N> result(lhs);
        return result += rhs;
    }

    template <class T, int N, class X>
    vector_t<typename largest_scalar_t<T, X>::type, N> operator-(const vector_t<T, N> &lhs, const vector_t<X, N> &rhs)
    {
        vector_t<typename largest_scalar_t<T, X>::type, N> result(lhs);
        return result -= rhs;
    }

    template <class T, int N, class X>
    vector_t<typename largest_scalar_t<T, X>::type, N> operator*(const vector_t<T, N> &lhs, X k)
    {
        vector_t<typename largest_scalar_t<T, X>::type, N> result(lhs);
        return result *= k;
    }

    template <class Channel>
    struct rgba_t
    {
        using scalar_type = Channel ;
        scalar_type red;
        scalar_type green;
        scalar_type blue;
        scalar_type alpha;
    };

    template <class Ch>
    struct rgb_t
    {
        using scalar_type = Ch;
        enum { dimension = 3 };
        scalar_type red;
        scalar_type green;
        scalar_type blue;
        template <class X>
        rgb_t(const rgb_t<X> &lhs) :
            red  (static_cast<scalar_type>(lhs.red   / scaling_unit<Ch, X, sizeof(Ch) >= sizeof(X)>::value)),
            green(static_cast<scalar_type>(lhs.green / scaling_unit<Ch, X, sizeof(Ch) >= sizeof(X)>::value)),
            blue (static_cast<scalar_type>(lhs.blue  / scaling_unit<Ch, X, sizeof(Ch) >= sizeof(X)>::value)) { }
        rgb_t(Ch r, Ch g = 0, Ch b = 0) : red(r), green(g), blue(b) {}
        rgb_t(void) {}
    };

    template <class T, class X>
    rgb_t<typename largest_scalar_t<T, X>::type> operator*(const rgb_t<T> &lhs, X k)
    {
        rgb_t<typename largest_scalar_t<T, X>::type> result = { lhs.red * k, lhs.green * k, lhs.blue * k };
        return result;
    }

    template <class T, class X>
    rgb_t<typename largest_scalar_t<T, X>::type> operator+(const rgb_t<T> &lhs, const rgb_t<X> &rhs)
    {
        rgb_t<typename largest_scalar_t<T, X>::type>
            result = { lhs.red + rhs.red, lhs.green + rhs.green, lhs.blue + rhs.blue };
        return result ;
    }

    template <class Channel>
    struct bgr_t
    {
        using scalar_type = Channel;
        enum { dimension = 3 };
        scalar_type blue;
        scalar_type green;
        scalar_type red;
    };
}


#endif
