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

#pragma once


#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#include <direct.h>
#undef min
#undef max
#define DEBUG_MESSAGE( x ) OutputDebugStringA( x )
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define DEBUG_MESSAGE( x ) printf( x )
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#define DEBUG_MESSAGE( x ) printf( x )
#endif

#include <map>
#include <vector>

#include <boost/cstdint.hpp>

#include "CG/cg.h"
#include "CG/CgGL.h"
#include "ShaderSemantics.h"


// Add libraries programmatically
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#pragma  comment( lib, "Cg.lib" )
#pragma  comment( lib, "CgGL.lib" )
#elif defined(linux) || defined(__linux) || defined(__linux__)
#pragma  comment( lib, "Cg.lib" )
#pragma  comment( lib, "CgGL.lib" )
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#pragma  comment( lib, "Cg.lib" )
#pragma  comment( lib, "CgGL.lib" )
#endif



namespace V
{
	// Implementation based on Cinder object
	class ShaderCGFX
	{
	public:
		typedef std::shared_ptr<ShaderCGFX> Ref;
		typedef std::map<int, std::string> ParamMap;


	private:
		// From cinder cinder 
		struct Obj 
		{
			Obj( bool aOwnsData );
			~Obj();


			// Members
			CGeffect		_effect;

			int32_t			mId;

			int				_NumOfPasses;
			CGtechnique		_currTechnique;
			CGpass			_currPass;
			ParamMap		_params;

			bool			mIsLoaded;
			bool			mOwnsData;
		};


	public:
		//
		// Protected methods
		//
		ShaderCGFX( );
		ShaderCGFX( CGcontext context, const std::string& filename );
		virtual ~ShaderCGFX();

		void release();

		//
		// Public methods
		//
		CGeffect getEffect();
		const std::string& getName()	{ return _name; }

		bool	isLoaded()				{ return mObj->mIsLoaded; }

		bool load( CGcontext context, const std::string& filename );
		bool reload();

		void enable();
		void disable();


		void setFirstTechnique();
		void setTechnique( const std::string& name );

		/**
			This method differs from the other techniques setters as it sets the first pass for the technique with the given name
		*/
		void SetPassFromTechnique( const std::string& techniqueName );

		/**Get first technique from the effect file and return it's first pass
		*/
		CGpass getFirstTechniquePass();

		/**
			Get technique with name 'name' and return first pass
		*/
		CGpass getTechniqueFirstPass( const std::string& name );


		void setPass();
		void nextPass();
		void resetPass();
		CGpass resetAndGetNextPass();

		bool isPassValid();

		int32_t getArrayDimension( const std::string& param );

		void setTextureParameter( const std::string& param, boost::int32_t val );
		void setParameter1d( const std::string& param, double x );
		void setParameter1f( const std::string& param, float x );
		void setParameter1fv( const std::string& param, float* v );
		void setParameter1i( const std::string& param, int x );
		void setParameter2f( const std::string& param, float x, float y );
		void setParameter2fv( const std::string& param, float* v );
		void setParameter3f( const std::string& param, float x, float y, float z );
		void setParameter3fv( const std::string& param, const float* v );
		void setParameter4f( const std::string& param, float x, float y, float z, float w );
		void setParameter4fv( const std::string& param, const float* v );
		void setMatrixParameterSemantic( const std::string& param, int matrixType_, int transformType_ );
		void setMatrixParameterSemantic( const std::string& param, float* v );
		void setParameterSemantic( const std::string& param, float x );
		void setParameterSemantic( const std::string& param, float x, float y );
		void setParameterSemantic( const std::string& param, float x, float y, float z );
		void setParameterSemantic( const std::string& param, float x, float y, float z, float w );
		void setParameter4x4d( const std::string& param, double* v );
		void setParameter4x4f( const std::string& param, float* v );
		void setParameter4x4f( const std::string& param, int matrixType_, int transformType_ );
		void setParameter4x4fBySemantic( const std::string& semanticName, int matrixType_, int transformType_ );

	public:
		// From cinder
		//@{	
		//! Emulates shared_ptr-like behavior
		//typedef std::shared_ptr<Obj> ShaderCGFX::*unspecified_bool_type;
		//operator unspecified_bool_type() const { return ( mObj.get() == 0 ) ? 0 : &ShaderCGFX::mObj; }
		//void reset() { mObj.reset(); }
		//@}


	//
	// Protected methods
	//
	//protected:
		//void checkCgError();

	//
	// Members
	//
	private:
		//std::shared_ptr<Obj>	mObj;
		Obj*					mObj;

	protected:
		CGcontext				_context;
		std::string				_name;
		int32_t					_type;

	}; // end class
	typedef std::shared_ptr<ShaderCGFX> ShaderCGFXRef;



	typedef int32_t ShaderID;
	//typedef std::vector< std::pair<boost::uint32_t, ShaderCGFXRef> > CGFXEffectList;
	typedef std::map<int32_t, ShaderCGFXRef> CGFXEffectMap;
	class CGFXManager
	{
	public:
		CGFXManager();
		~CGFXManager();

		void init();

		// Reference driven
		ShaderCGFXRef loadEffectFromFile( const std::string& filename );
		ShaderCGFXRef getEffect( const ShaderID id );
		void reloadEffect( V::ShaderCGFXRef fx );
		void reloadEffect( const ShaderID id );
		void reloadAll();

		ShaderCGFXRef active()	{ return mActiveFX; }

		// Id driven
		ShaderID createEffectFromFile( const std::string& filename );
		ShaderCGFXRef enable( uint32_t id );
		void disable();

	private:
		//CGFXEffectList	mEffectList;
		CGFXEffectMap	mEffectMap;
		CGcontext		mContext;

		ShaderID		mFXCount;
		ShaderCGFXRef	mActiveFX;
	};

}	// end namespace


//#endif