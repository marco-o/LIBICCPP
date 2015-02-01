#ifndef iccpp_profile_H
#define iccpp_profile_H
//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <fstream>
#include <memory>
#include <exception>

#include "iccpp_function.h"
#include "iccpp_color_spaces.h"
#include "iccpp_converters.h"
#include "iccpp_tags.h"

/** 
    @mainpage

    LibICC++ is a library that aims to load and apply ICC profiles using modern C++

    So instread of defining opaque handlers to profiles and transforms uses a 
    template class function_t<Y, X> that implements a class that maps an object of type X onto Y.
    X and Y rapresents a color. Typical implementations are the Lab_t or xyz_t struct and rgb_t template.
    The main feature of his class, after evaluation, is composition. Two functions functon_t<Y, X> and 
    function_t<X, Z> can be composed into a function_t<Y, Z>.

    ICC profile loading gives a function_t from the PCS color space (xyz_t, or Lab typically) 
    to device color space (rgb_t). A tranform can then be obtained by combining 
    an input profile (function_t from rgb_t to xyz_t) to an output profile. The profile class can still be queried
    to obtain the inout and output color space, but the actal color space is encoded into result type.
*/

namespace iccpp
{
	///
	/// @class icc_file_exception_t
	/// @brief Exception thrown in case of any problem encountered while loading a profile
	///
	class icc_file_exception_t : public std::runtime_error
	{ 
	public:
		icc_file_exception_t(const char *msg) : std::runtime_error(msg) {}
	};

	///
	/// @class profile_t
	/// @brief This is the class that defines the interface for profile loading
	///
	class profile_t
	{
	public:
		static profile_t *create_sRGB(void); ///< Construction of a standard case
		static profile_t *create(const char *); ///< Constructor from file path
		static profile_t *create(std::unique_ptr<std::ifstream> &); ///< Constructor from file path
		virtual ~profile_t(void) {}
		virtual color_space_t pcs(void) const = 0;
		virtual color_space_t device(void) const = 0;
		virtual void list_tags(std::ostream &) const {}
		virtual void load_all(void) {}
		virtual void load_tag(tag_signature_t) {}
        template <class Y, class X>
        static function_t<Y, X> extract_casted(algo_base_t *algo)
        {
            algo_t<Y, X> *result = dynamic_cast<algo_t<Y, X> *>(algo);
            if (!result) // non null pointers will be taken over by the called
                delete algo;
            return function_t<Y, X>(result);
        }
		template <class Y, class X>
        function_t<Y, X> pcs2device(rendering_intent_t intent = rendering_intent_t::relative_colorimetric)
		{
			algo_base_t *algo = pcs2dev(intent);
            if (algo)
            {
                std::unique_ptr<algo_base_t> domain_adapted(adapt_domain<X>(algo)); // domain is known
                if (domain_adapted)
                    return extract_casted<Y, X>(adapt_range2<Y, X>(domain_adapted.get()));
            }
            return function_t<Y, X>(dynamic_cast<algo_t<Y, X> *>(algo));
        }
		template <class Y, class X>
        function_t<Y, X> device2pcs(rendering_intent_t intent = rendering_intent_t::relative_colorimetric)
		{
            algo_base_t *algo = dev2pcs(intent);
            if (algo)
            {
                std::unique_ptr<algo_base_t> range_adapted(adapt_range<Y>(algo)); // domain is handled, since is a fairly standard space
                if (range_adapted)
                    return extract_casted<Y, X>(adapt_domain2<Y, X>(range_adapted.get()));
            }
            return function_t<Y, X>(dynamic_cast<algo_t<Y, X> *>(algo));
        }
	protected:
        virtual algo_base_t *dev2pcs(rendering_intent_t) = 0; ///< helps building PCS to device transform
        virtual algo_base_t *pcs2dev(rendering_intent_t) = 0;
	private:
	};
}
#endif