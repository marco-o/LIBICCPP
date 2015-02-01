#ifndef iccpp_tags_H
#define iccpp_tags_H
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
	// Let's start introduce names in this way
	typedef unsigned char  icc_uint8_t;
	typedef unsigned short icc_uint16_t;
	typedef unsigned int   icc_uint32_t;
	typedef int            icc_int32_t;
	typedef unsigned long long icc_uint64_t;
	typedef icc_uint32_t   icc_s15f16_number_t;
	//typedef cmsTagSignature tag_signature_t;


	enum class rendering_intent_t
	{
		perceptual            = 0,
		relative_colorimetric = 1,
		saturation            = 2,
		absolute_colorimetric = 3,
		place_holding         = 0x010203
	};

	// ICC Profile Class
	enum class class_signature_t
	{
		InputClass      = 0x73636E72,  // 'scnr'
		DisplayClass    = 0x6D6E7472,  // 'mntr'
		OutputClass     = 0x70727472,  // 'prtr'
		LinkClass       = 0x6C696E6B,  // 'link'
		AbstractClass   = 0x61627374,  // 'abst'
		ColorSpaceClass = 0x73706163,  // 'spac'
		NamedColorClass = 0x6e6d636c   // 'nmcl'
	} ;

	enum class tag_signature_t
	{
		AToB0Tag                          = 0x41324230,  // 'A2B0'
		AToB1Tag                          = 0x41324231,  // 'A2B1'
		AToB2Tag                          = 0x41324232,  // 'A2B2'
		BlueColorantTag                   = 0x6258595A,  // 'bXYZ'
		BlueMatrixColumnTag               = 0x6258595A,  // 'bXYZ'
		BlueTRCTag                        = 0x62545243,  // 'bTRC'
		BToA0Tag                          = 0x42324130,  // 'B2A0'
		BToA1Tag                          = 0x42324131,  // 'B2A1'
		BToA2Tag                          = 0x42324132,  // 'B2A2'
		CalibrationDateTimeTag            = 0x63616C74,  // 'calt'
		CharTargetTag                     = 0x74617267,  // 'targ'
		ChromaticAdaptationTag            = 0x63686164,  // 'chad'
		ChromaticityTag                   = 0x6368726D,  // 'chrm'
		ColorantOrderTag                  = 0x636C726F,  // 'clro'
		ColorantTableTag                  = 0x636C7274,  // 'clrt'
		ColorantTableOutTag               = 0x636C6F74,  // 'clot'
		ColorimetricIntentImageStateTag   = 0x63696973,  // 'ciis'
		CopyrightTag                      = 0x63707274,  // 'cprt'
		CrdInfoTag                        = 0x63726469,  // 'crdi'
		DataTag                           = 0x64617461,  // 'data'
		DateTimeTag                       = 0x6474696D,  // 'dtim'
		DeviceMfgDescTag                  = 0x646D6E64,  // 'dmnd'
		DeviceModelDescTag                = 0x646D6464,  // 'dmdd'
		DeviceSettingsTag                 = 0x64657673,  // 'devs'
		DToB0Tag                          = 0x44324230,  // 'D2B0'
		DToB1Tag                          = 0x44324231,  // 'D2B1'
		DToB2Tag                          = 0x44324232,  // 'D2B2'
		DToB3Tag                          = 0x44324233,  // 'D2B3'
		BToD0Tag                          = 0x42324430,  // 'B2D0'
		BToD1Tag                          = 0x42324431,  // 'B2D1'
		BToD2Tag                          = 0x42324432,  // 'B2D2'
		BToD3Tag                          = 0x42324433,  // 'B2D3'
		GamutTag                          = 0x67616D74,  // 'gamt'
		GrayTRCTag                        = 0x6b545243,  // 'kTRC'
		GreenColorantTag                  = 0x6758595A,  // 'gXYZ'
		GreenMatrixColumnTag              = 0x6758595A,  // 'gXYZ'
		GreenTRCTag                       = 0x67545243,  // 'gTRC'
		LuminanceTag                      = 0x6C756d69,  // 'lumi'
		MeasurementTag                    = 0x6D656173,  // 'meas'
		MediaBlackPointTag                = 0x626B7074,  // 'bkpt'
		MediaWhitePointTag                = 0x77747074,  // 'wtpt'
		NamedColorTag                     = 0x6E636f6C,  // 'ncol' // Deprecated by the ICC
		NamedColor2Tag                    = 0x6E636C32,  // 'ncl2'
		OutputResponseTag                 = 0x72657370,  // 'resp'
		PerceptualRenderingIntentGamutTag = 0x72696730,  // 'rig0'
		Preview0Tag                       = 0x70726530,  // 'pre0'
		Preview1Tag                       = 0x70726531,  // 'pre1'
		Preview2Tag                       = 0x70726532,  // 'pre2'
		ProfileDescriptionTag             = 0x64657363,  // 'desc'
		ProfileDescriptionMLTag           = 0x6473636d,  // 'dscm'
		ProfileSequenceDescTag            = 0x70736571,  // 'pseq'
		ProfileSequenceIdTag              = 0x70736964,  // 'psid'
		Ps2CRD0Tag                        = 0x70736430,  // 'psd0'
		Ps2CRD1Tag                        = 0x70736431,  // 'psd1'
		Ps2CRD2Tag                        = 0x70736432,  // 'psd2'
		Ps2CRD3Tag                        = 0x70736433,  // 'psd3'
		Ps2CSATag                         = 0x70733273,  // 'ps2s'
		Ps2RenderingIntentTag             = 0x70733269,  // 'ps2i'
		RedColorantTag                    = 0x7258595A,  // 'rXYZ'
		RedMatrixColumnTag                = 0x7258595A,  // 'rXYZ'
		RedTRCTag                         = 0x72545243,  // 'rTRC'
		SaturationRenderingIntentGamutTag = 0x72696732,  // 'rig2'
		ScreeningDescTag                  = 0x73637264,  // 'scrd'
		ScreeningTag                      = 0x7363726E,  // 'scrn'
		TechnologyTag                     = 0x74656368,  // 'tech'
		UcrBgTag                          = 0x62666420,  // 'bfd '
		ViewingCondDescTag                = 0x76756564,  // 'vued'
		ViewingConditionsTag              = 0x76696577,  // 'view'
		VcgtTag                           = 0x76636774,  // 'vcgt'
		MetaTag                           = 0x6D657461   // 'meta'
	};

	enum class tag_type_t
	{
		ChromaticityType                  = 0x6368726D,  // 'chrm'
		ColorantOrderType                 = 0x636C726F,  // 'clro'
		ColorantTableType                 = 0x636C7274,  // 'clrt'
		CrdInfoType                       = 0x63726469,  // 'crdi'
		CurveType                         = 0x63757276,  // 'curv'
		DataType                          = 0x64617461,  // 'data'
		DictType                          = 0x64696374,  // 'dict'
		DateTimeType                      = 0x6474696D,  // 'dtim'
		DeviceSettingsType                = 0x64657673,  // 'devs'
		Lut16Type                         = 0x6d667432,  // 'mft2'
		Lut8Type                          = 0x6d667431,  // 'mft1'
		LutAtoBType                       = 0x6d414220,  // 'mAB '
		LutBtoAType                       = 0x6d424120,  // 'mBA '
		MeasurementType                   = 0x6D656173,  // 'meas'
		MultiLocalizedUnicodeType         = 0x6D6C7563,  // 'mluc'
		MultiProcessElementType           = 0x6D706574,  // 'mpet'
		NamedColorType                    = 0x6E636f6C,  // 'ncol' -- DEPRECATED!
		NamedColor2Type                   = 0x6E636C32,  // 'ncl2'
		ParametricCurveType               = 0x70617261,  // 'para'
		ProfileSequenceDescType           = 0x70736571,  // 'pseq'
		ProfileSequenceIdType             = 0x70736964,  // 'psid'
		ResponseCurveSet16Type            = 0x72637332,  // 'rcs2'
		S15Fixed16ArrayType               = 0x73663332,  // 'sf32'
		ScreeningType                     = 0x7363726E,  // 'scrn'
		SignatureType                     = 0x73696720,  // 'sig '
		TextType                          = 0x74657874,  // 'text'
		TextDescriptionType               = 0x64657363,  // 'desc'
		U16Fixed16ArrayType               = 0x75663332,  // 'uf32'
		UcrBgType                         = 0x62666420,  // 'bfd '
		UInt16ArrayType                   = 0x75693136,  // 'ui16'
		UInt32ArrayType                   = 0x75693332,  // 'ui32'
		UInt64ArrayType                   = 0x75693634,  // 'ui64'
		UInt8ArrayType                    = 0x75693038,  // 'ui08'
		VcgtType                          = 0x76636774,  // 'vcgt'
		ViewingConditionsType             = 0x76696577,  // 'view'
		XYZType                           = 0x58595A20   // 'XYZ '
	} ;

	// ICC Color spaces
    enum class color_space_t
	{
        XYZData                           = 0x58595A20,  // 'XYZ '
        LabData                           = 0x4C616220,  // 'Lab '
        LuvData                           = 0x4C757620,  // 'Luv '
        YCbCrData                         = 0x59436272,  // 'YCbr'
        YxyData                           = 0x59787920,  // 'Yxy '
        RgbData                           = 0x52474220,  // 'RGB '
        GrayData                          = 0x47524159,  // 'GRAY'
        HsvData                           = 0x48535620,  // 'HSV '
        HlsData                           = 0x484C5320,  // 'HLS '
        CmykData                          = 0x434D594B,  // 'CMYK'
        CmyData                           = 0x434D5920,  // 'CMY '
        MCH1Data                          = 0x4D434831,  // 'MCH1'
        MCH2Data                          = 0x4D434832,  // 'MCH2'
        MCH3Data                          = 0x4D434833,  // 'MCH3'
        MCH4Data                          = 0x4D434834,  // 'MCH4'
        MCH5Data                          = 0x4D434835,  // 'MCH5'
        MCH6Data                          = 0x4D434836,  // 'MCH6'
        MCH7Data                          = 0x4D434837,  // 'MCH7'
        MCH8Data                          = 0x4D434838,  // 'MCH8'
        MCH9Data                          = 0x4D434839,  // 'MCH9'
        MCHAData                          = 0x4D434841,  // 'MCHA'
        MCHBData                          = 0x4D434842,  // 'MCHB'
        MCHCData                          = 0x4D434843,  // 'MCHC'
        MCHDData                          = 0x4D434844,  // 'MCHD'
        MCHEData                          = 0x4D434845,  // 'MCHE'
        MCHFData                          = 0x4D434846,  // 'MCHF'
        NamedData                         = 0x6e6d636c,  // 'nmcl'
        colorData1                        = 0x31434C52,  // '1CLR'
        colorData2                        = 0x32434C52,  // '2CLR'
        colorData3                        = 0x33434C52,  // '3CLR'
        colorData4                        = 0x34434C52,  // '4CLR'
        colorData5                        = 0x35434C52,  // '5CLR'
        colorData6                        = 0x36434C52,  // '6CLR'
        colorData7                        = 0x37434C52,  // '7CLR'
        colorData8                        = 0x38434C52,  // '8CLR'
        colorData9                        = 0x39434C52,  // '9CLR'
        colorData10                       = 0x41434C52,  // 'ACLR'
        colorData11                       = 0x42434C52,  // 'BCLR'
        colorData12                       = 0x43434C52,  // 'CCLR'
        colorData13                       = 0x44434C52,  // 'DCLR'
        colorData14                       = 0x45434C52,  // 'ECLR'
        colorData15                       = 0x46434C52,  // 'FCLR'
        LuvKData                          = 0x4C75764B   // 'LuvK'
    } ;

	enum class specials_t
	{
		// Definitions in ICC spec
		MagicNumber  =    0x61637370     // 'acsp'
	};

	struct date_time_number_t
	{
		icc_uint16_t      year;
		icc_uint16_t      month;
		icc_uint16_t      day;
		icc_uint16_t      hours;
		icc_uint16_t      minutes;
		icc_uint16_t      seconds;
	} ;

	struct encoded_xyz_t
	{
		icc_s15f16_number_t  X;
		icc_s15f16_number_t  Y;
		icc_s15f16_number_t  Z;
	} ;


	struct profile_header_t
	{
		icc_uint32_t        size;            // Profile size in bytes
		icc_uint32_t        cmm_id;          // CMM for this profile
		icc_uint32_t        version;         // Format version number
		class_signature_t   device_class;    // Type of profile
		color_space_t       color_space;     // Color space of data
		color_space_t       pcs;             // PCS, XYZ or Lab only
		date_time_number_t  date;            // Date profile was created
		icc_uint32_t        magic;           // Magic Number to identify an ICC profile
		icc_uint32_t        platform;        // Primary Platform
		icc_uint32_t        flags;           // Various bit settings
		icc_uint32_t        manufacturer;    // Device manufacturer
		icc_uint32_t        model;           // Device model number
		icc_uint64_t        attributes;      // Device attributes
		rendering_intent_t  rendering_intent;// Rendering intent
		encoded_xyz_t       illuminant;      // Profile illuminant
		icc_uint32_t        creator;         // Profile creator
		icc_uint8_t         profile_id[16];  // Profile ID using MD5
		icc_uint8_t         reserved[28];    // Reserved for future use
	};

}
#endif