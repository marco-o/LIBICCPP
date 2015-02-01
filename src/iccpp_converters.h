#ifndef icpp_converters_H
#define icpp_converters_H
//----------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "iccpp_typelist.h"
#include "iccpp_color_spaces.h"

/**
  @file
  @brief This file contanis some metaprogramming that inserts color space adapters on functions
 */

namespace iccpp
{
    /**
       @brief Adapts the domain of an algo_t<Y, X> by inserting a color_conversion_t<X, Z> 
              if required (i.e. X and Z are different color spaces)
              If there is no such conversion nullptr is returned.
              Nothing is added if X and Z are the same type 
              (there is a template specializaion for that case)
     */
    template <class Y, class X, class Z>
    struct domain_converter_t
    {
        static algo_t<Y, Z> *adapt(Z *, const algo_t<Y, X> *f1)
        {
            return new typename function_t<Y, X>::template composite_t<Z>(f1->clone(), new color_conversion_t<X, Z>);
        }
        static algo_t<Y, Z> *adapt(domain_null_t *, const algo_t<Y, X> *)
        {
            return nullptr;
        }
    };

    template <class Y, class X>
    struct domain_converter_t<Y, X, X>
    {
        static algo_t<Y, X> *adapt(X *, const algo_t<Y, X> *f)
        {
            return f->clone();
        }
        static algo_t<Y, X> *adapt(domain_null_t *, const algo_t<Y, X> *f)
        {
            return f->clone();
        }
    };

    /**
    @brief Adapts the range of an algo_t<X, Z> by inserting a color_conversion_t<Y, X>
            if required (i.e. X and Y are different color spaces)
            If there is no such conversion nullptr is returned.
    */
    template <class Y, class X, class Z>
    struct range_converter_t
    {
        static algo_t<Y, Z> *adapt(X *, const algo_t<X, Z> *f1)
        {
            return new typename function_t<Y, X>::template composite_t<Z>(new color_conversion_t<Y, X>, f1->clone());
        }
        static algo_t<Y, Z> *adapt(domain_null_t *, const algo_t<X, Z> *)
        {
            return nullptr;
        }
    };


    template <class Y, class X>
    struct range_converter_t<Y, Y, X>
    {
        static algo_t<Y, X> *adapt(Y *, const algo_t<Y, X> *f)
        {
            return f->clone();
        }
        static algo_t<Y, X> *adapt(domain_null_t *, const algo_t<Y, X> *f)
        {
            return f->clone();
        }
    };

    template <class X, class Z>
    class visitor_range_inspector_kernel_t
    {
    public:
        visitor_range_inspector_kernel_t(void) : result_(nullptr){}
        algo_base_t *result(void) { return result_; }
        template <class R>
        void range(algo_range_t<R> &algor)
        {
            algo_t<R, X> *algo = dynamic_cast<algo_t<R, X> *>(&algor);
            if (algo != 0)
                result_ = domain_converter_t<R, X, Z>::adapt(
                reinterpret_cast<typename color_conversion_t<X, Z>::domain_t *>(0), algo);
        }
    private:
        algo_base_t *result_;
    };
    template <class X, class Z>
    using visitor_range_inspector_tl_t = visitor_range_chain_t<color_space_list_t, visitor_range_inspector_kernel_t<X, Z>>;

    template <class Z>
    class visitor_domain_first_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_domain_first_kernel_t(void) : result_(0) {}
        algo_base_t *result(void) { return result_; }
        template <class D>
        void domain(algo_domain_t<D> &algo)
        {
            typename visitor_range_inspector_tl_t<D, Z>::type range;
            algo.accept_range(range);
            result_ = range.result();
        }
        algo_base_t *result_;
    };
    template <class Z>
    using visitor_domain_first_tl_t = visitor_domain_chain_t<color_space_list_t, visitor_domain_first_kernel_t<Z>>;

    // given a generic algo_t<Y, X> tries to add a conversion Z->X
    // to build a new algo_t<Y, Z>
    template <class Z>
    algo_base_t *adapt_domain(algo_base_t *algo)
    {
        typename visitor_domain_first_tl_t<Z>::type domain;
        algo->accept_domain(domain);
        return domain.result();
    }
    //
    // This section deals with range adaptaion
    // 
    template <class T, class R>
    class visitor_domain_inspector_kernel_t
    {
    public:
        visitor_domain_inspector_kernel_t(void) : result_(nullptr){}
        algo_base_t *result(void) { return result_; }
        template <class D>
        void domain(algo_domain_t<D> &algor)
        {
            algo_t<R, D> *algo = dynamic_cast<algo_t<R, D> *>(&algor);
            if (algo != 0)
                result_ = range_converter_t<T, R, D>::adapt(
                reinterpret_cast<typename color_conversion_t<T, R>::domain_t *>(0), algo);
        }
    private:
        algo_base_t *result_;
    };
    template <class T, class R>
    using visitor_domain_inspector_tl_t = visitor_domain_chain_t<color_space_list_t, visitor_domain_inspector_kernel_t<T, R>>;

    template <class T>
    class visitor_range_first_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_range_first_kernel_t(void) : result_(0) {}
        algo_base_t *result(void) { return result_; }
        template <class R>
        void range(algo_range_t<R> &algo)
        {
            typename visitor_domain_inspector_tl_t<T, R>::type domain;
            algo.accept_domain(domain);
            result_ = domain.result();
        }
        algo_base_t *result_;
    };
    template <class T>
    using visitor_range_first_tl_t = visitor_range_chain_t<color_space_list_t, visitor_range_first_kernel_t<T>>;

    template <class T>
    algo_base_t *adapt_range(algo_base_t *algo)
    {
        typename visitor_range_first_tl_t<T>::type range;
        algo->accept_range(range);
        return range.result();
    }

    template <class Y, class Z>
    class visitor_domain_only_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_domain_only_kernel_t(void) : result_(0) {}
        algo_base_t *result(void) { return result_; }
        template <class D>
        void domain(algo_domain_t<D> &algod)
        {
            algo_t<Y, D> *algo = dynamic_cast<algo_t<Y, D> *>(&algod);
            if (algo != 0)
                result_ = domain_converter_t<Y, D, Z>::adapt(
                reinterpret_cast<typename color_conversion_t<D, Z>::domain_t *>(0), algo);
        }
        algo_base_t *result_;
    };
    template <class Y, class Z>
    using visitor_domain_only_tl_t = visitor_domain_chain_t<color_space_list_t, visitor_domain_only_kernel_t<Y, Z>>;

    // this functon does the domain adaptation once the range is known
    // This works also when Y is not a type handled by the visitor
    template <class Y, class Z>
    algo_base_t *adapt_domain2(algo_base_t *algo)
    {
        typename visitor_domain_only_tl_t<Y, Z>::type domain;
        algo->accept_domain(domain);
        return domain.result();
    }

    template <class Y, class X>
    class visitor_range_only_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_range_only_kernel_t(void) : result_(0) {}
        algo_base_t *result(void) { return result_; }
        template <class R>
        void range(algo_range_t<R> &algod)
        {
            algo_t<R, X> *algo = dynamic_cast<algo_t<R, X> *>(&algod);
            if (algo != 0)
                result_ = range_converter_t<Y, R, X>::adapt(
                reinterpret_cast<typename color_conversion_t<Y, R>::domain_t *>(0), algo);
        }
        algo_base_t *result_;
    };
    template <class Y, class Z>
    using visitor_range_only_tl_t = visitor_range_chain_t<color_space_list_t, visitor_range_only_kernel_t<Y, Z>>;

    // this functon does the range adaptation once the domain is known
    // This works also when Y is not a type handled by the visitor
    template <class Y, class Z>
    algo_base_t *adapt_range2(algo_base_t *algo)
    {
        typename visitor_range_only_tl_t<Y, Z>::type range;
        algo->accept_range(range);
        return range.result();
    }

    template <class InputRange>
    class visitor_output_domain_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_output_domain_kernel_t(void) : input_(nullptr), result_(nullptr) {}
        algo_base_t *result(void) { return result_; }
        void input(algo_range_t<InputRange> *algo) { input_ = algo; }
        template <class D>
        void domain(algo_domain_t<D> &)
        {
            // task is adapt input to output domain D
            typename visitor_domain_inspector_tl_t<D, InputRange>::type domain;
            input_->accept_domain(domain);
            result_ = domain.result();
        }
    private:
        algo_range_t<InputRange> *input_;
        algo_base_t *result_;
    };

    class visitor_input_range_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_input_range_kernel_t(void) : output_(0), result_(0) {}
        algo_base_t *result(void) { return result_; }
        void output(algo_base_t *algo) { output_ = algo; }
        template <class R>
        void range(algo_range_t<R> &algod)
        {
            typename visitor_domain_chain_t<color_space_list_t,
                                            visitor_output_domain_kernel_t<R >> ::type output_domain;
            output_domain.input(&algod);
            output_->accept_domain(output_domain);
            result_ = output_domain.result();
        }
        algo_base_t *output_;
        algo_base_t *result_;
    };

    template <class R, class D>
    class visitor_joiner_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_joiner_kernel_t(void) : input_(nullptr), result_(nullptr) {}
        algo_base_t *result(void) { return result_; }
        void input(algo_t<R, D> *input) { input_ = input; }
        template <class Y>
        void range(algo_range_t<Y> &algod)
        {
            algo_t<Y, R> *output = dynamic_cast<algo_t<Y, R> *>(&algod);
            if (output != 0)
                result_ = new typename function_t<Y, R>::template composite_t<D>(output->clone(), input_);
        }
    private:
        algo_t<R, D> *input_;
        algo_base_t  *result_;

    };

    template <class R>
    class visitor_input_domain1_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_input_domain1_kernel_t(void) : output_(0), result_(0) {}
        algo_base_t *result(void) { return result_; }
        void output(algo_base_t *algo) { output_ = algo; }
        template <class D>
        void domain(algo_domain_t<D> &algod)
        {
            algo_t<R, D> *input = dynamic_cast<algo_t<R, D> *>(&algod);
            if (input != nullptr)
            {
                typename visitor_range_chain_t<color_space_list_t,
                                               visitor_joiner_kernel_t<R, D >> ::type output_range;
                output_range.input(input);
                output_->accept_range(output_range);
                result_ = output_range.result();
            }
        }
        algo_base_t *output_;
        algo_base_t *result_;
    };

    // get input domain for another task...
    class visitor_input_range1_kernel_t : virtual public visitor_base_t
    {
    public:
        visitor_input_range1_kernel_t(void) : output_(0), result_(0) {}
        algo_base_t *result(void) { return result_; }
        void output(algo_base_t *algo) { output_ = algo; }
        template <class R>
        void range(algo_range_t<R> &algod)
        {
            typename visitor_domain_chain_t<color_space_list_t,
                                            visitor_input_domain1_kernel_t<R >> ::type input_domain;
            input_domain.output(output_);
            algod.accept_domain(input_domain);
            result_ = input_domain.result();
        }
        algo_base_t *output_;
        algo_base_t *result_;
    };

    class transform_t
    {
    public:
        template <class Y, class X>
        static algo_t<Y, X> *create(algo_base_t *output, algo_base_t *input)
        {
            algo_t<Y, X> *result = nullptr;
            std::unique_ptr<algo_base_t> algo(create_joined(output, input));
            if (algo)
            {
                std::unique_ptr<algo_base_t> domain_adapted(adapt_domain<X>(algo.get())); // domain is known
                if (domain_adapted)
                {
                    std::unique_ptr<algo_base_t> adapted(adapt_range2<Y, X>(domain_adapted.get()));
                    result = dynamic_cast<algo_t<Y, X> *>(adapted.get());
                    if (result)
                        adapted.release();
                }
            }
            return result;
        }
    private:
        static algo_base_t *create_joined(algo_base_t *output, algo_base_t *input)
        {
            visitor_range_chain_t<color_space_list_t,
                visitor_input_range_kernel_t>::type input_range;
            input_range.output(output);
            input->accept_range(input_range);
            algo_base_t *input1 = input_range.result();
            visitor_range_chain_t<color_space_list_t,
                visitor_input_range1_kernel_t>::type joiner;
            joiner.output(output);
            input1->accept_range(joiner);
            return joiner.result();
        }
    };


}

#endif
