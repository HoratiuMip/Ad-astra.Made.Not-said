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



};



};

#else
    #error Compiling for OpenGL without choosing this GL.
#endif
