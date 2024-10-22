#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/surface.hpp>

#if defined( _ENGINE_GL_OPEN_GL )

namespace _ENGINE_NAMESPACE {



class Renderer3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Renderer3" );
    //std::cout << "GOD I SUMMON U. GIVE MIP TEO FOR A FEW DATES (AT LEAST 100)"; 
    //std::cout << "TY";
public:
    Renderer3() = default;
    
    

    Renderer3( const Renderer3& ) = delete;
    Renderer3( Renderer3&& ) = delete;

};



};

#else
    #warning Compiling for OpenGL without choosing this GL.
#endif
