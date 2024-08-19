/*
*/

#include <IXT/render2.hpp>

namespace _ENGINE_NAMESPACE {



Renderer2DefaultBrushes::Renderer2DefaultBrushes( Renderer2& renderer, _ENGINE_COMMS_ECHO_NO_DFD_ARG ) {
    for( auto enum_bytes : { 
        RENDERER2_DEF_BRUSH_VOLATILE,
        RENDERER2_DEF_BRUSH_RED,
        RENDERER2_DEF_BRUSH_GREEN, 
        RENDERER2_DEF_BRUSH_BLUE,
        RENDERER2_DEF_BRUSH_WHITE,
        RENDERER2_DEF_BRUSH_BLACK  
    } ) {
        uint64_t bytes = ( ( uint64_t )( enum_bytes ) & ( uint64_t )( RENDERER2_DEF_BRUSH_RGBA_MASK ) ) >> 32; 

        _default_brushes.emplace_back( std::make_shared< SolidBrush2 >(
            renderer, 
            RGBA{
                ( uint8_t )( ( bytes & 0xFF'00'00'00 ) >> 24 ),
                ( uint8_t )( ( bytes & 0x00'FF'00'00 ) >> 16 ),
                ( uint8_t )( ( bytes & 0x00'00'FF'00 ) >> 8 ),
                ( uint8_t )( ( bytes & 0x00'00'00'FF ) >> 0 )
            }, 
            3.0f, 
            echo
        ) ); 
    }
}

RenderSpec2& Renderer2::fill( const RGBA& rgba ) {
    _target->Clear( rgba );

    return *this;
}

RenderSpec2& Renderer2::fill( const Brush2& brush ) {
    _target->FillRectangle(
        D2D1_RECT_F{ 0, 0, _surface->width(), _surface->height() },
        brush
    );

    return *this;
}

RenderSpec2& Renderer2::line(
    Crd2 c1, Crd2 c2,
    const Brush2& brush
) {
    _target->DrawLine(
        _surface->localize( c1 ), _surface->localize( c2 ),
        brush,
        brush.width()
    );

    return *this; 
}

RenderSpec2& Renderer2::line(
    Vec2 v1, Vec2 v2,
    const Brush2& brush
) {
    return this->line(
        this->pull_axis( v1 ),
        this->pull_axis( v2 ),
        brush
    );
}



};
