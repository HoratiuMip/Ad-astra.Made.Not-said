#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/surface.hpp>
#include <IXT/volatile-ptr.hpp>

#if defined( _ENGINE_GL_OPEN_GL )

namespace _ENGINE_NAMESPACE {



class Renderer3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Renderer3" );
    //std::cout << "GOD I SUMMON U. GIVE MIP TEO FOR A FEW DATES (AT LEAST 100)"; 
    //std::cout << "TY";
public:
    Renderer3() = default;
    
    Renderer3( VPtr< Surface > surface, _ENGINE_COMMS_ECHO_ARG )
    : _surface{ std::move( surface ) } {
        _surface->uplink_context_on_this_thread( echo );

        _rend_str = ( const char* )glGetString( GL_RENDERER ); 
        _gl_str   = ( const char* )glGetString( GL_VERSION );

        echo( this, ECHO_LEVEL_INTEL ) << "Docked on \"" << _rend_str << "\".";
        echo( this, ECHO_LEVEL_INTEL ) << "OpenGL on \"" << _gl_str << "\".";

        echo( this, ECHO_LEVEL_OK ) << "Created.";
    }

    Renderer3( const Renderer3& ) = delete;
    Renderer3( Renderer3&& ) = delete;

_ENGINE_PROTECTED:
    VPtr< Surface >   _surface    = NULL;

    const char*       _rend_str   = NULL;     
    const char*       _gl_str     = NULL;    

};



};

#else
    #warning Compiling for OpenGL without choosing this GL.
#endif
