#ifndef iccpp_cut_H
#define iccpp_cut_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "iccpp_pixel.h"
#include "iccpp_traits.h"
#include <algorithm>
#include <string.h>

namespace iccpp
{
    template <class T>
    struct delta_item_t
    {
        T      value;
        size_t offset;
        operator T(void) const { return value; }
    };

    template <int N, bool>
    class permutation_t
    {
    public:
        template <class T>
        static void indexes(delta_item_t<T> *items) 
        {
            std::sort(items,
                      items + N,
                      [](const delta_item_t<T> &lhs, const delta_item_t<T> &rhs)
                      { return lhs.value > rhs.value; });
        }
    };

    // specialized class for low N 
    template <int N>
    class permutation_t<N, false>
    {
    public:
        permutation_t(void)
        {
            // no metaprogramming here, speed is not critical
            size_t perm[N];
            for (size_t i = 0; i < N; i++)
                perm[i] = i;
            memset(perm_, sizeof(perm_), 0);
            do
            {
                size_t *line = perm_[signature(perm)];
                delta_item_t<size_t> items[N];
                for (size_t i = 0; i < N; i++)
                {
                    items[i].value = perm[i];
                    items[i].offset = i;
                }
                permutation_t<N, true>::indexes(items);
                for (size_t i = 0; i < N; i++)
                    line[i] = items[i].offset;
            } while (std::next_permutation(perm, perm + N));
        }
        template <class T>
        void indexes(delta_item_t<T> *items) const
        {
            delta_item_t<T> temp[N];

            std::copy(items, items + N, temp);
            const size_t *perm = perm_[signature(items)];
            permute(items, temp, perm);
        }
        // makes all comparisons (data[i], data[j]), i < j and set a bit for every true result
        // a monotonically decreasing data maps to 0, increasing ones to 2^N-1
        template <class T>
        static void permute(delta_item_t<T> *dest, delta_item_t<T> *src, const size_t *perm)
        {
            dest[0] = src[perm[0]];
            permutation_t<N - 1, false>::permute(dest + 1, src, perm + 1);
        }
        template <class T>
        static size_t signature(const T *data)
        {
            return linear(data[0], data + 1) + (permutation_t<N - 1, false>::signature(data + 1) << (N - 1));
        }
        template <class T>
        static size_t linear(T ref, const T *data)
        {
            return (ref > data[0] ? 0 : 1) + (permutation_t<N - 1, false>::linear(ref, data + 1) << 1);
        }
    private:
        // size of this array is given by the number of comparisons
        // this scheme assumes the comparison of all pairs
        // for n = 5 means 10 bits, 1024 entries in the table
        // for greater values of N precomputing is no longer worth the game...
        size_t perm_[1 << ((N-1)*N/2)][N];
    };
    template <>
    struct permutation_t<1, false>
    {
        template <class T>
        static size_t linear(T, const T *) { return 0; }
        template <class T>
        static size_t signature(const T *)    { return 0; }
        template <class T>
        static void permute(delta_item_t<T> *dest, delta_item_t<T> *src, const size_t *perm)
        {
            dest[0] = src[perm[0]];
        }
        template <class T>
        void indexes(delta_item_t<T> *) const {  }
    };
    //
    // this helper is required to keep vector_t out of lut_t
    // the names of the functions refer to case Dim = 3
    //
    template <class X>
    struct lut_input_traits_t
    {
        typedef X scalar_type;
    };


    template <class T, int Dim>
    struct iteration_t
    {
        template <class X>
        static size_t substitute(delta_item_t<X> *deltas, const size_t *offset)
        {
            size_t result = iteration_t<T, Dim - 1>::substitute(deltas + 1, offset + 1) + offset[0] * deltas->offset;
            deltas->offset = offset[0];
            return result;
        }
        template <class X>
        static void split(const T *src, const T *step, delta_item_t<X> *deltas, const size_t *size1)
        {
            T div = src[0] / step[0];
            deltas->offset = static_cast<size_t>(div);
            // when this happens we are at the extreme end of a hypercube, ending
            // up in a space not initialized. In this case the code adds an elements 
            // with weight 0 but pointing to an unintitalized memory
            // This is he 'teoric' code, below the one that avoids the if
            // if (deltas->offset >= size1[0])
            //     deltas->offset = size1[0] - 1;
            deltas->offset -= deltas->offset / size1[0];
            deltas->value = src[0] * scalar_traits_t<T>::one() / step[0] - deltas->offset * scalar_traits_t<T>::one();
            iteration_t<T, Dim - 1>::split(src + 1, step + 1, deltas + 1, size1 + 1);
        }
        static void build(const T *dx, const size_t *steps, T *result)
        {
            result[0] = static_cast<T>(dx[0] * steps[0]);
            iteration_t<T, Dim - 1>::build(dx + 1, steps + 1, result + 1);
        }
    };

    template <class T>
    struct iteration_t<T, 0>
    {
        template <class X>
        static size_t substitute(delta_item_t<X> *, const size_t *)
        {
            return 0;
        }
        template <class X>
        static void split(const T *, const T *, delta_item_t<X> *, const size_t *)
        {
        };
        static void build(const T *, const size_t *, T *)
        {
        };
    };

    template <class Y, class S, int N>
    struct voxel_sum_t
    {
        static auto tetrahedral(const Y *cube, delta_item_t<S> *deltas) -> decltype(cube[0] * 1)
        {
            S weight = deltas->value;
            cube += deltas->offset;
            deltas++;
            return cube[0] * (weight - deltas->value) + voxel_sum_t<Y, S, N - 1>::tetrahedral(cube, deltas);
        }
        static auto multilinear(const Y *cube, delta_item_t<S> *deltas) -> decltype(cube[0] * 1)
        {
            return voxel_sum_t<Y, S, N - 1>::multilinear(cube, deltas + 1) * (scalar_traits_t<S>::one() - deltas->value) +
                   voxel_sum_t<Y, S, N - 1>::multilinear(cube + deltas->offset, deltas + 1) * deltas->value;
        }
    };

    template <class Y, class S>
    struct voxel_sum_t<Y, S, 1>
    {
        static auto tetrahedral(const Y *cube, delta_item_t<S> *deltas) -> decltype(cube[0] * 1)
        {
            return cube[deltas->offset] * deltas->value;
        }
        static auto multilinear(const Y *cube, delta_item_t<S> *deltas) -> decltype(cube[0] * 1)
        {
            return cube[0] * (scalar_traits_t<S>::one() - deltas->value) +
                   cube[deltas->offset] * deltas->value;
        }
    };

    struct interp_tetra_t {};
    struct interp_multi_t {};

    template <class Y, int N>
    class arrayn_t
    {
    public:
        arrayn_t(const size_t *size)
        {
            arithmetic_t<size_t, N>::assign(size_, size);
            offset_[0] = 1;
            for (size_t i = 0; i < N; i++)
            {
                size1_[i] = size[i] - 1;
                offset_[i + 1] = offset_[i] * size_[i];
            }
            data_ = new Y[offset_[N]];
        }
        ~arrayn_t(void)
        {
            delete[] data_;
        }
        void copy_table(const arrayn_t<Y, N> &rhs)
        {
            if (offset_[N] == rhs.offset_[N])
                memcpy(data_, rhs.data_, offset_[N] * sizeof(Y));
        }
        //
        // Accessors
        //
        size_t datasize(void) const { return offset_[N]; }
        const size_t *size(void) const { return size_; }
        const size_t *size1(void) const { return size1_; }
        const size_t *offset(void) const { return offset_; }
        template <class S> // S is a scalar
        Y combine(delta_item_t<S> *deltas, interp_tetra_t) const
        {
            // computes offset of first cube
            // and substitutes the indexes with offset for each dimension
            const Y *data = data_ + iteration_t<S, N>::substitute(deltas, offset_);
            // sort deltas to have them in decreasing order
            perm_.indexes(deltas);
            // for byte type one() is 256, hence the trait
            return data[0] * (scalar_traits_t<S>::one() - deltas->value) +
                               voxel_sum_t<Y, S, N>::tetrahedral(data, deltas);
        }
        template <class S>
        Y combine(delta_item_t<S> *deltas, interp_multi_t) const
        {
            // computes offset of first cube
            // and substitutes the indexes with offset for each dimension
            const Y *data = data_ + iteration_t<S, N>::substitute(deltas, offset_);
            return voxel_sum_t<Y, S, N>::multilinear(data, deltas);
        }

        // that's a bit silly but effective....
        Y &operator[](int index) { return data_[index]; }
    private:
        size_t                    size_[N];
        size_t                    size1_[N]; // == size-1, useful precomputed value
        size_t                    offset_[N + 1];
        Y                        *data_;
        permutation_t<N, (N > 5)> perm_;
    };

    template <class Y, class X, class I = interp_tetra_t>
    class lut_t : public algo_t<Y, X>
    {
    public:
        enum {dimension = X::dimension};
        using input_traits_t = lut_input_traits_t<X>;
        using scalar_type = typename input_traits_t::scalar_type;
        using weight_type = typename input_traits_t::weight_type;
        lut_t(const size_t *size, const X &step) : data_(size), step_(step) {}
        // that's a bit silly but effective to fill in the data
        size_t datasize(void) const { return data_.datasize(); }
        Y &operator[](int index) { return data_[index]; }
        virtual Y eval(const X &x) const override
        {
            delta_item_t<weight_type> deltas[dimension];
            // find the cube x belongs (and the remainders)
            input_traits_t::split(x, step_, deltas, data_.size1());
            // build the interpolation
            return data_.combine(deltas, I());
        }
        lut_t<Y, X, I> *clone(void) const override
        {
            lut_t<Y, X, I> *result = new lut_t<Y, X, I>(data_.size(), step_);
            result->data_.copy_table(data_);
            return result;
        }
     private:
        arrayn_t<Y, dimension>  data_; // TODO: consider sharing this data...
        X                       step_;
    };

}
#endif
