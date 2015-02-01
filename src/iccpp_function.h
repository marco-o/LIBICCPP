#ifndef iccpp_function_H
#define iccpp_function_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "iccpp_config.h"

#include <memory>
namespace iccpp
{

    class visitor_base_t
    {
    public:
        virtual ~visitor_base_t(void){}
    };

    template <class X> class algo_domain_t;
    template <class Y> class algo_range_t;

    class algo_base_t
    {
    public:
        virtual ~algo_base_t(void) {}
        virtual void accept_domain(visitor_base_t &) = 0;
        virtual void accept_range(visitor_base_t &) {}
    };

    template <class X>
    class algo_domain_t : virtual public algo_base_t
    {
    public:
    };

    template <class X>
    class visitor_domain_t : virtual public visitor_base_t
    {
    public:
        virtual void visit_domain(algo_domain_t<X> &) {}
    };

    template <class Y>
    class algo_range_t : virtual public algo_base_t
    {
    public:
    };

    template <class Y>
    class visitor_range_t : virtual public visitor_base_t
    {
    public:
        virtual void visit_range(algo_range_t<Y> &) {}
    };

    /**
       @class algo_t
       @brief Base class for algorithms.
       This class implements a generic functional algorithm that maps values of type X
       onto values of type Y. This class is used as letter in a letter-envelope pattern,
       where the function_t template playes the role of th nvelope
    */
    template <class Y, class X>
    class algo_t : virtual public algo_domain_t<X>,
                   virtual public algo_range_t<Y>
    {
    public:
        typedef X domain_type;
        typedef Y range_type;
        // 
        // virtual destructr (this is a base class)
        //
        virtual ~algo_t(void) {}
        ///
        /// @brief Cloning function, required to implement copy and composition
        ///
        virtual algo_t<Y, X> *clone(void) const = 0;
        ///
        /// @brief Interface of the evaluation function
        ///
        virtual Y eval(const X &x) const = 0;
        ///
        /// @brief Visitor-related functions allows type inspection
        ///
        virtual void accept_domain(visitor_base_t &visitor) override
        {
            visitor_domain_t<X> *vin = dynamic_cast<visitor_domain_t<X> *>(&visitor);
            if (vin)
                vin->visit_domain(*this);
        }
        virtual void accept_range(visitor_base_t &visitor) override
        {
            visitor_range_t<Y> *vout = dynamic_cast<visitor_range_t<Y> *>(&visitor);
            if (vout)
                vout->visit_range(*this);
        }
    };

    ///
    /// @class function_t
    /// @brief This is a envelope class used to represent functions. 
    /// It holds a pointer to an algorithm and implements two operations: 
    /// evaluation and compostion.
    ///
    template <class Y, class X>
    class function_t
    {
    public:
        typedef X domain_type ;
        typedef Y range_type;

        ///
        /// @class composite_t
        /// @brief Implements the composition of f1 : X->Z and f2 : Z->Y as an f: X ->Y
        ///
        template <class Z>
        class composite_t : public algo_t<Y, Z>
        {
        public:
            composite_t(algo_t<Y, X> *lhs,
                        algo_t<X, Z> *rhs) : lhs_(lhs), rhs_(rhs) {}
            ///
            /// @brief Evaluation of functional composition
            ///
            virtual Y eval(const Z &z) const override
            {
                X x = rhs_->eval(z);
                return lhs_->eval(x);
            }
            virtual composite_t<Z> *clone(void) const override
            {
                return new composite_t<Z>(lhs_->clone(), rhs_->clone());
            }
        private:
            std::unique_ptr<algo_t<Y, X>> lhs_;
            std::unique_ptr<algo_t<X, Z>> rhs_;
        };
        ///
        /// @brief Construction from an algorithm
        ///
        function_t(algo_t<Y, X> *algo) : imp_(algo) {}
        ///
        /// @brief Copy construction. Clones the algorithm
        ///
        function_t(const function_t<Y, X> &rhs) : imp_(rhs.clone()) {}
        operator bool() { return imp_.get() != nullptr; }
        //
        // Something to rearrange better...
        //
        algo_t<Y, X> *get(void)     { return imp_.get(); }
        algo_t<Y, X> *release(void) { return imp_.release(); }
        //
        // Operations
        //
        range_type eval(const domain_type &x) const ///< Function evaluation
        {
            return imp_->eval(x);
        }
        range_type operator()(const domain_type &x) const
        {
            return eval(x);
        }
        algo_t<Y, X> *clone(void) const
        { 
            return imp_->clone(); 
        }
        template <class Z>
        function_t<Y, Z> operator*(const function_t<X, Z> &rhs) const ///< Function composition operation
        {
            return function_t<Y, Z>(new composite_t<Z>(imp_->clone(), rhs.clone()));
        }
    private:
        std::unique_ptr<algo_t<Y, X>> imp_;
    };

}
#endif
