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
				"%s v%d.%d\r%s",
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
	
	strcpy_s(def.name, sizeof(def.name), "Impact Frame");
	PF_ADD_CHECKBOX(def.name, FALSE, IMPACT_FRAME_DISK_ID, 0, 0);
	
	AEFX_CLR_STRUCT(def);
	strcpy_s(def.name, sizeof(def.name), "Hold");
	PF_ADD_CHECKBOX(def.name, FALSE, HOLD_DISK_ID, 0, 0);
	
	PF_ADD_FLOAT_SLIDERX("Radius", 
						RADIUS_MIN,
						RADIUS_MAX,
						RADIUS_MIN,
						RADIUS_MAX,
						RADIUS_DEFAULT,
						SLIDER_PRECISION,
						DISPLAY_FLAGS,
						0,
						RADIUS_DISK_ID);
	
	AEFX_CLR_STRUCT(def);
	def.ui_flags = PF_PUI_CONTROL;
	PF_ADD_POPUP("Edge Type", 0, 7, "Fade\0Hard\0Flame\0Paint\0Grade\0Blend\0Rough\0", EDGE_TYPE_DISK_ID);
	
	out_data->num_params = IMP_NUM_PARAMS;

	return err;
}

template<typename T>
inline T clamp(T value, T min, T max) {
	return (value < min) ? min : (value > max) ? max : value;
}

static PF_Err SaveFrameToTempFile(PF_InData *in_data, PF_LayerDef *input_layer, A_long width, A_long height, char *filePath)
{
	PF_Err err = PF_Err_NONE;
	
	GetTempPathA(255, filePath);
	strcat_s(filePath, 256, "LUNAR_IMP_frame.raw");
	
	FILE *file = NULL;
	fopen_s(&file, filePath, "wb");
	
	if (file) {
		PF_EffectWorld *input_world = reinterpret_cast<PF_EffectWorld*>(input_layer);
		
		for (A_long y = 0; y < height; y++) {
			PF_Pixel8 *srcP = (PF_Pixel8*)((A_u_char*)input_world->data + y * input_world->rowbytes);
			fwrite(srcP, sizeof(PF_Pixel8), width, file);
		}
		
		fclose(file);
	} else {
		err = PF_Err_OUT_OF_MEMORY;
	}
	
	return err;
}

static PF_Err LoadFrameFromTempFile(char *filePath, PF_Pixel8 *frameBuffer, A_long width, A_long height)
{
	PF_Err err = PF_Err_NONE;
	
	FILE *file = NULL;
	fopen_s(&file, filePath, "rb");
	
	if (file) {
		for (A_long y = 0; y < height; y++) {
			PF_Pixel8 *dstP = frameBuffer + y * width;
			fread(dstP, sizeof(PF_Pixel8), width, file);
		}
		
		fclose(file);
	} else {
		err = PF_Err_OUT_OF_MEMORY;
	}
	
	return err;
}

static void ApplyImpEffect8(PF_Pixel8 *inP, PF_Pixel8 *outP, PF_Pixel8 *impactP, ImpInfo *iiP, A_long x, A_long y)
{
	if (!iiP || !inP || !outP || !impactP) return;
	
	PF_FpLong radius = iiP->radiusF / 100.0;
	
	if (radius <= 0.0) {
		*outP = *inP;
		return;
	}
	
	if (radius >= 1.0) {
		*outP = *impactP;
		return;
	}
	
	PF_FpLong anchorX = iiP->widthL / 2.0;
	PF_FpLong anchorY = iiP->heightL / 2.0;
	
	PF_FpLong distX = (x - anchorX) / (PF_FpLong)iiP->widthL;
	PF_FpLong distY = (y - anchorY) / (PF_FpLong)iiP->heightL;
	PF_FpLong distance = sqrt(distX * distX + distY * distY);
	
	PF_FpLong blend = clamp(distance / radius, 0.0, 1.0);
	
	switch (iiP->edgeTypeL) {
		case 0:
			blend = 1.0 - blend;
			blend = blend * blend * (3.0 - 2.0 * blend);
			break;
		case 1:
			blend = blend < 0.5 ? 0.0 : 1.0;
			break;
		case 2:
			blend = 1.0 - blend;
			blend = pow(blend, 0.5);
			break;
		case 3:
			blend = 1.0 - blend;
			blend = blend * blend;
			break;
		case 4:
			blend = 1.0 - blend;
			blend = pow(blend, 3.0);
			break;
		case 5:
			blend = 1.0 - blend;
			break;
		case 6:
			blend = 1.0 - blend;
			blend = 1.0 - pow(1.0 - blend, 2.0);
			break;
	}
	
	outP->alpha = inP->alpha;
	outP->red = (A_u_char)clamp(inP->red * (1.0 - blend) + impactP->red * blend, 0.0, 255.0);
	outP->green = (A_u_char)clamp(inP->green * (1.0 - blend) + impactP->green * blend, 0.0, 255.0);
	outP->blue = (A_u_char)clamp(inP->blue * (1.0 - blend) + impactP->blue * blend, 0.0, 255.0);
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
	
	ImpInfo *	iiP		= reinterpret_cast<ImpInfo*>(refcon);
				
	if (iiP && iiP->impactFrameL > 0 && strlen(iiP->impactFilePath) > 0) {
		static PF_Pixel8 *impactFrameBuffer = NULL;
		static A_long bufferWidth = 0;
		static A_long bufferHeight = 0;
		
		if (!impactFrameBuffer || bufferWidth != iiP->widthL || bufferHeight != iiP->heightL) {
			if (impactFrameBuffer) {
				free(impactFrameBuffer);
			}
			bufferWidth = iiP->widthL;
			bufferHeight = iiP->heightL;
			impactFrameBuffer = (PF_Pixel8*)malloc(bufferWidth * bufferHeight * sizeof(PF_Pixel8));
			
			if (impactFrameBuffer) {
				LoadFrameFromTempFile(iiP->impactFilePath, impactFrameBuffer, bufferWidth, bufferHeight);
			}
		}
		
		if (impactFrameBuffer) {
			PF_Pixel8 *impactPixelP = impactFrameBuffer + yL * iiP->widthL + xL;
			ApplyImpEffect8(inP, outP, impactPixelP, iiP, xL, yL);
		} else {
			*outP = *inP;
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
	iiP.radiusF = params[IMP_RADIUS]->u.fs_d.value;
	iiP.edgeTypeL = params[IMP_EDGE_TYPE]->u.pd.value;
	iiP.widthL = in_dataP->width;
	iiP.heightL = in_dataP->height;
	iiP.holdActiveB = params[IMP_HOLD]->u.bd.value;
	iiP.impactFilePath[0] = '\0';
	
	if (params[IMP_IMPACT_FRAME]->u.bd.value) {
		if (iiP.holdActiveB) {
		} else {
			SaveFrameToTempFile(in_dataP, &params[IMP_INPUT]->u.ld, iiP.widthL, iiP.heightL, iiP.impactFilePath);
		}
		iiP.impactFrameL = in_dataP->current_time;
	} else {
		iiP.impactFrameL = 0;
	}

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
		
	} else if(iiP.radiusF > 0.0) {

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
	PF_ParamDef impact_param, hold_param, radius_param, edge_param;
	PF_RenderRequest req = extraP->input->output_request;
	PF_CheckoutResult in_result;
	
	AEFX_CLR_STRUCT(impact_param);
	AEFX_CLR_STRUCT(hold_param);
	AEFX_CLR_STRUCT(radius_param);
	AEFX_CLR_STRUCT(edge_param);

	AEFX_SuiteScoper<PF_HandleSuite1> handleSuite = AEFX_SuiteScoper<PF_HandleSuite1>(	in_dataP,
																					kPFHandleSuite,
																					kPFHandleSuiteVersion1,
																					out_dataP);

	PF_Handle infoH	= handleSuite->host_new_handle(sizeof(ImpInfo));
	
	if (infoH){

		ImpInfo *infoP = reinterpret_cast<ImpInfo*>(handleSuite->host_lock_handle(infoH));
		
		if (infoP){

			extraP->output->pre_render_data = infoH;
			
			ERR(PF_CHECKOUT_PARAM(	in_dataP, 
									IMP_IMPACT_FRAME, 
									in_dataP->current_time, 
									in_dataP->time_step, 
									in_dataP->time_scale, 
									&impact_param));
			
			ERR(PF_CHECKOUT_PARAM(	in_dataP, 
									IMP_HOLD, 
									in_dataP->current_time, 
									in_dataP->time_step, 
									in_dataP->time_scale, 
									&hold_param));
			
			ERR(PF_CHECKOUT_PARAM(	in_dataP, 
									IMP_RADIUS, 
									in_dataP->current_time, 
									in_dataP->time_step, 
									in_dataP->time_scale, 
									&radius_param));
			
			ERR(PF_CHECKOUT_PARAM(	in_dataP, 
									IMP_EDGE_TYPE, 
									in_dataP->current_time, 
									in_dataP->time_step, 
									in_dataP->time_scale, 
									&edge_param));
			
			if (!err){
				infoP->radiusF = radius_param.u.fs_d.value;
				infoP->edgeTypeL = edge_param.u.pd.value;
				infoP->widthL = in_dataP->width;
				infoP->heightL = in_dataP->height;
				infoP->holdActiveB = hold_param.u.bd.value;
				infoP->impactFilePath[0] = '\0';
				
				if (impact_param.u.bd.value) {
					infoP->impactFrameL = in_dataP->current_time;
				} else {
					infoP->impactFrameL = 0;
				}
			}
			
			ERR(extraP->cb->checkout_layer(	in_dataP->effect_ref,
											IMP_INPUT,
											IMP_INPUT,
											&req,
											in_dataP->current_time,
											in_dataP->time_step,
											in_dataP->time_scale,
											&in_result));
			
			UnionLRect(&in_result.result_rect, 		&extraP->output->result_rect);
			UnionLRect(&in_result.max_result_rect, 	&extraP->output->max_result_rect);		
			handleSuite->host_unlock_handle(infoH);
		}
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
		
		infoP->impactFilePath[0] = '\0';
		
		if (infoP->impactFrameL > 0) {
			if (infoP->holdActiveB) {
			} else {
				SaveFrameToTempFile(in_data, input_worldP, infoP->widthL, infoP->heightL, infoP->impactFilePath);
			}
		}
		
		if (infoP->radiusF == 0.0) {
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
