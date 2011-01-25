#pragma once

#include "CG/cg.h"
#include "CG/cgGL.h"




namespace V
{
	//
	// Matrices type
	//
	enum ShaderMatrixTypeSemantics
	{
		WORLD_MATRIX = 0,
		VIEW_MATRIX = 0,
		PROJECTION_MATRIX = 1,
		TEXTURE_MATRIX = 2,
		WORLDVIEWPROJECTION_MATRIX = 3,
		VIEWPROJECTION_MATRIX = 3
	};


	//
	// Transform type
	enum ShaderTransformTypeSemantics
	{
		IDENTITY_MATRIX = 0,
		INVERSE_MATRIX = 1,
		TRANSPOSE_MATRIX = 2,
		INVERSE_TRANSPOSE_MATRIX = 3
	};


	CGGLenum shaderMatrixMap[];
	CGGLenum shaderTransformMap[];

}	// V
