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


#include <string>
#include <sstream>
#include "ShaderCGFX.h"


namespace V
{
	void LogError( std::stringstream& ss )
	{
		OutputDebugStringA( ss.str().c_str() );
	}



	void checkCgError()
	{
		CGerror error = cgGetError();
		//std::stringstream ss;
		//ss << "[ShaderCGFX]  : " << cgGetErrorString(error);
		//LogError( ss );

		if( error != CG_NO_ERROR ) 
		{
			std::stringstream str;
			str << "CG error: " << cgGetErrorString(error) << std::endl;;
			std::stringstream strListing;
			//strListing << "CG error: " << cgGetLastListing(mObj->_context);
			LogError( str );
			//LogError( strListing );
			throw new std::exception( strListing.str().c_str() );
		}
	} 




	ShaderCGFX::Obj::Obj( bool aOwnsData ) 
		: mOwnsData( aOwnsData ) 
	{
		_effect = NULL;
		_currTechnique = NULL;
		_currPass = NULL;
	}

	ShaderCGFX::Obj::~Obj()
	{
		//if( mOwnsData )
		{
			if( _effect != NULL ) 
			{
				cgDestroyEffect( _effect );
				_effect = NULL;
			}

			_currTechnique = NULL;
			_currPass = NULL;
		}
	}


	ShaderCGFX::ShaderCGFX( ) 
		: mObj( new Obj( true ) )
	{
		//_type = CGFX;
		_name = "null";
	}


	ShaderCGFX::ShaderCGFX( CGcontext context, const std::string& filename )
		: mObj( new Obj( true ) )
	{
		//_type = CGFX;
		_name = filename;

		this->load( context, filename );
	}


	ShaderCGFX::~ShaderCGFX()
	{
		release();
	}


	void ShaderCGFX::release()
	{
		if( mObj )
		{
			delete mObj;
			mObj = NULL;
		}
	}

	void ShaderCGFX::checkCgError() 
	{
		CGerror error = cgGetError();
		//std::stringstream ss;
		//ss << "[ShaderCGFX]  '" << _name << "' : " << cgGetErrorString(error);
		//LogError( ss );

		if( error != CG_NO_ERROR ) 
		{
			std::stringstream str;
			str << "'" << _name << "'   CG error: " << cgGetErrorString(error) << std::endl;;
			//std::stringstream strListing;
			//strListing << "CG error: " << cgGetLastListing(mObj->_context);
			LogError( str );
			//LogError( strListing );
			throw new std::exception( str.str().c_str() );
		}
	 } 



	CGeffect ShaderCGFX::getEffect()
	{
		return mObj->_effect;
	}




	bool ShaderCGFX::load( CGcontext context, const std::string& filename )
	{
		bool result = false;
		try 
		{
			_name = filename;

			std::string path = filename;

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
			char AppAbsPath[1024];
			path = "/" + path;
			path = _getcwd( AppAbsPath, 1024 ) + path;
			DEBUG_MESSAGE( path.c_str() );
#else
			char AppAbsPath[1024];
			path = "/" + path;
			path = getcwd( AppAbsPath, 1024 ) + path;
			DEBUG_MESSAGE( path.c_str() );
#endif

			mObj->_effect = cgCreateEffectFromFile( context, path.c_str(), NULL ); 
			if( !mObj->_effect ) return false;

			this->checkCgError();	    

			// Take first technique and pass and save it
			mObj->_currTechnique = cgGetFirstTechnique( mObj->_effect );    
			mObj->_currPass = cgGetFirstPass( mObj->_currTechnique );

    		CGbool isValidated = cgValidateTechnique( mObj->_currTechnique );
			if( isValidated ) result = true;
			else result = false;


			/*
			Get list of techniques/passes from an fx effect and save programs if needed

			http://developer.nvidia.com/forums/index.php?showtopic=2091&mode=linear
			You can get the compiled program string with cgGetProgramString( program, CG_COMPILED_PROGRAM );
			To find the programs within an effect, use the cgGetFirstXYZ & cgGetNextXYZ to cycle through the objects in your effect. Alternatively you can use the cgGetNamedXYZ functions when you know the names of the technique/pass/etc. that you are interested in.
			Once you have a pass handle use cgGetPassProgram to get the program that you are interested in. This API is new to Cg 2.1. If you have to use an older version of Cg, then cycle through the state assignments of the pass looking for those of CG_PROGRAM_TYPE type, then check that the programs domain matches the one you are interested in. 
			*/

			// http://www.devmaster.net/forums/showthread.php?t=10074
			// Iterate techniques in effect-file
			CGtechnique technique = cgGetFirstTechnique( mObj->_effect );
			while( technique != 0 )
			{
				CGstateassignment stateAssignment = NULL;
				// Walk passes
				CGpass pass = cgGetFirstPass( technique );
				while( pass != 0 )
				{
					// Find the vertex program state
					stateAssignment = cgGetNamedStateAssignment( pass, "VS" );
					if( stateAssignment != 0 )
					{
						// Get the program
						CGprogram program = cgGetProgramStateAssignmentValue( stateAssignment );
						if( program != 0 )
						{
							// Go nuts :)
							//OutputDebugStringA( V::String::Format("Program: ").str() );
						}
					}

					// Find the fragment program state
					stateAssignment = cgGetNamedStateAssignment( pass, "PS" );
					if( stateAssignment != 0 )
					{
						// Get the program
						CGprogram program = cgGetProgramStateAssignmentValue( stateAssignment );
						if( program != 0 )
						{
							// Go nuts :)
							//OutputDebugStringA( V::String::Format("Program: ").str() );
						}
					}

					// Proceed to the next pass
					pass = cgGetNextPass( pass );
				}

				// Proceed to next technique
				technique = cgGetNextTechnique( technique );
			}


		} catch( std::exception e )
		{
			//std::stringstream ss;
			//ss << "ShaderCGFX:: Failed to load filename: " << file.c_str() << std::endl;
			//throw std::exception( ss.str().c_str() );
		}

		return result;
	}



	void ShaderCGFX::enable()
	{
		// empty
	}



	void ShaderCGFX::disable()
	{
		// empty
	}



	void ShaderCGFX::setFirstTechnique()
	{
		mObj->_currTechnique = cgGetFirstTechnique( mObj->_effect );    
		//if( mObj->_currTechnique == NULL ) System.err.println( "(ShaderCGFX)  technique is NULL" );

		// Get technique's first pass
		mObj->_currPass = cgGetFirstPass( mObj->_currTechnique );
	}



	void ShaderCGFX::setTechnique( const std::string& name )
	{
		mObj->_currTechnique = cgGetNamedTechnique( mObj->_effect, name.c_str() );    
		//if( mObj->_currTechnique == NULL ) System.err.println( "(ShaderCGFX)  technique '" + name + "' is NULL" );

		// Get technique's first pass
		mObj->_currPass = cgGetFirstPass( mObj->_currTechnique );
	}



	/**
		This method differs from the other techniques setters as it sets the first pass for the technique with the given name
	*/
	void ShaderCGFX::SetPassFromTechnique( const std::string& techniqueName )
	{
		mObj->_currTechnique = cgGetNamedTechnique( mObj->_effect, techniqueName.c_str() );    
		//std::stringstream ss0;
		//ss0 << "[ShaderCGFX]  Technique '" << techniqueName.c_str() << "' was not found" << std::endl;
		//if( mObj->_currTechnique == NULL ) throw std::exception( ss0.str().c_str() );

		// Get technique's first pass
		mObj->_currPass = cgGetFirstPass( mObj->_currTechnique );
		//std::stringstream ss1;
		//ss1 << "[ShaderCGFX]  Technique '" << techniqueName << "' does not seem to have any passes" << std::endl;
		//if( mObj->_currPass == NULL ) throw std::exception( ss1.str().c_str() );

		// Set pass
		setPass();
		//checkCgError();
	}


	void ShaderCGFX::setPass()
	{
		cgSetPassState( mObj->_currPass );
		//checkCgError();
	}



	void ShaderCGFX::nextPass()
	{
		mObj->_currPass = cgGetNextPass( mObj->_currPass );
		cgSetPassState( mObj->_currPass );
	}



	void ShaderCGFX::resetPass()
	{
		cgResetPassState( mObj->_currPass ); 
		//checkCgError();
	}


	bool ShaderCGFX::isPassValid()
	{
		if( mObj->_currPass == NULL )
			return false;
		
		return true;
	}


	/**
	 * Reset and get next pass of a technique
	 * Returns null if no next pass available
	 */
	CGpass ShaderCGFX::resetAndGetNextPass()
	{
	    cgResetPassState( mObj->_currPass );
		
		mObj->_currPass = cgGetNextPass( mObj->_currPass );
	    cgSetPassState( mObj->_currPass );
	    
	    return mObj->_currPass;
		
	}


	CGpass ShaderCGFX::getFirstTechniquePass()
	{
		mObj->_currTechnique = cgGetFirstTechnique( mObj->_effect );    
		//if( mObj->_currTechnique == NULL ) System.err.println( "(ShaderCGFX)  first technique is NULL" );

		// Get technique's first pass
		mObj->_currPass = cgGetFirstPass( mObj->_currTechnique );

		return mObj->_currPass;
	} 



	CGpass ShaderCGFX::getTechniqueFirstPass( const std::string& name )
	{
		mObj->_currTechnique = cgGetNamedTechnique( mObj->_effect, name.c_str() );    
		if( mObj->_currTechnique == NULL )
		{
			return NULL;
		}

		// Get technique's first pass
		mObj->_currPass = cgGetFirstPass( mObj->_currTechnique );

		return mObj->_currPass;
	} 



	int32_t ShaderCGFX::getArrayDimension( const std::string& param )
	{
		CGparameter p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		return cgGetArrayDimension( p );
	}


	void ShaderCGFX::setTextureParameter( const std::string& param, boost::int32_t val )
	{
		  CGparameter p = NULL;
		  p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		  cgGLSetTextureParameter( p, val );
		  cgSetSamplerState( p );
	}

	void ShaderCGFX::setParameter1d( const std::string& param, double x )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter1d( p, x );
	}

	void ShaderCGFX::setParameter1f( const std::string& param, float x )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter1f( p, x );
	}

	void ShaderCGFX::setParameter1fv( const std::string& param, float* v )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgGLSetParameterArray1f( p, 0, sizeof(v), v );
	}

	void ShaderCGFX::setParameter1i( const std::string& param, int x )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter1i( p, x );
	}

	void ShaderCGFX::setParameter2f( const std::string& param, float x, float y )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter2f( p, x, y );
	}

	void ShaderCGFX::setParameter2fv( const std::string& param, float* v )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgGLSetParameterArray2f( p, 0, sizeof(v), v );
	}


	void ShaderCGFX::setParameter3f( const std::string& param, float x, float y, float z )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter3f( p, x, y, z );
	}

	void ShaderCGFX::setParameter3fv( const std::string& param, float* v )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgGLSetParameterArray3f( p, 0, sizeof(v), v );
	}

	void ShaderCGFX::setParameter4f( const std::string& param, float x, float y, float z, float w )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter4f( p, x, y, z, w );
	}

	void ShaderCGFX::setParameter4fv( const std::string& param, float* v )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgGLSetParameterArray4f( p, 0, sizeof(v), v );
		//cgGLSetParameterArray4f( p, 0, sizeof(v), v );
	}

	void ShaderCGFX::setMatrixParameterSemantic( const std::string& param, int matrixType_, int transformType_ )
	{
		CGGLenum matrix = shaderMatrixMap[matrixType_];
		CGGLenum transform = shaderTransformMap[transformType_];
		CGparameter p = cgGetEffectParameterBySemantic( mObj->_effect, param.c_str() );
		cgGLSetStateMatrixParameter( p, matrix, transform );
	}

	void ShaderCGFX::setMatrixParameterSemantic( const std::string& param, float* v )
	{
		CGparameter p = NULL;
		p = cgGetEffectParameterBySemantic( mObj->_effect, param.c_str() );
		//p = cgGetNamedEffectParameter( mObj->_effect, param );
		cgGLSetMatrixParameterfr( p, v );
	}

	void ShaderCGFX::setParameterSemantic( const std::string& param, float x )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter1f( p, x );
	}
	void ShaderCGFX::setParameterSemantic( const std::string& param, float x, float y )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter2f( p, x, y );
	}
	void ShaderCGFX::setParameterSemantic( const std::string& param, float x, float y, float z )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter3f( p, x, y, z );
	}
	void ShaderCGFX::setParameterSemantic( const std::string& param, float x, float y, float z, float w )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter4f( p, x, y, z, w );
	}

	void ShaderCGFX::setParameter4x4d( const std::string& param, double* v )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetMatrixParameterdr( p, v );
	}

	void ShaderCGFX::setParameter4x4f( const std::string& param, float* v )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgGLSetMatrixParameterfr( p, v );
	}

	void ShaderCGFX::setParameter4x4f( const std::string& param, int matrixType_, int transformType_ )
	{
		CGGLenum matrix = V::shaderMatrixMap[matrixType_];
		CGGLenum transform = V::shaderTransformMap[transformType_];
		CGparameter p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgGLSetStateMatrixParameter( p, matrix, transform );
	}

	void ShaderCGFX::setParameter4x4fBySemantic( const std::string& semanticName, int matrixType_, int transformType_ )
	{
		CGGLenum matrix = V::shaderMatrixMap[matrixType_];
		CGGLenum transform = V::shaderTransformMap[transformType_];
		CGparameter p = cgGetEffectParameterBySemantic( mObj->_effect, semanticName.c_str() );
		cgGLSetStateMatrixParameter( p, matrix, transform );
	}


	CGFXManager::CGFXManager()
	{
	}

	CGFXManager::~CGFXManager()
	{
		cgDestroyContext( mContext );
		mContext = NULL;

		// release
		//for( CGFXEffectList::iterator it = mEffectList.begin(); it != mEffectList.end(); it++ )
		//{
		//	ShaderCGFX fx = (*it);
		//}
	}


	void CGFXManager::init()
	{
		mContext = cgCreateContext();
		checkCgError();
		cgGLRegisterStates( mContext );
		cgSetParameterSettingMode( mContext, CG_DEFERRED_PARAMETER_SETTING );
		cgGLSetManageTextureParameters( mContext, true );
	}

	V::ShaderCGFXRef CGFXManager::loadEffectFromFile( std::string filename )
	{
		ShaderCGFXRef fx = ShaderCGFXRef( new ShaderCGFX() );
		if( !fx->load( mContext, filename ) )
		{
			std::stringstream ss;
			ss << "[CGFXManager]  Failed to load effect: '" << filename << "'" << std::endl;
			LogError( ss );
			return ShaderCGFXRef();
		}

		mEffectList.push_back( std::make_pair(mEffectList.size(), fx) );
		mEffectMap[filename] = fx;
		return fx;
	}

	V::ShaderCGFXRef CGFXManager::getEffect( std::string filename )
	{
		CGFXEffectMap::iterator it;
		it = mEffectMap.find( filename );
		return (*it).second;
		//for( CGFXEffectList::iterator it = mEffectList.begin(); it != mEffectList.end(); it++ )
		//{
		//	if( (*it).second.getName() == filename )
		//		return (*it).second;
		//}

		std::stringstream ss;
		ss << "[CGFXManager]  Effect not found! ''" << filename << "'" << std::endl;;
		LogError( ss );
		return ShaderCGFXRef();
	}
}