/*
	Copyright (C) 2007-2010 Victor Martins. 
	All rights reserved.
 
	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from the 
	use of this software.
	
	You may use this file in accordance with the terms contained in an agreement
	between you and the copyright holder(s).
	
	For more information, please visit http://www.pixelnerve.com .
*/


#include "ShaderSemantics.h"



namespace V
{
	CGGLenum shaderMatrixMap[] = 
	{
		CG_GL_MODELVIEW_MATRIX,
		CG_GL_PROJECTION_MATRIX,
		CG_GL_TEXTURE_MATRIX,
		CG_GL_MODELVIEW_PROJECTION_MATRIX
	};

	CGGLenum shaderTransformMap[] = 
	{
		CG_GL_MATRIX_IDENTITY,
		CG_GL_MATRIX_INVERSE,
		CG_GL_MATRIX_TRANSPOSE,
		CG_GL_MATRIX_INVERSE_TRANSPOSE
	};
}