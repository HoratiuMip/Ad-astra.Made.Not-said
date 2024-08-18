/*
*/

#include <IXT/render2.hpp>

namespace _ENGINE_NAMESPACE {



RenderSpec2& Renderer2::fill( const Chroma& chroma ) {
    return *this;
}

RenderSpec2& Renderer2::fill( const Brush2& brush ) {
    return *this; 
}

RenderSpec2& Renderer2::line(
    Crd2 c1, Crd2 c2,
    const Brush2& brush
) {
    return *this; 
}

RenderSpec2& Renderer2::line(
    Vec2 v1, Vec2 v2,
    const Brush2& brush
) {
    return *this; 
}



};
