/*******************************************************************/
/*                                                                 */
/*                      LUNAR --FX PLUGIN                          */
/*                   LUNAR.SNITCH Effect                           */
/*                                                                 */
/* Copyright 2026 LUNAR --FX. All Rights Reserved.                 */
/*                                                                 */
/* Let your edit shine like a moon                                 */
/*                                                                 */
/*******************************************************************/

#include "LUNAR_SNITCH.h"

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

	// For Premiere - declare supported pixel formats
	if (in_dataP->appl_id == 'PrMr'){

		AEFX_SuiteScoper<PF_PixelFormatSuite1> pixelFormatSuite = 
			AEFX_SuiteScoper<PF_PixelFormatSuite1>(	in_dataP,
													kPFPixelFormatSuite,
													kPFPixelFormatSuiteVersion1,
													out_data);

		//	Add the pixel formats we support in order of preference.
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
	
	// Main intensity slider - controls everything
	PF_ADD_FLOAT_SLIDERX("Intensity", 
						INTENSITY_MIN,
						INTENSITY_MAX,
						INTENSITY_MIN,
						INTENSITY_MAX,
						INTENSITY_DEFAULT,
						SLIDER_PRECISION,
						DISPLAY_FLAGS,
						0,
						INTENSITY_DISK_ID);
	
	out_data->num_params = SNITCH_NUM_PARAMS;

	return err;
}

// Helper function to clamp values
template<typename T>
inline T clamp(T value, T min, T max) {
	return (value < min) ? min : (value > max) ? max : value;
}

// Apply anime enhancement to a single pixel
static void ApplySnitchEffect8(PF_Pixel8 *inP, PF_Pixel8 *outP, SnitchInfo *siP, A_long x, A_long y)
{
	if (!siP || !inP || !outP) return;
	
	PF_FpLong intensity = siP->intensityF / 100.0;
	
	// Calculate derived values from intensity
	PF_FpLong chromatic = intensity * 0.3;  // Chromatic scales with intensity
	PF_FpLong contrast = 1.0 + (intensity * 0.4);  // Contrast boost
	PF_FpLong glow = intensity * 0.6;  // Glow scales with intensity
	
	// Start with original pixel
	outP->alpha = inP->alpha;
	outP->red = inP->red;
	outP->green = inP->green;
	outP->blue = inP->blue;
	
	if (intensity <= 0.0) return;
	
	// 1. Apply contrast enhancement
	PF_FpLong redF = (outP->red / 255.0 - 0.5) * contrast + 0.5;
	PF_FpLong greenF = (outP->green / 255.0 - 0.5) * contrast + 0.5;
	PF_FpLong blueF = (outP->blue / 255.0 - 0.5) * contrast + 0.5;
	
	// 2. Apply subtle chromatic aberration based on position
	A_long centerX = siP->widthL / 2;
	A_long centerY = siP->heightL / 2;
	PF_FpLong distX = (x - centerX) / (PF_FpLong)centerX;
	PF_FpLong distY = (y - centerY) / (PF_FpLong)centerY;
	PF_FpLong distance = sqrt(distX * distX + distY * distY);
	
	PF_FpLong chromaticOffset = chromatic * distance * 2.0;
	redF = clamp(redF + chromaticOffset, 0.0, 1.0);
	blueF = clamp(blueF - chromaticOffset * 0.5, 0.0, 1.0);
	
	// 3. Apply anime-style color enhancement (boost reds and blues slightly)
	if (redF > 0.6) {
		redF = clamp(redF * (1.0 + 0.1 * intensity), 0.0, 1.0);
	}
	if (blueF > 0.5) {
		blueF = clamp(blueF * (1.0 + 0.05 * intensity), 0.0, 1.0);
	}
	
	// 4. Apply subtle glow (brighten mid-tones)
	PF_FpLong luminance = (redF * 0.299 + greenF * 0.587 + blueF * 0.114);
	if (luminance > 0.2 && luminance < 0.8) {
		PF_FpLong glowFactor = 1.0 + (glow * 0.2);
		redF = clamp(redF * glowFactor, 0.0, 1.0);
		greenF = clamp(greenF * glowFactor, 0.0, 1.0);
		blueF = clamp(blueF * glowFactor, 0.0, 1.0);
	}
	
	// Convert back to 8-bit
	outP->red = (A_u_char)clamp(redF * 255.0, 0.0, 255.0);
	outP->green = (A_u_char)clamp(greenF * 255.0, 0.0, 255.0);
	outP->blue = (A_u_char)clamp(blueF * 255.0, 0.0, 255.0);
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
	
	SnitchInfo *	siP		= reinterpret_cast<SnitchInfo*>(refcon);
					
	if (siP){
		ApplySnitchEffect8(inP, outP, siP, xL, yL);
	}
	return err;
}

static PF_Err 
FilterImageBGRA_8u (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err			err = PF_Err_NONE;

	PF_Pixel_BGRA_8u *inBGRA_8uP, *outBGRA_8uP;
	inBGRA_8uP = reinterpret_cast<PF_Pixel_BGRA_8u*>(inP);
	outBGRA_8uP = reinterpret_cast<PF_Pixel_BGRA_8u*>(outP);

	SnitchInfo *	siP		= reinterpret_cast<SnitchInfo*>(refcon);
					
	if (siP){
		// Convert to standard PF_Pixel8 for processing
		PF_Pixel8 tempIn, tempOut;
		tempIn.alpha = inBGRA_8uP->alpha;
		tempIn.red = inBGRA_8uP->red;
		tempIn.green = inBGRA_8uP->green;
		tempIn.blue = inBGRA_8uP->blue;
		
		ApplySnitchEffect8(&tempIn, &tempOut, siP, xL, yL);
		
		outBGRA_8uP->alpha = tempOut.alpha;
		outBGRA_8uP->red = tempOut.red;
		outBGRA_8uP->green = tempOut.green;
		outBGRA_8uP->blue = tempOut.blue;
	}
	return err;
}

static PF_Err 
FilterImageVUYA_8u (
	void		*refcon,
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err			err = PF_Err_NONE;

	PF_Pixel_VUYA_8u *inVUYA_8uP, *outVUYA_8uP;
	inVUYA_8uP = reinterpret_cast<PF_Pixel_VUYA_8u*>(inP);
	outVUYA_8uP = reinterpret_cast<PF_Pixel_VUYA_8u*>(outP);

	SnitchInfo *	siP		= reinterpret_cast<SnitchInfo*>(refcon);
					
	if (siP){
		// For YUV, we'll primarily work on luma and preserve color
		outVUYA_8uP->alpha = inVUYA_8uP->alpha;
		outVUYA_8uP->Pb = inVUYA_8uP->Pb;
		outVUYA_8uP->Pr = inVUYA_8uP->Pr;
		
		// Apply contrast and glow to luma channel
		PF_FpLong intensity = siP->intensityF / 100.0;
		PF_FpLong contrast = 1.0 + (intensity * 0.4);
		PF_FpLong glow = intensity * 0.6;
		
		if (intensity > 0.0) {
			PF_FpLong lumaF = inVUYA_8uP->luma / 255.0;
			
			// Apply contrast
			lumaF = (lumaF - 0.5) * contrast + 0.5;
			
			// Apply glow to mid-tones
			if (lumaF > 0.2 && lumaF < 0.8) {
				PF_FpLong glowFactor = 1.0 + (glow * intensity * 0.2);
				lumaF = clamp(lumaF * glowFactor, 0.0, 1.0);
			}
			
			outVUYA_8uP->luma = (A_u_char)clamp(lumaF * 255.0, 0.0, 255.0);
		} else {
			outVUYA_8uP->luma = inVUYA_8uP->luma;
		}
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

	SnitchInfo			siP;
	A_long				linesL	= 0;
	
	AEFX_CLR_STRUCT(siP);
	
	linesL 		= output->extent_hint.bottom - output->extent_hint.top;
	siP.intensityF = params[SNITCH_INTENSITY]->u.fs_d.value;
	siP.widthL = in_dataP->width;
	siP.heightL = in_dataP->height;

	// Do high-bit depth rendering in Premiere Pro
	if (in_dataP->appl_id == 'PrMr') {

		// Get the Premiere pixel format suite
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
									0,								// progress base
									linesL,							// progress final
									&params[SNITCH_INPUT]->u.ld,		// src 
									NULL,							// area - null for all pixels
									(void*)&siP,					// refcon - your custom data pointer
									FilterImageBGRA_8u,				// pixel function pointer
									output);	

		} else if (destinationPixelFormat == PrPixelFormat_VUYA_4444_8u){

			AEFX_SuiteScoper<PF_Iterate8Suite1> iterate8Suite = 
				AEFX_SuiteScoper<PF_Iterate8Suite1>(in_dataP,
													kPFIterate8Suite,
													kPFIterate8SuiteVersion1,
													out_data);

			iterate8Suite->iterate(	in_dataP,
									0,								// progress base
									linesL,							// progress final
									&params[SNITCH_INPUT]->u.ld,		// src 
									NULL,							// area - null for all pixels
									(void*)&siP,					// refcon - your custom data pointer
									FilterImageVUYA_8u,				// pixel function pointer
									output);

		} else {
			//	Return error, because we don't know how to handle the specified pixel type
			return PF_Err_UNRECOGNIZED_PARAM_TYPE;
		}
		
	} else if(siP.intensityF > 0.0) {

		AEFX_SuiteScoper<PF_Iterate8Suite1> iterate8Suite = 
			AEFX_SuiteScoper<PF_Iterate8Suite1>(in_dataP,
												kPFIterate8Suite,
												kPFIterate8SuiteVersion1,
												out_data);

		iterate8Suite->iterate(	in_dataP,
								0,								// progress base
								linesL,							// progress final
								&params[SNITCH_INPUT]->u.ld,		// src 
								NULL,							// area - null for all pixels
								(void*)&siP,					// refcon - your custom data pointer
								FilterImage8,					// pixel function pointer
								output);						// dest
	} else {

		AEFX_SuiteScoper<PF_WorldTransformSuite1> worldTransformSuite = 
			AEFX_SuiteScoper<PF_WorldTransformSuite1>(	in_dataP,
														kPFWorldTransformSuite,
														kPFWorldTransformSuiteVersion1,
														out_data);

		worldTransformSuite->copy(	in_dataP->effect_ref,			// This effect ref (unique id)
									&params[SNITCH_INPUT]->u.ld,		// Source
									output,							// Dest
									NULL,							// Source rect - null for all pixels
									NULL);							// Dest rect - null for all pixels
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
	PF_ParamDef intensity_param;
	PF_RenderRequest req = extraP->input->output_request;
	PF_CheckoutResult in_result;
	
	AEFX_CLR_STRUCT(intensity_param);

	AEFX_SuiteScoper<PF_HandleSuite1> handleSuite = AEFX_SuiteScoper<PF_HandleSuite1>(	in_dataP,
																						kPFHandleSuite,
																						kPFHandleSuiteVersion1,
																						out_dataP);

	PF_Handle infoH	= handleSuite->host_new_handle(sizeof(SnitchInfo));
	
	if (infoH){

		SnitchInfo *infoP = reinterpret_cast<SnitchInfo*>(handleSuite->host_lock_handle(infoH));
		
		if (infoP){

			extraP->output->pre_render_data = infoH;
			
			ERR(PF_CHECKOUT_PARAM(	in_dataP, 
									SNITCH_INTENSITY, 
									in_dataP->current_time, 
									in_dataP->time_step, 
									in_dataP->time_scale, 
									&intensity_param));
			
			if (!err){
				infoP->intensityF = intensity_param.u.fs_d.value;
				infoP->widthL = in_dataP->width;
				infoP->heightL = in_dataP->height;
			}
			
			ERR(extraP->cb->checkout_layer(	in_dataP->effect_ref,
											SNITCH_INPUT,
											SNITCH_INPUT,
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
	
	SnitchInfo	*infoP = reinterpret_cast<SnitchInfo*>(handleSuite->host_lock_handle(reinterpret_cast<PF_Handle>(extraP->input->pre_render_data)));
	
	if (infoP){
		ERR((extraP->cb->checkout_layer_pixels(	in_data->effect_ref, SNITCH_INPUT, &input_worldP)));
		ERR(extraP->cb->checkout_output(in_data->effect_ref, &output_worldP));
		
		PF_PixelFormat		format	=	PF_PixelFormat_INVALID;
		
		AEFX_SuiteScoper<PF_WorldSuite2> wsP = AEFX_SuiteScoper<PF_WorldSuite2>(in_data,
																				kPFWorldSuite,
																				kPFWorldSuiteVersion2,
																				out_data);
		
		if (infoP->intensityF == 0.0) {
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
		
						// TODO: Implement 32f processing
						err = PF_COPY(input_worldP, output_worldP, NULL, NULL);
						break;
						
					case PF_PixelFormat_ARGB64:
										
						// TODO: Implement 16-bit processing
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
		ERR2(extraP->cb->checkin_layer_pixels(in_data->effect_ref, SNITCH_INPUT));
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
		"LUNAR.SNITCH", // Name
		"LUNAR SNITCH", // Match Name
		"LUNAR --FX", // Category
		AE_RESERVED_INFO); // Reserved Info

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
		// Never EVER throw exceptions into AE.
		err = thrown_err;
	}
	return err;
}
