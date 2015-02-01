
#ifndef iccpp_color_spaces_h
#define iccpp_color_spaces_h
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
#include "iccpp_traits.h"
#include <math.h>

//
// definition of some common color spaces and conversion between them
//
namespace iccpp
{

    struct xyz_t
    {
        double x;
        double y;
        double z;
        // there are the two most common white points
        static xyz_t D65(void) { return xyz_t{ 0.9504, 1.0, 1.0891 }; } // D65 XYZ normalized to Y=1.0
        static xyz_t D50(void) { return xyz_t{ 0.9642, 1.0, 0.8249 }; } // D50 XYZ normalized to Y=1.0
    };

    struct lab_t
    {
        double L;
        double a;
        double b;
    };

    struct domain_null_t{};
    ///
    /// @class color_conversion_t
    /// @brief template class doing generic color conversion between two spaces
    ///        There is no implementation for a such generic class, 
    ///        but many specialized implementations that handle specific cases
    template <class Y, class X>
    class color_conversion_t : public algo_t<Y, X>
    {
    public: 
        /// @brief This typedef indicates that generic conversion is not upported
        ///        Any specialization will give domain_t proper implementation
        typedef domain_null_t domain_t;
    };
    ///
    /// @brief xyz -> Lab conversion
    ///
    template <>
    class color_conversion_t<lab_t, xyz_t> : public algo_t<lab_t, xyz_t>
    {
    public:
        typedef xyz_t domain_t;
        color_conversion_t<lab_t, xyz_t>(const xyz_t &white = xyz_t::D65()) : white_(white) {}
        virtual lab_t eval(const xyz_t &xyz) const override
        {
            lab_t result;
            const double thrs = 0.008856;

            double t = xyz.y / white_.y;
            if (t > thrs)
                result.L = 116 * pow(t, 1 / 3.0) - 16;
            else
                result.L = 903.3 * t;
            double ft = f(t);
            result.a = 500 * (f(xyz.x / white_.x) - ft);
            result.b = 200 * (ft - f(xyz.z / white_.z));
            return result;
        }
        virtual color_conversion_t<lab_t, xyz_t> *clone(void) const override
        {
            return new color_conversion_t<lab_t, xyz_t>(white_);
        }
    private:
        double f(double t) const
        {
            const double thrs = 0.008856;
            if (t > thrs)
                return pow(t, 1 / 3.0);
            else
                return 7.787 * t + 16.0 / 116;
        }
        xyz_t white_;
    };
    using xyz2lab_t = color_conversion_t<lab_t, xyz_t>;

    template <>
    class color_conversion_t<xyz_t, lab_t> : public algo_t<xyz_t, lab_t>
    {
    public:
        typedef lab_t domain_t;
        color_conversion_t<xyz_t, lab_t>(const xyz_t &white = xyz_t::D65()) : white_(white) {}
        virtual xyz_t eval(const lab_t &lab) const override
        {
            xyz_t result;
            double P = (lab.L + 16.0) / 116;

            result.x = white_.x * pow(P + lab.a / 500, 3);
            result.y = white_.y * pow(P, 3);
            result.z = white_.z * pow(P - lab.b / 200, 3);

            return result;
        }
        virtual color_conversion_t<xyz_t, lab_t> *clone(void) const override
        {
            return new color_conversion_t<xyz_t, lab_t>(white_);
        }
    private:
        double f(double t) const
        {
            const double thrs = 0.008856;
            if (t < thrs)
                return pow(t, 1 / 3.0);
            else
                return 7.787 * t + 16.0 / 116;
        }
        xyz_t white_;
    };
    using lab2xyz_t = color_conversion_t<xyz_t, lab_t>;

    /// @brief rgb_t<T> -> xyz_t conversion assuming D65 white point
    template <class S>
    class color_conversion_t<rgb_t<S>, xyz_t> : public algo_t<rgb_t<S>, xyz_t>
    {
    public:
        typedef xyz_t domain_t;
        virtual rgb_t<S> eval(const xyz_t &xyz) const override
        {
            rgb_t<S> result;

            result.red   = gamma( 3.240479 * xyz.x - 1.537150 * xyz.y - 0.498535 * xyz.z);
            result.green = gamma(-0.969256 * xyz.x + 1.875992 * xyz.y + 0.041556 * xyz.z);
            result.blue  = gamma( 0.055648 * xyz.x - 0.204043 * xyz.y + 1.057311 * xyz.z);
            return result;
        }
        virtual color_conversion_t<rgb_t<S>, xyz_t> *clone(void) const override
        {
            return new color_conversion_t<rgb_t<S>, xyz_t>;
        }
    private:
        static S gamma(double x)
        {
            double y = 0;
            if (x > 1.0)
                y = 1.0;
            else if (x > 0)
            {
                const double a = 0.055;
                if (x < 0.0031308)
                    y = 12.92 * x;
                else
                    y = (1 + a) * pow(x, 1.0 / 2.4) - a;
            }
            return static_cast<S>(scalar_traits_t<S>::cube_size() * y);
        }
    };
    using xyz2rgb_t = color_conversion_t<rgb_t<double>, xyz_t>;

    /// @brief xyz_t -> rgb_t<T> conversion assuming D65 white point
    template <class S>
    class color_conversion_t<xyz_t, rgb_t<S>> : public algo_t<xyz_t, rgb_t<S>>
    {
    public:
        typedef rgb_t<S> domain_t;
        virtual xyz_t eval(const rgb_t<S> &rgb) const override
        {
            xyz_t result;  

            rgb_t<double> rgblin;
            rgblin.red   = gamma(rgb.red);
            rgblin.green = gamma(rgb.green);
            rgblin.blue  = gamma(rgb.blue);
            result.x = 0.412453 * rgblin.red + 0.357580 * rgblin.green + 0.180423 * rgblin.blue;
            result.y = 0.212671 * rgblin.red + 0.715160 * rgblin.green + 0.072169 * rgblin.blue;
            result.z = 0.019334 * rgblin.red + 0.119193 * rgblin.green + 0.950227 * rgblin.blue;
            return result;
        }
        virtual color_conversion_t<xyz_t, rgb_t<S>> *clone(void) const override
        {
            return new color_conversion_t<xyz_t, rgb_t<S>>;
        }
    private:
        static double gamma(S s)
        {
            double y = 0.0;
            double x = static_cast<double>(s) / scalar_traits_t<S>::cube_size();
            if (x > 1)
                y = 1;
            else if (x > 0)
            {
                const double a = 0.055;
                if (x < 0.04045)
                    y = x / 12.92;
                else
                    y = pow((x + a) / (1 + a), 2.4);
            }
            return y;
        }
    };
    using rgb2xyz_t = color_conversion_t<xyz_t, rgb_t<double>>;

    
    /// @brief This conversion is more a data interpretation rule
    template <>
    class color_conversion_t<rgb_t<double>, vector_t<double, 3>> : public algo_t<rgb_t<double>, vector_t<double, 3>>
    {
    public:
        typedef vector_t<double, 3> domain_t;
        virtual rgb_t<double> eval(const vector_t<double, 3> &v) const override
        {
            rgb_t<double> result;

            result.red = v[0];
            result.green = v[1];
            result.blue = v[2];
            return result;
        }
        virtual color_conversion_t<rgb_t<double>, vector_t<double, 3>> *clone(void) const override
        {
            return new color_conversion_t<rgb_t<double>, vector_t<double, 3>>;
        }
    };

    /// @brief This conversion is more a data interpretation rule
    template <>
    class color_conversion_t<vector_t<double, 3>, rgb_t<double>> : public algo_t<vector_t<double, 3>, rgb_t<double>>
    {
    public:
        typedef rgb_t<double> domain_t;
        virtual vector_t<double, 3> eval(const rgb_t<double> &rgb) const override
        {
            vector_t<double, 3> result;

            result[0] = rgb.red ;
            result[1] = rgb.green;
            result[2] = rgb.blue ;
            return result;
        }
        virtual color_conversion_t<vector_t<double, 3>, rgb_t<double>> *clone(void) const override
        {
            return new color_conversion_t<vector_t<double, 3>, rgb_t<double>>;
        }
    };

    /// @brief Conversion between rgb typed based on different channel type
    /// Scaling to cube_size (255) is done for unsigned char channel
    ///
    template <class S, class T>
    class color_conversion_t<rgb_t<S>, rgb_t<T>> : public algo_t<rgb_t<S>, rgb_t<T>>
    {
    public:
        typedef rgb_t<T> domain_t;
        virtual rgb_t<S> eval(const rgb_t<T> &rgb) const override
        {
            rgb_t<S> result;

            result.red   = static_cast<S>(rgb.red   * scalar_traits_t<S>::cube_size() / scalar_traits_t<T>::cube_size()) ;
            result.green = static_cast<S>(rgb.green * scalar_traits_t<S>::cube_size() / scalar_traits_t<T>::cube_size()) ;
            result.blue  = static_cast<S>(rgb.blue  * scalar_traits_t<S>::cube_size() / scalar_traits_t<T>::cube_size()) ;
            return result;
        }
        virtual color_conversion_t<rgb_t<S>, rgb_t<T>> *clone(void) const override
        {
            return new color_conversion_t<rgb_t<S>, rgb_t<T>>;
        }
    };

    /// @brief Conversion between bgr typed based on different channel type
    /// Scaling to cube_size (255) is done for unsigned char channel
    ///
    template <class S, class T>
    class color_conversion_t<bgr_t<S>, bgr_t<T>> : public algo_t<bgr_t<S>, bgr_t<T>>
    {
    public:
        typedef bgr_t<T> domain_t;
        virtual bgr_t<S> eval(const bgr_t<T> &bgr) const override
        {
            bgr_t<S> result;

            result.red   = static_cast<S>(bgr.red   * scalar_traits_t<S>::cube_size() / scalar_traits_t<T>::cube_size());
            result.green = static_cast<S>(bgr.green * scalar_traits_t<S>::cube_size() / scalar_traits_t<T>::cube_size());
            result.blue  = static_cast<S>(bgr.blue  * scalar_traits_t<S>::cube_size() / scalar_traits_t<T>::cube_size());
            return result;
        }
        virtual color_conversion_t<bgr_t<S>, bgr_t<T>> *clone(void) const override
        {
            return new color_conversion_t<bgr_t<S>, bgr_t<T>>;
        }
    };

    /// @brief Conversion between rgb and bgr types, typed based on different channel type
    /// Scaling to cube_size (255) is done for unsigned char channel
    ///
    template <class S, class T>
    class color_conversion_t<rgb_t<S>, bgr_t<T>> : public algo_t<rgb_t<S>, bgr_t<T>>
    {
    public:
        typedef bgr_t<T> domain_t;
        virtual rgb_t<S> eval(const bgr_t<T> &rgb) const override
        {
            rgb_t<S> result;

            result.red   = static_cast<S>(rgb.red   * scalar_traits_t<S>::cube_size / scalar_traits_t<T>::cube_size);
            result.green = static_cast<S>(rgb.green * scalar_traits_t<S>::cube_size / scalar_traits_t<T>::cube_size);
            result.blue  = static_cast<S>(rgb.blue  * scalar_traits_t<S>::cube_size / scalar_traits_t<T>::cube_size);
            return result;
        }
        virtual color_conversion_t<rgb_t<S>, bgr_t<T>> *clone(void) const override
        {
            return new color_conversion_t<rgb_t<S>, bgr_t<T>>;
        }
    };

    template <class S, class T>
    class color_conversion_t<bgr_t<S>, rgb_t<T>> : public algo_t<bgr_t<S>, rgb_t<T>>
    {
    public:
        typedef rgb_t<T> domain_t;
        virtual bgr_t<S> eval(const rgb_t<T> &rgb) const override
        {
            bgr_t<S> result;

            result.red   = static_cast<S>(rgb.red   * scalar_traits_t<S>::cube_size / scalar_traits_t<T>::cube_size);
            result.green = static_cast<S>(rgb.green * scalar_traits_t<S>::cube_size / scalar_traits_t<T>::cube_size);
            result.blue  = static_cast<S>(rgb.blue  * scalar_traits_t<S>::cube_size / scalar_traits_t<T>::cube_size);
            return result;
        }
        virtual color_conversion_t<bgr_t<S>, rgb_t<T>> *clone(void) const override
        {
            return new color_conversion_t<bgr_t<S>, rgb_t<T>>;
        }
    };

    template <>
    class color_conversion_t<xyz_t, vector_t<double, 3>> : public algo_t<xyz_t, vector_t<double, 3>>
    {
    public:
        typedef vector_t<double, 3> domain_t;
        virtual xyz_t eval(const vector_t<double, 3> &v) const override
        {
            xyz_t result;

            result.x = v[0];
            result.y = v[1];
            result.z = v[2];
            return result;
        }
        virtual color_conversion_t<xyz_t, vector_t<double, 3>> *clone(void) const override
        {
            return new color_conversion_t<xyz_t, vector_t<double, 3>>;
        }
    };

    /// @brief This conversion is more a data interpretation rule
    template <>
    class color_conversion_t<vector_t<double, 3>, xyz_t> : public algo_t<vector_t<double, 3>, xyz_t>
    {
    public:
        typedef xyz_t domain_t;
        virtual vector_t<double, 3> eval(const xyz_t &xyz) const override
        {
            vector_t<double, 3> result;

            result[0] = xyz.x;
            result[1] = xyz.y;
            result[2] = xyz.z;
            return result;
        }
        virtual color_conversion_t<vector_t<double, 3>, xyz_t> *clone(void) const override
        {
            return new color_conversion_t<vector_t<double, 3>, xyz_t>;
        }
    };

    /// @brief This conversion is an encoding rule
    template <>
    class color_conversion_t<lab_t, vector_t<double, 3>> : public algo_t<lab_t, vector_t<double, 3>>
    {
    public:
        typedef vector_t<double, 3> domain_t;
        virtual lab_t eval(const vector_t<double, 3> &v) const override
        {
            lab_t result;

            result.L = v[0] * 100;
            result.a = v[1] * 255 - 128;
            result.b = v[2] * 255 - 128;
            return result;
        }
        virtual color_conversion_t<lab_t, vector_t<double, 3>> *clone(void) const override
        {
            return new color_conversion_t<lab_t, vector_t<double, 3>>;
        }
    };

    template <>
    class color_conversion_t<vector_t<double, 3>, lab_t> : public algo_t<vector_t<double, 3>, lab_t>
    {
    public:
        typedef lab_t domain_t;
        virtual vector_t<double, 3> eval(const lab_t &lab) const override
        {
            vector_t<double, 3> result;

            result[0] = lab.L / 100;
            result[1] = (lab.a + 128.0) / 255.0;
            result[2] = (lab.b + 128.0) / 255.0;
            return result;
        }
        virtual color_conversion_t<vector_t<double, 3>, lab_t> *clone(void) const override
        {
            return new color_conversion_t<vector_t<double, 3>, lab_t>;
        }
    };

} // namespace iccpp

#endif
