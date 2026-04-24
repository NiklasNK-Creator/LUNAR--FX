/*******************************************************************/
/*                                                                 */
/*                      LUNAR --FX PLUGIN                          */
/*                   LUNAR.SNITCH Effect                           */
/*                                                                 */
/* Copyright 2026 LUNAR --FX. All Rights Reserved.                 */
/*                                                                 */
/* Subtle anime enhancement effect for AMV editing                 */
/*                                                                 */
/*******************************************************************/

#pragma once
#ifndef LUNAR_SNITCH_H
#define LUNAR_SNITCH_H

#include "AEConfig.h"
#include "entry.h"
#include "AEFX_SuiteHelper.h"
#include "PrSDKAESupport.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_EffectCBSuites.h"
#include "AE_Macros.h"
#include "AEGP_SuiteHandler.h"
#include "String_Utils.h"
#include "Param_Utils.h"
#include "Smart_Utils.h"

#ifdef AE_OS_WIN
	#include <Windows.h>
	#include <cmath>
#endif

#define DESCRIPTION	"\nCopyright 2026 LUNAR --FX.\rLet your edit shine like a moon."

#define NAME			"LUNAR.SNITCH"
#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1

enum {
	SNITCH_INPUT = 0,
	SNITCH_INTENSITY,		// Main effect intensity (controls everything)
	SNITCH_NUM_PARAMS
};

enum {
	INTENSITY_DISK_ID = 1
};

// Parameter ranges
#define INTENSITY_MIN		0
#define INTENSITY_MAX		100
#define INTENSITY_DEFAULT	30

#define SLIDER_PRECISION	1
#define DISPLAY_FLAGS		PF_ValueDisplayFlag_PERCENT

extern "C" {
	DllExport 
	PF_Err
	EffectMain (	
		PF_Cmd			cmd,
		PF_InData		*in_data,
		PF_OutData		*out_data,
		PF_ParamDef		*params[],
		PF_LayerDef		*output,
		void			*extra);
}

typedef struct SnitchInfo {
	PF_FpLong	intensityF;		// Main effect intensity (controls everything)
	A_long		widthL;			// Image width
	A_long		heightL;		// Image height
} SnitchInfo, *SnitchInfoP, **SnitchInfoH;

// Pixel format structures
typedef struct {
	A_u_char	blue, green, red, alpha;
} PF_Pixel_BGRA_8u;

typedef struct {
	A_u_char	Pr, Pb, luma, alpha;
} PF_Pixel_VUYA_8u;

typedef struct {
	PF_FpShort	blue, green, red, alpha;
} PF_Pixel_BGRA_32f;

typedef struct {
	PF_FpShort	Pr, Pb, luma, alpha;
} PF_Pixel_VUYA_32f;

#endif // LUNAR_SNITCH_H
