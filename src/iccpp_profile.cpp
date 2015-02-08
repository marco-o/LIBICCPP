//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "iccpp_profile.h"
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "iccpp_function.h"

//#define DEBUG_ALGO

#ifdef DEBUG_ALGO
#include <iostream>
#include "iccpp_streamio.h"
#define LOG_PIXEL(text, x) std::cout << text << "(" << x << ")" << std::endl ;
#else
#define LOG_PIXEL(text, x) 
#endif
#include "iccpp_clut.h"
#include "iccpp_curve.h"
#include "iccpp_rgb.h"
#include "iccpp_pixel_traits.h"

namespace iccpp
{
    namespace 
    {
        //
        // A metaprogramming helper
        //
        template <int Outputs, int Inputs>
        struct cond_prod_t
        {
            static function_vect_t<Outputs, Inputs> exec(const function_vect_t<Outputs, Inputs> &f, const function_vect_t<3, 3>)
            {
                return f;
            }
        };
        template <>
        struct cond_prod_t<3, 3>
        {
            static function_vect_t<3, 3> exec(const function_vect_t<3, 3> &f, const function_vect_t<3, 3> &m)
            {
                return f * m;
            }
        };
    }
    namespace loader
    {
        struct tag_entry_t
        {
            tag_signature_t signature;
            icc_uint32_t    offset;
            icc_uint32_t    size;
        };

        class reader_t
        {
        public:
            reader_t(std::istream &ist, size_t offset = 0) : ist_(ist), offset_(offset), dsize_(1)
            {
                if (offset != 0)
                    ist.seekg(offset);
            }
            reader_t &operator=(const reader_t &) = delete;
            void   entry_size(size_t value) { dsize_ = value; }
            size_t entry_size(void) const { return dsize_; }
            reader_t operator + (int offset)
            {
                return reader_t(ist_, offset_ + offset);
            }
            size_t offset(void) const { return offset_; }
            template <class T>
            T read(void)
            {
                T result;
                (*this)(result);
                return result;
            }
            void zero(size_t bytes)
            {
                char data[32];
                if (bytes > 32)
                    bytes = 32;
                ist_.read(data, bytes);
                for (size_t i = 0; i < bytes; i++)
                if (data[i] != 0)
                    throw icc_file_exception_t("Nonzero padding");
            }
            double read_s15f16(void)
            {
                return read<icc_int32_t>() / 65536.0;
            }
            double read_u16f16(void)
            {
                return read<icc_uint32_t>() / 65536.0;
            }
            double read_u1f15(void)
            {
                return read<icc_uint16_t>() / 32768.0;
            }
            double read_u0f16(void)
            {
                return read<icc_uint16_t>() / 65536.0;
            }
            double read_u8f8(void)
            {
                return read<icc_uint16_t>() / 256.0;
            }
            template <class T, int N>
            void operator()(vector_t<T, N> &value)
            {
                for (int i = 0; i < N; i++)
                    value[i] = read<T>();
            }
            void operator()(icc_uint8_t &value)
            {
                value = integer<icc_uint8_t>();
            }
            void operator()(icc_uint16_t &value)
            {
                value = integer<icc_uint16_t>();
            }
            void operator()(icc_int32_t &value)
            {
                value = integer<icc_uint32_t>();
            }
            void operator()(icc_uint32_t &value)
            {
                value = integer<icc_uint32_t>();
            }
            void operator()(icc_uint64_t &value)
            {
                value = integer<icc_uint64_t>();
            }
            void operator()(double &value)
            {
                switch (dsize_)
                {
                case 1:
                    value = read<icc_uint8_t>() / 255.0;
                    break;
                case 2:
                    value = read<icc_uint16_t>() / 65535.0;
                    break;
                default:
                    throw icc_file_exception_t("Invalid clut element size");
                }
            }            // that's a sort of back door for enums (otherwise cast is a problem...)
            template <class T>
            void cast(T &value)
            {
                value = static_cast<T>(integer<icc_uint32_t>());
            }
            template <int N>
            void operator()(unsigned char(&value)[N])
            {
                ist_.read(reinterpret_cast<char *>(&value[0]), N);
            }
            template <int N>
            void operator()(signed char(&value)[N])
            {
                ist_.read(reinterpret_cast<char *>(&value[0]), N);
            }
        private:
            template <class T>
            T integer(void)
            {
                unsigned char value[sizeof(T)];
                ist_.read(reinterpret_cast<char *>(value), sizeof(T));
                T result = 0;
                for (int i = 0; i < sizeof(T); i++)
                    result = (result << 8) + value[i];
                return result;
            }
            std::istream &ist_;
            size_t        offset_;
            size_t        dsize_; // normalized way of reading 
        };

        template <class S>
        void exx(S &serializer, encoded_xyz_t &xyz)
        {
            serializer(xyz.X);
            serializer(xyz.Y);
            serializer(xyz.Z);
        }

        template <class S>
        void exx(S &serializer, tag_entry_t &tag)
        {
            serializer.cast(tag.signature);
            serializer(tag.offset);
            serializer(tag.size);
        }

        template <class S>
        void exx(S &serializer, date_time_number_t &date_time)
        {
            serializer(date_time.year);
            serializer(date_time.month);
            serializer(date_time.day);
            serializer(date_time.hours);
            serializer(date_time.minutes);
            serializer(date_time.seconds);
        }

        template <class S>
        void exx(S &serializer, profile_header_t &header)
        {
            serializer(header.size);
            serializer(header.cmm_id);
            serializer(header.version);
            serializer.cast(header.device_class);
            serializer.cast(header.color_space);
            serializer.cast(header.pcs);
            exx(serializer, header.date);
            serializer(header.magic);
            serializer.cast(header.platform);
            serializer(header.flags);
            serializer(header.manufacturer);
            serializer(header.model);
            serializer(header.attributes);
            serializer.cast(header.rendering_intent);
            exx(serializer, header.illuminant);
            serializer(header.creator);
            serializer(header.profile_id);
            serializer(header.reserved);
        }

        class tag_content_base_t 
        {
        public:
            virtual ~tag_content_base_t(void) {}
        };
        typedef std::shared_ptr<tag_content_base_t> tag_content_ptr_t;

        template <class X>
        class tag_content_t : public tag_content_base_t
        {
        public:
            tag_content_t(const X &x) : x_(x) {}
            X &get(void) { return x_; }
            const X &get(void) const { return x_; }
        private:
            X x_;
        };
        template <class X>
        tag_content_t<X> *make_content(const X &x)
        {
            return new tag_content_t<X>(x);
        }

        class tag_algo_t : public tag_content_base_t
        {
        public:
            tag_algo_t(algo_base_t *algo) : algo_(algo) {}
            algo_base_t *algo(void) { return algo_.get(); }
        private:
            std::unique_ptr<algo_base_t> algo_;
        };

        tag_content_base_t *make_algo_content(algo_base_t *algo)
        {
            return new tag_algo_t(algo);
        }

        template <int Outputs, int Inputs, class D>
        struct domain_t
        {
            typedef algo_t<vector_t<double, Outputs>, vector_t<double, Inputs>> algo_type_t;
            static algo_base_t *type(algo_base_t *algob)
            {
                algo_type_t *algo = dynamic_cast<algo_type_t *>(algob);
                if (algo)
                    algob = domain_converter_t<vector_t<double, Outputs>,
                                                vector_t<double, Inputs>,
          D>::adapt(reinterpret_cast<typename color_conversion_t<vector_t<double, Inputs>, D>::domain_t *>(0), algo);
                return algob;
            }
        };

        template <class R, int Outputs, class D>
        struct range_t
        {
            typedef algo_t<vector_t<double, Outputs>, D> algo_type_t;
            static algo_base_t *type(algo_base_t *algob)
            {
                algo_type_t *algo = dynamic_cast<algo_type_t *>(algob);
                if (algo)
                    algob = range_converter_t<R, vector_t<double, Outputs>, D>::adapt(
                    reinterpret_cast<typename color_conversion_t<R, vector_t<double, Outputs>>::domain_t *>(0), algo);
                return algob;
            }
        };

        template <int Outputs, class D>
        tag_content_base_t *adapt_algo_range(algo_base_t *algo, color_space_t output)
        {
            switch (output)
            {
            case color_space_t::XYZData:
                algo = range_t<xyz_t, 3, D>::type(algo);
                break;
            case color_space_t::LabData:
                algo = range_t<lab_t, 3, D>::type(algo);
                break;
            case color_space_t::RgbData:
                algo = range_t<rgb_t<double>, 3, D>::type(algo);
                break;
            default:
                break ;
            }
            return make_algo_content(algo);
        }

        template <int Outputs, int Inputs>
        tag_content_base_t *adapt_algo_domain(algo_base_t *algo, color_space_t output, color_space_t input)
        {
            std::unique_ptr<algo_base_t> domain_adapted;
            switch (input)
            {
            case color_space_t::XYZData:
                domain_adapted.reset(domain_t<Outputs, 3, xyz_t>::type(algo));
                return adapt_algo_range<Outputs, xyz_t>(domain_adapted.get(), output);
            case color_space_t::LabData:
                domain_adapted.reset(domain_t<Outputs, 3, lab_t>::type(algo));
                return adapt_algo_range<Outputs, lab_t>(domain_adapted.get(), output);
            default:
                return make_algo_content(algo);
            }
        }

        template <class D, int N>
        class domain_space_t : virtual public visitor_domain_t<vector_t<double, N>>
        {
        public:
            domain_space_t(void) : result_(0) {}
            algo_base_t *result(algo_base_t *algo)
            {
                return (result_ == 0 ? algo : result_);
            }
            virtual void visit_domain(algo_domain_t<vector_t<double, N>> &algo) override
            {  // which is the range of algo? Do I care of it?
                result_ = algo.compose_domain(new color_conversion_t<vector_t<double, N>, xyz_t>);
            }
            static algo_base_t *convert(algo_base_t *algo)
            {
                domain_space_t<D, N> visitor;
                algo->accept_domain(visitor);
                return visitor.result();
            }
        private:
            algo_domain_t<D> *result_;
        };

        // tags used to get the right overload in lut_loaderIO
        struct BToAtag_t{};
        struct AToBtag_t{};
        struct Lut_816_t{};
        struct Lut_mpe_t{} ;
        // a strcture that helps in loading the tag
        template <class tag_t>
        struct io_size_trait_t
        {
            typedef icc_uint8_t type;
        };
        template  <>
        struct io_size_trait_t<Lut_mpe_t>
        {
            typedef icc_uint16_t type;
        };
        //
        // This class avoids a long switch case with all enumerations
        //
        template <class S, int N, int M>
        struct switch_unroller_t
        {
            template <class Tag>
            static tag_content_ptr_t lut_loaderI(S &s, int inputs, reader_t &reader)
            {
                if (inputs == N)
                    return s.template lut_loaderI<Tag, N>(reader);
                else
                    return switch_unroller_t<S, N + 1, M>::template lut_loaderI<Tag>(s, inputs, reader);
            }
            template <class Tag, int I>
            static tag_content_ptr_t lut_loaderIO(S &s, int outputs, reader_t &reader)
            {
                if (outputs == N)
                    return s.template lut_loaderIO<I, N>(Tag(), reader);
                else
                    return switch_unroller_t<S, N + 1, M>::template lut_loaderIO<Tag, I>(s, outputs, reader);
            }
        };

        template <class S, int M>
        struct switch_unroller_t<S, M, M>
        {
            template <class Tag>
            static tag_content_ptr_t lut_loaderI(S &, int, reader_t &)
            {
                throw icc_file_exception_t("Unsupported number of inputs");
            }
            template <class Tag, int I>
            static tag_content_ptr_t lut_loaderIO(S &, int, reader_t &)
            {
                throw icc_file_exception_t("Unsupported number of outputs");
            }
        };

        template <class S>
        using switch_unroll_t = switch_unroller_t<S, 1, 10>;

        template <class X>
        tag_content_t<algo_base_t *> *make_algo(const X &x)
        {
            return new tag_content_t<algo_base_t *>(x);
        }
        /// @brief This is the class that loads and holds the profile
        /// 
        ///
        class profile_imp_t : public profile_t
        {
        public:
            profile_imp_t(const profile_header_t &header, std::unique_ptr<std::ifstream> &stream) :
                header_(header), stream_(std::move(stream))
            {}
            virtual color_space_t pcs(void) const override
            {
                return header_.pcs;
            }
            virtual color_space_t device(void) const override
            {
                return header_.color_space;
            }
            void load_index(void)
            {
                icc_uint32_t tags = 0;
                reader_t reader(*stream_);

                reader(tags);
                for (icc_uint32_t i = 0; i < tags; i++)
                {
                    tag_entry_t tag;
                    exx(reader, tag);
                    if (*stream_)
                        tags_.push_back(tag);
                    else
                        throw icc_file_exception_t("Cannot read tags table");
                }
            }
            void list_tags(std::ostream &ost) const override
            {
                for (const tag_entry_t &tag : tags_)
                {
                    char type[4];
                    std::string signature(reinterpret_cast<const char *>(&tag.signature), 4);
                    stream_->seekg(tag.offset);
                    stream_->read(type, 4);
                    ost << signature << " -> " << std::string(type, 4) << "\n";
                }
            }
            virtual void load_tag(tag_signature_t signature) override
            {
                load_tag_imp(signature);
            }
            virtual void load_all(void) override
            {
                for (const tag_entry_t &tag : tags_)
                {
                    reader_t reader(*stream_, tag.offset);
                    load_tag(reader, tag.signature);
                }
            }
        private:
            virtual algo_base_t *dev2pcs(rendering_intent_t intent) override
            { // 'A' is device side, 'B' is PCS
                static const tag_signature_t sc_dev2pcs[] = { tag_signature_t::AToB0Tag,     // Perceptual
                                                              tag_signature_t::AToB1Tag,     // Relative colorimetric
                                                              tag_signature_t::AToB2Tag,     // Saturation
                                                              tag_signature_t::AToB1Tag };   // Absolute colorimetric

                static const tag_signature_t sc_dev2pcsfl[] = { tag_signature_t::DToB0Tag,     // Perceptual
                                                                tag_signature_t::DToB1Tag,     // Relative colorimetric
                                                                tag_signature_t::DToB2Tag,     // Saturation
                                                                tag_signature_t::DToB3Tag };   // Absolute colorimetric

                return get_tag(get_preferred_tag(intent, sc_dev2pcsfl, sc_dev2pcs));
            }
            virtual algo_base_t *pcs2dev(rendering_intent_t intent) override
            {  // 'A' is device side, 'B' is PCS
                static const tag_signature_t sc_pcs2dev[] = { tag_signature_t::BToA0Tag,     // Perceptual
                                                              tag_signature_t::BToA1Tag,     // Relative colorimetric
                                                              tag_signature_t::BToA2Tag,     // Saturation
                                                              tag_signature_t::BToA1Tag };   // Absolute colorimetric

                static const tag_signature_t sc_pcs2devfl[] = { tag_signature_t::BToD0Tag,     // Perceptual
                                                                tag_signature_t::BToD1Tag,     // Relative colorimetric
                                                                tag_signature_t::BToD2Tag,     // Saturation
                                                                tag_signature_t::BToD3Tag };   // Absolute colorimetric

                return get_tag(get_preferred_tag(intent, sc_pcs2devfl, sc_pcs2dev));
            }
            tag_signature_t get_preferred_tag(rendering_intent_t intent,
                                              const tag_signature_t *fltags,
                                              const tag_signature_t *tags)
            {
                size_t index = static_cast<size_t>(intent);
                if (index > 4)
                    index = 0;
                tag_signature_t signature = fltags[index]; // prefer floating point if present
                //if (!has_tag(signature)) multi process not supported yet
                    signature = tags[index];
                if (!has_tag(signature))
                    signature = tags[0]; // default is perceptual
                return signature;
            }
            algo_base_t *get_tag(tag_signature_t signature)
            {
                tag_content_ptr_t result = load_tag_imp(signature);
                tag_algo_t *tag_algo = dynamic_cast<tag_algo_t *>(result.get());
                if (tag_algo != nullptr)
                    return tag_algo->algo();
                else
                    return nullptr;
            }
            tag_content_ptr_t load_tag_imp(tag_signature_t signature)
            {
                for (const tag_entry_t &tag : tags_)
                    if (tag.signature == signature)
                    {
                        reader_t reader(*stream_, tag.offset);
                        return load_tag(reader, signature);
                    }
                return tag_content_ptr_t();
            }
            bool has_tag(tag_signature_t signature)
            {
                for (const tag_entry_t &tag : tags_)
                    if (tag.signature == signature)
                        return true;
                return false;
            }
            tag_type_t get_tag(reader_t &reader)
            {
                tag_type_t tag_type;
                reader.cast(tag_type);
                reader.zero(4);
                return tag_type;
            }
            tag_content_ptr_t load_tag(reader_t &reader, tag_signature_t signature)
            {
                tag_content_ptr_t content;
                auto it = contents_.find(signature);
                if (it != contents_.end())
                    return it->second;
                tag_type_t tag = get_tag(reader);
                switch (tag)
                {
                case tag_type_t::LutAtoBType:
                    content = lut_loader<AToBtag_t>(reader);
                    break;
                case tag_type_t::LutBtoAType:
                    content = lut_loader<BToAtag_t>(reader);
                    break;
                case tag_type_t::Lut16Type:
                    reader.entry_size(2);
                    content = lut_loader<Lut_816_t>(reader);
                    break;
                case tag_type_t::Lut8Type:
                    reader.entry_size(1);
                    content = lut_loader<Lut_816_t>(reader);
                    break;
                case tag_type_t::MultiProcessElementType:
                    //content = lut_loader<Lut_mpe_t>(reader);
                    break;
                case tag_type_t::MultiLocalizedUnicodeType:
                    break;
                default:
                    break;
                }
                if (content)
                    contents_[signature] = content;
                return content;
            }
            template <class Tag>
            tag_content_ptr_t lut_loader(reader_t &reader)
            {
                auto inputs = reader.read<typename io_size_trait_t<Tag>::type>();
                return switch_unroll_t<profile_imp_t>::lut_loaderI<Tag>(*this, inputs, reader);
            }
        public: // architecturally not sound, but required by switch unroller...
            template <class Tag, int Inputs>
            tag_content_ptr_t lut_loaderI(reader_t &reader)
            {
                auto outputs = reader.read<typename io_size_trait_t<Tag>::type>();
                return switch_unroll_t<profile_imp_t>::template lut_loaderIO<Tag, Inputs>(*this, outputs, reader);
            }
            /*
            template <int Outputs, int Inputs>
            tag_content_ptr_t lut_loaderIO(Lut_mpe_t, reader_t reader)
            {
                tag_content_ptr_t result;
                icc_uint16_t elem = reader.read<icc_uint16_t>();

                return result;
            }
            */
            template <int Outputs, int Inputs>
            tag_content_ptr_t lut_loaderIO(Lut_816_t, reader_t reader)
            {
                tag_content_ptr_t result;
                icc_uint8_t grids = reader.read<icc_uint8_t>();
                reader.zero(1);
                function_vect_t<3, 3> matrix(build_matrix_linear<3, 3>(reader));
                //reader_t reader1 = (reader + 48);// if Inputs != 3 revious line is wrong and matrix discarded
                icc_uint16_t input_entries = (reader.entry_size() == 2 ? reader.read<icc_uint16_t>() : 256);
                icc_uint16_t output_entries = (reader.entry_size() == 2 ? reader.read<icc_uint16_t>() : 256);

                function_vect_t<Inputs, Inputs> input(build_curve_m<Inputs>(reader, input_entries));
                size_t gridpoints[Inputs];
                arithmetic_t<size_t, Inputs>::assign(gridpoints, grids);
                function_vect_t<Outputs, Inputs> clut(build_clut1<vector_t<double, Outputs>, 
                                                                  vector_t<double, Inputs >> (reader, 
                                                                                              step<Inputs>(gridpoints), 
                                                                                              gridpoints));
                function_vect_t<Outputs, Outputs> output(build_curve_m<Outputs>(reader, output_entries));
                function_vect_t<Inputs, Inputs>  f1 = cond_prod_t<Inputs, Inputs>::exec(input, matrix);
                function_vect_t<Outputs, Inputs> f2 = (output *(clut * f1));
                return tag_content_ptr_t(adapt_algo_domain<Outputs, Inputs>(f2.get(), header_.color_space, header_.pcs));
            }
            //
            // A to B reading stuff
            //
            template <int Outputs, int Inputs>
            tag_content_ptr_t lut_loaderIO(AToBtag_t, reader_t &reader)
            {
                tag_content_ptr_t result;

                reader.zero(2);
                icc_uint32_t b_curve_offset = reader.read<icc_uint32_t>();
                icc_uint32_t matrix_offset = reader.read<icc_uint32_t>();
                icc_uint32_t m_curve_offset = reader.read<icc_uint32_t>();
                icc_uint32_t clut_offset = reader.read<icc_uint32_t>();
                icc_uint32_t a_curve_offset = reader.read<icc_uint32_t>();

                // TODO take into account cases where an offset is null

                function_vect_t<Outputs, Outputs> bcurve(build_curve_n<Outputs>(reader + b_curve_offset));
                function_vect_t<3, 3> matrix(build_matrix_affine<3, 3>(reader + matrix_offset));
                function_vect_t<Outputs, Outputs> mcurve(build_curve_n<Outputs>(reader + m_curve_offset));
                function_vect_t<Outputs, Inputs> clut(build_clut<Outputs, Inputs>(reader + clut_offset));
                function_vect_t<Inputs, Inputs> acurve(build_curve_n<Inputs>(reader + a_curve_offset));

                function_vect_t<Outputs, Inputs> f1 = mcurve * (clut * acurve) ;
                function_vect_t<Outputs, Inputs> f2 = cond_prod_t<Outputs, Inputs>::exec(f1, matrix);
                function_vect_t<Outputs, Inputs> f3 = bcurve * f2;

                // nice, here we have an object and we know its type, but we must throw everything...
                return tag_content_ptr_t(adapt_algo_domain<Outputs, Inputs>(f3.get(), header_.pcs, header_.color_space));
            }
            //
            // B to A reading stuff
            //
            template <int Outputs, int Inputs>
            tag_content_ptr_t lut_loaderIO(BToAtag_t, reader_t &reader)
            {
                tag_content_ptr_t result;
                reader.zero(2);
                icc_uint32_t b_curve_offset = reader.read<icc_uint32_t>();
                icc_uint32_t matrix_offset = reader.read<icc_uint32_t>();
                icc_uint32_t m_curve_offset = reader.read<icc_uint32_t>();
                icc_uint32_t clut_offset = reader.read<icc_uint32_t>();
                icc_uint32_t a_curve_offset = reader.read<icc_uint32_t>();

                // TODO take into account cases where an offset is null

                function_vect_t<Inputs, Inputs> bcurve(build_curve_n<Inputs>(reader + b_curve_offset));
                function_vect_t<3, 3> matrix(build_matrix_affine<3, 3>(reader + matrix_offset));
                function_vect_t<Inputs, Inputs> mcurve(build_curve_n<Inputs>(reader + m_curve_offset));
                function_vect_t<Outputs, Inputs> clut(build_clut<Outputs, Inputs>(reader + clut_offset));
                function_vect_t<Outputs, Outputs> acurve(build_curve_n<Outputs>(reader + a_curve_offset));

                function_vect_t<Inputs, Inputs> f1 = cond_prod_t<Inputs, Inputs>::exec(bcurve, matrix);
                function_vect_t<Outputs, Inputs> f2 = acurve * (clut *(mcurve * f1)) ;
                // nice, here we have an object and we know its type, but we must throw everything...
                return tag_content_ptr_t(adapt_algo_domain<Outputs, Inputs>(f2.get(), header_.color_space, header_.pcs));
            }
            algo_t<double, double> *build_parametric_curve(reader_t &reader)
            {
                icc_uint16_t type = reader.read<icc_uint16_t>();
                reader.zero(2);
                double a = 1.0;
                double b = 0.0;
                double c = 0.0;
                double d = 0.0;
                double e = 0.0;
                double f = 0.0;
                double g = reader.read_s15f16();
                while (type >=1)
                {
                    a = reader.read_s15f16();
                    b = reader.read_s15f16();
                    if (type == 1)
                        break;
                    c = reader.read_s15f16();
                    if (type == 2)
                        break;
                    d = reader.read_s15f16();
                    if (type == 3)
                        break;
                    e = reader.read_s15f16();
                    f = reader.read_s15f16();
                    break;
                }
                return new icc_curve_parametric_t(g, a, b, c, d, e, f);
            }
            algo_t<double, double> *build_piecewise_curve(reader_t &reader)
            {
                icc_uint32_t count = reader.read<icc_uint32_t>() ;
                if (count == 0)
                    return new identity_t<double>;
                else if (count == 1)
                    return new icc_gamma_curve_t(reader.read_u8f8());
                else
                {
                    std::vector<double> knots;
                    for (icc_uint32_t i = 0; i < count; i++)
                        knots.push_back(reader.read_u0f16());
                    return new icc_curve_interpolated_t(knots);
                }
            }
            algo_t<double, double> *build_curve(reader_t &reader)
            {
                algo_t<double, double> *result = 0;
                switch (get_tag(reader))
                {
                case tag_type_t::CurveType:
                    result = build_piecewise_curve(reader);
                    break;
                case tag_type_t::ParametricCurveType:
                    result = build_parametric_curve(reader);
                    break;
                default:
                    throw icc_file_exception_t("Unknown curve type");
                }
                return result;
            }
            template <int Inputs>
            icc_curve_t<Inputs> *build_curve_n(reader_t reader)
            {
                algo_t<double, double> *components[Inputs];
                for (icc_uint8_t index = 0; index < Inputs; index++)
                    components[index] = build_curve(reader);
                return new icc_curve_t<Inputs>(components);
            }
            template <int Dimension>
            icc_curve_t<Dimension> *build_curve_m(reader_t reader, size_t grids)
            {
                algo_t<double, double> *components[Dimension];
                for (icc_uint8_t index = 0; index < Dimension; index++)
                {
                    std::vector<double> knots;
                    for (icc_uint32_t i = 0; i < grids; i++)
                        knots.push_back(reader.read<double>());
                    components[index] = new icc_curve_interpolated_t(knots);
                }
                return new icc_curve_t<Dimension>(components);
            }
            template <int Outputs, int Inputs>
            algo_vect_t<Outputs, Inputs> *build_clut(reader_t reader)
            {
                icc_uint8_t prec[4];
                icc_uint8_t gridpoints8[16];
                size_t gridpoints[16];
                reader(gridpoints8);
                reader(prec);
                // only prec[0] should be non zero
                arithmetic_t<size_t, 16>::assign<icc_uint8_t>(gridpoints, gridpoints8);
                reader.entry_size(prec[0]);

                return build_clut1<vector_t<double, Outputs>, 
                                   vector_t<double, Inputs >> (reader, step<Inputs>(gridpoints), gridpoints);
            }
            // build step vector for the CLUT out of grid points
            template <int Dim>
            vector_t<double, Dim> step(const size_t *gridpoints)
            {
                vector_t<double, Dim> result;
                for (size_t i = 0; i < Dim; i++ )
                    result[i] = 1.0 / (gridpoints[i] - 1);
                return result;
            }
            // Lab uses multilinear interpolation.
            // Here is better to switch to a run time check, this way to work
            // is a symptom of metaprogramming addiction...
            template <class Y, class X>
            algo_t<Y, X> *build_clut1(reader_t &reader, const X &step,  size_t *gridpoints)
            {
                if (header_.pcs == color_space_t::LabData)
                    return build_clut2<Y, X, interp_multi_t>(reader, step, gridpoints);
                else
                    return build_clut2<Y, X, interp_tetra_t>(reader, step, gridpoints);
            }
            // this is required because inside ICC profile the indexing scheme is different
            size_t revindex(size_t i, const size_t *gridpoints, size_t n)
            {
                size_t result = 0;
                for (size_t j = 0; j < n; j++)
                {
                    if (j > 0)
                        result *= gridpoints[j];
                    result += i % gridpoints[j];
                    i /= gridpoints[j];
                }
                return result;
            }
            // fill in CLUT data
            template <class Y, class X, class I>
            algo_t<Y, X> *build_clut2(reader_t &reader, const X &step, const size_t *gridpoints)
            {
                lut_t<Y, X, I> *result = new lut_t<Y, X, I>(gridpoints, step);
                size_t samples = result->datasize();
                for (size_t i = 0; i < samples; i++)
                    (*result)[revindex(i, gridpoints, X::dimension)] = reader.read<Y>();
                 return result;
            }
            // read matrix sections.
            // It's a templat but actually just the 3x3 case is used.
            template <int Inputs, int Outputs>
            icc_matrix_affine_t<Outputs, Inputs> *build_matrix_affine(reader_t reader)
            {
                std::unique_ptr<icc_matrix_affine_t<Outputs, Inputs>> result(new icc_matrix_affine_t<Outputs, Inputs>());
                for (int i = 0; i < Outputs; i++)
                {
                    double *line = (*result)[i];
                    for (int j = 0; j < Inputs; j++)
                        line[j] = reader.read_s15f16();
                }
                for (int j = 0; j < Outputs; j++)
                    (*result)[j][Inputs] = reader.read_s15f16();
                return result.release();
            }
            template <int Inputs, int Outputs>
            icc_matrix_linear_t<Outputs, Inputs> *build_matrix_linear(reader_t reader)
            {
                std::unique_ptr<icc_matrix_linear_t<Outputs, Inputs>> result(new icc_matrix_linear_t<Outputs, Inputs>());
                for (int i = 0; i < Outputs; i++)
                {
                    double *line = (*result)[i];
                    for (int j = 0; j < Inputs; j++)
                        line[j] = reader.read_s15f16();
                }
                return result.release();
            }
            profile_header_t                    header_;
            std::unique_ptr<std::ifstream>      stream_;
            std::vector<tag_entry_t>            tags_;
            std::map<tag_signature_t, tag_content_ptr_t> contents_;
        };

        class profile_srgb_t : public profile_t
        {
        public:
            virtual color_space_t pcs(void) const { return color_space_t::XYZData; }
            virtual color_space_t device(void) const { return color_space_t::RgbData; }
        protected:
            virtual algo_base_t *dev2pcs(rendering_intent_t)
            {
                if (!rgb2xyz_)
                    rgb2xyz_.reset(new color_conversion_t<xyz_t, rgb_t<double>>);
                return rgb2xyz_.get();
            }
            virtual algo_base_t *pcs2dev(rendering_intent_t)
            {
                if (!xyz2rgb_)
                    xyz2rgb_.reset(new color_conversion_t<rgb_t<double>, xyz_t>);
                return xyz2rgb_.get();
            }
        private:
            std::unique_ptr<algo_t<xyz_t, rgb_t<double>>>   rgb2xyz_;
            std::unique_ptr<algo_t<rgb_t<double>, xyz_t>>   xyz2rgb_;
        };
    } // namespace loader


    profile_t *profile_t::create(const char *file)
    {
        std::unique_ptr<std::ifstream> stream(new std::ifstream(file, std::ios_base::binary));
        if (!stream)
            return nullptr;
        return profile_t::create(stream);
    }

    profile_t *profile_t::create_sRGB(void)
    {
        return new loader::profile_srgb_t;
    }

    profile_t *profile_t::create(std::unique_ptr<std::ifstream> &stream)
    {
        loader::reader_t reader(*stream);
        profile_header_t header;

        exx(reader, header);

        if (header.magic != static_cast<int>(specials_t::MagicNumber))
            return 0;
        std::unique_ptr<loader::profile_imp_t> result(new loader::profile_imp_t(header, stream));
        result->load_index();
        return result.release() ;
    }
}
