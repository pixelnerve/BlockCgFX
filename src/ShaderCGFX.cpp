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

#include <exception>
#include <cassert>
#include <string>
#include <sstream>
#include "ShaderCGFX.h"


// Make sure we link libraries
//#pragma comment( lib, "CgGL.lib" )
//#pragma comment( lib, "Cg.lib" )


namespace V
{
    class CgfxException: public std::exception
    {
    public:
        CgfxException( const char* msg ) { _msg = (char*)msg; }
        virtual const char* what() const throw() { return _msg; }
    private:
        char* _msg;
    };
    
    
    
	void LogError( std::stringstream& ss )
	{
#ifdef _WIN32
		OutputDebugStringA( ss.str().c_str() );
#else
        printf( "%s", ss.str().c_str() );
#endif
	}


	static void checkForCgError( CGcontext context, const char *situation )
	{
		CGerror error;
		const char *string = cgGetLastErrorString( &error );

		std::stringstream strr;
		strr << situation << std::endl;
		LogError( strr );

		if (error != CG_NO_ERROR) 
		{
			std::stringstream str;
			str << "  -- CG error: " << string << std::endl;
			LogError( str );
			//if (error == CG_COMPILER_ERROR) 
			{
				std::stringstream strListing;
				strListing << (char*)cgGetLastListing( context ) << std::endl;
				LogError( strListing );
			}
		}
		else
			OutputDebugStringA( "CgFX NO ERROR\n" );
	}

	void CHECK_CG_ERROR( CGcontext context )
	{
		CGerror error = cgGetError();
		std::stringstream ss;
		ss << "[ShaderCGFX]  : " << cgGetErrorString(error) << std::endl;
		LogError( ss );

		if( error != CG_NO_ERROR ) 
		{
			std::stringstream strListing;
			strListing << "CG error: " << cgGetLastListing(context) << std::endl;
			LogError( strListing );
			throw new CgfxException( (char*)strListing.str().c_str() );
		}
	} 




	ShaderCGFX::Obj::Obj( bool aOwnsData ) 
		: mOwnsData( aOwnsData ) 
	{
		_effect = NULL;
		_currTechnique = NULL;
		_currPass = NULL;
		mIsLoaded = false;
		mId = -1;
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
		_context = NULL;
		//_type = CGFX;
		_name = "null";
	}


	ShaderCGFX::ShaderCGFX( CGcontext context, const std::string& filename )
		: mObj( new Obj( true ) )
	{
		//_type = CGFX;
		_name = filename;
		_context = context;	// pointer to

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

	//void ShaderCGFX::checkCgError() 
	//{
	//	CGerror error = cgGetError();
	//	std::stringstream ss;
	//	ss << "[ShaderCGFX]  '" << _name << "' : " << cgGetErrorString(error) << std::endl;
	//	LogError( ss );

	//	if( error != CG_NO_ERROR ) 
	//	{
	//		std::stringstream strListing;
	//		strListing << "CG error: " << cgGetLastListing(_context) << std::endl;
	//		LogError( strListing );
	//		throw new std::exception( strListing.str().c_str() );
	//	}
	// } 



	CGeffect ShaderCGFX::getEffect()
	{
		return mObj->_effect;
	}




	bool ShaderCGFX::load( CGcontext context, const std::string& filename )
	{
		bool result = false;
		try 
		{
			if( !_context ) 
				_context = context;
			_name = filename;

			std::string path = filename;

			std::stringstream ssname;
			ssname << "+++ Loading effect filename: " << filename << std::endl;
			DEBUG_MESSAGE( ssname.str().c_str() );

/*
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
			char AppAbsPath[1024];
			path = "/" + path;
			path = _getcwd( AppAbsPath, 1024 ) + path;
			//DEBUG_MESSAGE( path.c_str() );
#else
			char AppAbsPath[1024];
			path = "/" + path;
			path = getcwd( AppAbsPath, 1024 ) + path;
			//DEBUG_MESSAGE( path.c_str() );
#endif
*/
			mObj->_effect = cgCreateEffectFromFile( context, path.c_str(), NULL ); 
			checkForCgError( context, "load effect file" );
			//CHECK_CG_ERROR( context );
			//if( !mObj->_effect ) 
				//return false;

			// Take first technique and pass and save it
			mObj->_currTechnique = cgGetFirstTechnique( mObj->_effect );    
			mObj->_currPass = cgGetFirstPass( mObj->_currTechnique );

			// Validate
    		CGbool isValidated = cgValidateTechnique( mObj->_currTechnique );
			//OutputDebugStringA( cgGetTechniqueName( mObj->_currTechnique ) );
			//checkForCgError( context, "validate technique" );
			if( isValidated == CG_TRUE ) 
				result = true;
			else
			{
				DEBUG_MESSAGE( "*** Failed to validate technique. Check if profile is a valid one\n" );
				result = false;
			}


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


			// OK! This effect is ready to use
			if( result ) mObj->mIsLoaded = true;

		} catch( std::exception& e )
		{
			std::stringstream ss;
			ss << e.what() << "\n[ShaderCGFX]  Failed to load filename: " << filename.c_str() << std::endl;
			throw CgfxException( (char*)ss.str().c_str() );
		}

		return true;
	}



	bool ShaderCGFX::reload()
	{
		mObj->mIsLoaded = false;

		if( mObj->_effect )
		{
			cgDestroyEffect( mObj->_effect );
			mObj->_effect = NULL;
		}
		return load( _context, _name );
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
//		checkForCgError( _context, "setTechnique" );
		//if( mObj->_currTechnique == NULL ) System.err.println( "(ShaderCGFX)  technique '" + name + "' is NULL" );

		// Get technique's first pass
		mObj->_currPass = cgGetFirstPass( mObj->_currTechnique );
//		checkForCgError( _context, "setTechnique" );
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
//		checkForCgError( _context, "setPass" );
	}



	void ShaderCGFX::nextPass()
	{
		mObj->_currPass = cgGetNextPass( mObj->_currPass );
		cgSetPassState( mObj->_currPass );
	}



	void ShaderCGFX::resetPass()
	{
		cgResetPassState( mObj->_currPass ); 
//		checkForCgError( _context, "resetPass" );
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



	int ShaderCGFX::getArrayDimension( const std::string& param )
	{
		CGparameter p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		return cgGetArrayDimension( p );
	}


	void ShaderCGFX::setTextureParameter( const std::string& param, int val )
	{
		  CGparameter p = NULL;
		  p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		  assert( p != NULL );
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

	void ShaderCGFX::setParameter3fv( const std::string& param, const float* v )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		assert( p != NULL );
		cgGLSetParameterArray3f( p, 0, 0, v );
	}

	void ShaderCGFX::setParameter4f( const std::string& param, float x, float y, float z, float w )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		cgSetParameter4f( p, x, y, z, w );
	}

	void ShaderCGFX::setParameter4fv( const std::string& param, const float* v )
	{
		CGparameter p = NULL;
		p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
		assert( p != NULL );
		cgGLSetParameterArray4f( p, 0, 4, v );
	}

	void ShaderCGFX::setMatrixParameterSemantic( const std::string& param, int matrixType_, int transformType_ )
	{
		CGGLenum matrix = shaderMatrixMap[matrixType_];
		CGGLenum transform = shaderTransformMap[transformType_];
		CGparameter p = cgGetEffectParameterBySemantic( mObj->_effect, param.c_str() );
		if( !p ) throw new CgfxException( "Failed on setMatrixParameterSemantic()" );
		//assert( p!=NULL && "Failed on setMatrixParameterSemantic()" );
		cgGLSetStateMatrixParameter( p, matrix, transform );
	}

	void ShaderCGFX::setMatrixParameterSemantic( const std::string& param, float* v )
	{
		CGparameter p = NULL;
		p = cgGetEffectParameterBySemantic( mObj->_effect, param.c_str() );
		if( !p ) throw new CgfxException( "Failed on setMatrixParameterSemantic()" );
		//assert( p!=NULL && "Failed on setMatrixParameterSemantic()" );
		cgGLSetMatrixParameterfr( p, v );
	}

	//void ShaderCGFX::setParameterSemantic( const std::string& param, float x )
	//{
	//	CGparameter p = NULL;
	//	p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
	//	cgSetParameter1f( p, x );
	//}
	//void ShaderCGFX::setParameterSemantic( const std::string& param, float x, float y )
	//{
	//	CGparameter p = NULL;
	//	p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
	//	cgSetParameter2f( p, x, y );
	//}
	//void ShaderCGFX::setParameterSemantic( const std::string& param, float x, float y, float z )
	//{
	//	CGparameter p = NULL;
	//	p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
	//	cgSetParameter3f( p, x, y, z );
	//}
	//void ShaderCGFX::setParameterSemantic( const std::string& param, float x, float y, float z, float w )
	//{
	//	CGparameter p = NULL;
	//	p = cgGetNamedEffectParameter( mObj->_effect, param.c_str() );
	//	cgSetParameter4f( p, x, y, z, w );
	//}

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
		mContext = NULL;
		//init();
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
		CGerror err  = CG_NO_ERROR;

		mContext = cgCreateContext();
		checkForCgError( mContext, "create context" );
		//CHECK_CG_ERROR( mContext );
		cgGLRegisterStates( mContext );
		checkForCgError( mContext, "register states" );
		//cgGLSetDebugMode(CG_FALSE);
		cgSetParameterSettingMode( mContext, CG_DEFERRED_PARAMETER_SETTING );
		cgGLSetManageTextureParameters( mContext, true );
		checkForCgError( mContext, "manage texture parameters" );
/*
		CGprofile profile;
		int nProfiles = cgGetNumSupportedProfiles();
		std::stringstream ss;
		ss << "NumSupportedProfiles: " << nProfiles << std::endl;
		DEBUG_MESSAGE( ss.str().c_str() );

		DEBUG_MESSAGE( "OpenGL Profiles:\n" );
		for( int i=0; i<nProfiles; ++i ) 
		{
			profile = cgGetSupportedProfile( i );
			if( cgGetProfileProperty(profile, CG_IS_OPENGL_PROFILE) ) 
			{
				ss1 << cgGetProfileString(profile) << " is an OpenGL profile\n" << std::endl;
				DEBUG_MESSAGE( ss1.str().c_str() );
			} 
			else 
			{
				ss1 << cgGetProfileString(profile) << " is not an OpenGL profile\n" << std::endl;
				DEBUG_MESSAGE( ss1.str().c_str() );
			}
		}
*/

		mFXCount = 0;
	}

	ShaderID CGFXManager::createEffectFromFile( const std::string& filename )
	{
		ShaderCGFXRef fx = ShaderCGFXRef( new ShaderCGFX() );
		if( !fx->load( mContext, filename ) )
		{
			std::stringstream ss;
			ss << "[CGFXManager]  Failed to load effect: '" << filename << "'" << std::endl;
			LogError( ss );
			return -1;
		}

		//mEffectList.push_back( std::make_pair(mEffectList.size(), fx) );
		mEffectMap[mFXCount] = fx;
		mFXCount++;
		return mFXCount-1;
	}

	V::ShaderCGFXRef CGFXManager::loadEffectFromFile( const std::string& filename )
	{
		ShaderCGFXRef fx = ShaderCGFXRef( new ShaderCGFX() );
		if( !fx->load( mContext, filename ) )
		{
			std::stringstream ss;
			ss << "[CGFXManager]  Failed to load effect: '" << filename << "'" << std::endl;
			LogError( ss );
			return ShaderCGFXRef();
		}

		//mEffectList.push_back( std::make_pair(mEffectList.size(), fx) );
		mEffectMap[mFXCount] = fx;
		mFXCount++;
		return fx;
	}

	void CGFXManager::reloadEffect( V::ShaderCGFXRef fx )
	{
		fx->reload();
	}

	void CGFXManager::reloadEffect( const ShaderID id )
	{
		CGFXEffectMap::iterator it;
		it = mEffectMap.find( id );
		(*it).second->reload();
	}


	void CGFXManager::reloadAll()
	{
		for( CGFXEffectMap::iterator it = mEffectMap.begin(); it != mEffectMap.end(); ++it )
		{
			(*it).second->reload();
		}
	}

	V::ShaderCGFXRef CGFXManager::getEffect( const int id )
	{
		CGFXEffectMap::iterator it;
		it = mEffectMap.find( id );
		return (*it).second;

		std::stringstream ss;
		ss << "[CGFXManager]  Effect not found! ''" << id << "'" << std::endl;;
		LogError( ss );
		return ShaderCGFXRef();
	}

	V::ShaderCGFXRef CGFXManager::enable( unsigned int id )
	{
		mActiveFX = getEffect( id );
		return mActiveFX;
	}

	void CGFXManager::disable()
	{
	}
}