/*******************************************************************/
/*                                                                 */
/*                      LUNAR --FX PLUGIN                          */
/*                   LUNAR.IMP Effect                              */
/*                                                                 */
/* Copyright 2026 LUNAR --FX. All Rights Reserved.                 */
/*                                                                 */
/* Let your edit shine like a moon                                 */
/*                                                                 */
/*******************************************************************/

#pragma once
#ifndef LUNAR_IMP_H
#define LUNAR_IMP_H

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

#define NAME			"LUNAR.IMP"
#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1

enum {
	IMP_INPUT = 0,
	IMP_IMPACT_FRAME,
	IMP_HOLD,
	IMP_ANCHOR,
	IMP_RADIUS,
	IMP_EDGE_TYPE,
	IMP_NUM_PARAMS
};

enum {
	IMPACT_FRAME_DISK_ID = 1,
	HOLD_DISK_ID,
	ANCHOR_DISK_ID,
	RADIUS_DISK_ID,
	EDGE_TYPE_DISK_ID
};

#define RADIUS_MIN		0
#define RADIUS_MAX		100
#define RADIUS_DEFAULT	50

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

typedef struct ImpInfo {
	PF_FpLong	radiusF;
	A_long		edgeTypeL;
	A_long		widthL;
	A_long		heightL;
	A_long		impactFrameL;
	A_Boolean	holdActiveB;
	PF_Fixed	anchorX;
	PF_Fixed	anchorY;
	char		impactFilePath[256];
} ImpInfo, *ImpInfoP, **ImpInfoH;

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

#endif // LUNAR_IMP_H
