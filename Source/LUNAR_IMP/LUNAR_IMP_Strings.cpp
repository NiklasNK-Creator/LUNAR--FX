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

typedef struct {
	unsigned long	index;
	char			str[256];
} TableString;

TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,						""
};

char *GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
