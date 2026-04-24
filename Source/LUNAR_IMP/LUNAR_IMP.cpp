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

#include "LUNAR_IMP.h"
#include <stdio.h>

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_SPRINTF(	out_data->return_msg, 
				"%s v%d.%d\r%s\r\rLUNAR --FX\rhttps://github.com/NiklasNK-Creator/LUNAR--FX",
				NAME, 
				MAJOR_VERSION, 
				MINOR_VERSION, 
				"Let your edit shine like a moon");

	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (
	PF_InData		*in_dataP,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err	err				= PF_Err_NONE;

	out_data->my_version	= PF_VERSION(	MAJOR_VERSION, 
											MINOR_VERSION,
											BUG_VERSION, 
											STAGE_VERSION, 
											BUILD_VERSION);
	
	out_data->out_flags		=	PF_OutFlag_PIX_INDEPENDENT	|
								PF_OutFlag_DEEP_COLOR_AWARE |
								PF_OutFlag_NON_PARAM_VARY;

	out_data->out_flags2	=	PF_OutFlag2_FLOAT_COLOR_AWARE	|
								PF_OutFlag2_SUPPORTS_SMART_RENDER	|
								PF_OutFlag2_SUPPORTS_THREADED_RENDERING;

	if (in_dataP->appl_id == 'PrMr'){

		AEFX_SuiteScoper<PF_PixelFormatSuite1> pixelFormatSuite = 
			AEFX_SuiteScoper<PF_PixelFormatSuite1>(	in_dataP,
													kPFPixelFormatSuite,
													kPFPixelFormatSuiteVersion1,
													out_data);

		(*pixelFormatSuite->ClearSupportedPixelFormats)(in_dataP->effect_ref);	
		(*pixelFormatSuite->AddSupportedPixelFormat)(
													in_dataP->effect_ref,
													PrPixelFormat_VUYA_4444_32f);
		(*pixelFormatSuite->AddSupportedPixelFormat)(
													in_dataP->effect_ref,
													PrPixelFormat_BGRA_4444_32f);	
		(*pixelFormatSuite->AddSupportedPixelFormat)(
													in_dataP->effect_ref,
													PrPixelFormat_VUYA_4444_8u);
		(*pixelFormatSuite->AddSupportedPixelFormat)(
													in_dataP->effect_ref,
													PrPixelFormat_BGRA_4444_8u);
	}

	return err;
}

static PF_Err 
ParamsSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output)
{
	PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;
	
	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX("Impact Intensity", 
						INTENSITY_MIN,
						INTENSITY_MAX,
						INTENSITY_MIN,
						INTENSITY_MAX,
						INTENSITY_DEFAULT,
						SLIDER_PRECISION,
						DISPLAY_FLAGS,
						0,
						INTENSITY_DISK_ID);

	AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP("Hold Frame", 2, 0, STR(StrID_HoldChoices), HOLD_DISK_ID);
	
	out_data->num_params = IMP_NUM_PARAMS;

	return err;
}

template<typename T>
inline T clamp(T value, T min, T max) {
	return (value < min) ? min : (value > max) ? max : value;
}

static void ApplyImpEffect8(PF_Pixel8 *inP, PF_Pixel8 *outP, ImpInfo *iiP)
{
	if (!iiP || !inP || !outP) return;
	
	PF_FpLong intensity = iiP->impactIntensityF / 100.0;
	
	if (intensity <= 0.0) {
		*outP = *inP;
		return;
	}
	
	PF_FpLong brightness = 1.0 + intensity * 0.5;
	
	outP->alpha = inP->alpha;
	outP->red = (A_u_char)clamp(inP->red * brightness, 0.0, 255.0);
	outP->green = (A_u_char)clamp(inP->green * brightness, 0.0, 255.0);
	outP->blue = (A_u_char)clamp(inP->blue * brightness, 0.0, 255.0);
}

static PF_Err 
FilterImage8 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err			err = PF_Err_NONE;
	
	ImpInfo *		iiP		= reinterpret_cast<ImpInfo*>(refcon);
				
	if (iiP && iiP->impactIntensityF > 0.0) {
		static PF_Pixel8 *frameBuffer = NULL;
		static A_long bufferWidth = 0;
		static A_long bufferHeight = 0;
		static A_Boolean frameCaptured = FALSE;
		
		if (iiP->holdActiveB && frameCaptured && bufferWidth == iiP->widthL && bufferHeight == iiP->heightL) {
			PF_Pixel8 *capturedP = frameBuffer + yL * iiP->widthL + xL;
			ApplyImpEffect8(capturedP, outP, iiP);
		} else {
			if (!frameBuffer || bufferWidth != iiP->widthL || bufferHeight != iiP->heightL) {
				if (frameBuffer) free(frameBuffer);
				bufferWidth = iiP->widthL;
				bufferHeight = iiP->heightL;
				frameBuffer = (PF_Pixel8*)calloc(bufferWidth * bufferHeight, sizeof(PF_Pixel8));
				frameCaptured = FALSE;
			}
			
			if (frameBuffer) {
				frameBuffer[yL * iiP->widthL + xL] = *inP;
				if (xL == bufferWidth-1 && yL == bufferHeight-1) frameCaptured = TRUE;
				ApplyImpEffect8(inP, outP, iiP);
			} else {
				*outP = *inP;
			}
		}
	} else {
		*outP = *inP;
	}
	return err;
}

static PF_Err 
Render ( 
	PF_InData		*in_dataP,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;

	ImpInfo			iiP;
	A_long				linesL	= 0;
	
	AEFX_CLR_STRUCT(iiP);
	
	linesL 		= output->extent_hint.bottom - output->extent_hint.top;
	iiP.widthL = in_dataP->width;
	iiP.heightL = in_dataP->height;
	iiP.impactIntensityF = params[IMP_INTENSITY]->u.fs_d.value;
	iiP.holdActiveB = params[IMP_HOLD]->u.pd.value == 1;

	if (in_dataP->appl_id == 'PrMr') {

		AEFX_SuiteScoper<PF_PixelFormatSuite1> pixelFormatSuite = 
			AEFX_SuiteScoper<PF_PixelFormatSuite1>(	in_dataP,
													kPFPixelFormatSuite,
													kPFPixelFormatSuiteVersion1,
													out_data);

		PrPixelFormat destinationPixelFormat = PrPixelFormat_BGRA_4444_8u;

		pixelFormatSuite->GetPixelFormat(output, &destinationPixelFormat);

		if (destinationPixelFormat == PrPixelFormat_BGRA_4444_8u){

			AEFX_SuiteScoper<PF_Iterate8Suite1> iterate8Suite = 
				AEFX_SuiteScoper<PF_Iterate8Suite1>(in_dataP,
													kPFIterate8Suite,
													kPFIterate8SuiteVersion1,
													out_data);

			iterate8Suite->iterate(	in_dataP,
									0,
									linesL,
									&params[IMP_INPUT]->u.ld,
									NULL,
									(void*)&iiP,
									FilterImage8,
									output);	

		} else if (destinationPixelFormat == PrPixelFormat_VUYA_4444_8u){

			AEFX_SuiteScoper<PF_Iterate8Suite1> iterate8Suite = 
				AEFX_SuiteScoper<PF_Iterate8Suite1>(in_dataP,
													kPFIterate8Suite,
													kPFIterate8SuiteVersion1,
													out_data);

			iterate8Suite->iterate(	in_dataP,
									0,
									linesL,
									&params[IMP_INPUT]->u.ld,
									NULL,
									(void*)&iiP,
									FilterImage8,
									output);

		} else {
			return PF_Err_UNRECOGNIZED_PARAM_TYPE;
		}
		
	} else if(iiP.impactIntensityF > 0.0) {

		AEFX_SuiteScoper<PF_Iterate8Suite1> iterate8Suite = 
			AEFX_SuiteScoper<PF_Iterate8Suite1>(in_dataP,
												kPFIterate8Suite,
												kPFIterate8SuiteVersion1,
												out_data);

		iterate8Suite->iterate(	in_dataP,
								0,
								linesL,
								&params[IMP_INPUT]->u.ld,
								NULL,
								(void*)&iiP,
								FilterImage8,
								output);
	} else {

		AEFX_SuiteScoper<PF_WorldTransformSuite1> worldTransformSuite = 
			AEFX_SuiteScoper<PF_WorldTransformSuite1>(	in_dataP,
														kPFWorldTransformSuite,
														kPFWorldTransformSuiteVersion1,
														out_data);

		worldTransformSuite->copy(	in_dataP->effect_ref,
									&params[IMP_INPUT]->u.ld,
									output,
									NULL,
									NULL);
	}
	return err;
}

static PF_Err
PreRender(
	PF_InData			*in_dataP,
	PF_OutData			*out_dataP,
	PF_PreRenderExtra	*extraP)
{
	PF_Err err = PF_Err_NONE;
	PF_ParamDef intensity_param, hold_param;
	PF_RenderRequest req = extraP->input->output_request;
	PF_CheckoutResult in_result;
	
	AEFX_CLR_STRUCT(intensity_param);
	AEFX_CLR_STRUCT(hold_param);

	AEFX_SuiteScoper<PF_HandleSuite1> handleSuite = AEFX_SuiteScoper<PF_HandleSuite1>(	in_dataP,
																					kPFHandleSuite,
																					kPFHandleSuiteVersion1,
																					out_dataP);

	PF_Handle infoH	= handleSuite->host_new_handle(sizeof(ImpInfo));
	
	if (infoH) {
		ImpInfo *infoP = reinterpret_cast<ImpInfo*>(handleSuite->host_lock_handle(infoH));
		
		if (infoP) {
			ERR(PF_CHECKOUT_PARAM(	in_dataP, 
									IMP_INTENSITY, 
									in_dataP->current_time, 
									in_dataP->time_step, 
									in_dataP->time_scale, 
									&intensity_param));
			
			ERR(PF_CHECKOUT_PARAM(	in_dataP, 
									IMP_HOLD, 
									in_dataP->current_time, 
									in_dataP->time_step, 
									in_dataP->time_scale, 
									&hold_param));
			
			if (!err){
				infoP->widthL = in_dataP->width;
				infoP->heightL = in_dataP->height;
				infoP->impactIntensityF = intensity_param.u.fs_d.value;
				infoP->holdActiveB = hold_param.u.pd.value == 1;
			}
			
			ERR(extraP->cb->checkout_layer(	in_dataP->effect_ref,
											IMP_INPUT,
											IMP_INPUT,
											&req,
											in_dataP->current_time,
											in_dataP->time_step,
											in_dataP->time_scale,
											&in_result));
											
			if (!err) {
				UnionLRect(&in_result.result_rect, &extraP->output->result_rect);
				UnionLRect(&in_result.max_result_rect, &extraP->output->max_result_rect);
			}
			
			handleSuite->host_unlock_handle(infoH);
		}
		
		extraP->output->pre_render_data = infoH;
	} else {
		err = PF_Err_OUT_OF_MEMORY;
	}
	
	return err;
}

static PF_Err
SmartRender(
	PF_InData				*in_data,
	PF_OutData				*out_data,
	PF_SmartRenderExtra		*extraP)
{
	
	PF_Err			err		= PF_Err_NONE,
					err2 	= PF_Err_NONE;
	
	PF_EffectWorld	*input_worldP	= NULL, 
					*output_worldP  = NULL;

	AEFX_SuiteScoper<PF_HandleSuite1> handleSuite = AEFX_SuiteScoper<PF_HandleSuite1>(	in_data,
																					kPFHandleSuite,
																					kPFHandleSuiteVersion1,
																					out_data);
	
	ImpInfo	*infoP = reinterpret_cast<ImpInfo*>(handleSuite->host_lock_handle(reinterpret_cast<PF_Handle>(extraP->input->pre_render_data)));
	
	if (infoP){
		ERR((extraP->cb->checkout_layer_pixels(	in_data->effect_ref, IMP_INPUT, &input_worldP)));
		ERR(extraP->cb->checkout_output(in_data->effect_ref, &output_worldP));
		
		PF_PixelFormat		format	=	PF_PixelFormat_INVALID;
		
		AEFX_SuiteScoper<PF_WorldSuite2> wsP = AEFX_SuiteScoper<PF_WorldSuite2>(in_data,
																			kPFWorldSuite,
																			kPFWorldSuiteVersion2,
																			out_data);
		
		if (infoP->impactIntensityF == 0.0) {
			err = PF_COPY(input_worldP, output_worldP, NULL, NULL);
		} else {
			ERR(wsP->PF_GetPixelFormat(input_worldP, &format));
			
			if (!err){

				AEFX_SuiteScoper<PF_iterateFloatSuite1> iterateFloatSuite =
					AEFX_SuiteScoper<PF_iterateFloatSuite1>(in_data,
															kPFIterateFloatSuite,
															kPFIterateFloatSuiteVersion1,
															out_data);
				AEFX_SuiteScoper<PF_iterate16Suite1> iterate16Suite =
					AEFX_SuiteScoper<PF_iterate16Suite1>(	in_data,
															kPFIterate16Suite,
															kPFIterate16SuiteVersion1,
															out_data);

				AEFX_SuiteScoper<PF_Iterate8Suite1> iterate8Suite =
					AEFX_SuiteScoper<PF_Iterate8Suite1>(	in_data,
															kPFIterate8Suite,
															kPFIterate8SuiteVersion1,
															out_data);
				switch (format) {
					
					case PF_PixelFormat_ARGB128:
		
						err = PF_COPY(input_worldP, output_worldP, NULL, NULL);
						break;
						
					case PF_PixelFormat_ARGB64:
										
						err = PF_COPY(input_worldP, output_worldP, NULL, NULL);
						break;
						
					case PF_PixelFormat_ARGB32:
			
						iterate8Suite->iterate(	in_data,
												0,
												output_worldP->height,
												input_worldP,
												NULL,
												(void*)infoP,
												FilterImage8,
												output_worldP);
						break;
						
					default:
						err = PF_Err_BAD_CALLBACK_PARAM;
						break;
				}
			}		
		}
		ERR2(extraP->cb->checkin_layer_pixels(in_data->effect_ref, IMP_INPUT));
	}
	return err;
	
}

extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		"LUNAR.IMP",
		"LUNAR IMP",
		"LUNAR --FX",
		AE_RESERVED_INFO);

	return result;
}

PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_dataP,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) 
		{
			case PF_Cmd_ABOUT:
				err = About(in_dataP,out_data,params,output);
				break;
			case PF_Cmd_GLOBAL_SETUP:
				err = GlobalSetup(in_dataP,out_data,params,output);
				break;
			case PF_Cmd_PARAMS_SETUP:
				err = ParamsSetup(in_dataP,out_data,params,output);
				break;
			case PF_Cmd_RENDER:
				err = Render(in_dataP,out_data,params,output);
				break;
			case PF_Cmd_SMART_PRE_RENDER:
				err = PreRender(in_dataP, out_data, (PF_PreRenderExtra*)extra);
				break;
			case PF_Cmd_SMART_RENDER:
				err = SmartRender(in_dataP, out_data, (PF_SmartRenderExtra*)extra);
				break;
		}
	} catch(PF_Err &thrown_err) {
		err = thrown_err;
	}
	return err;
}
