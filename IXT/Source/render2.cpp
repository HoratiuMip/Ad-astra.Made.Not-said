/*
*/

#include <IXT/render2.hpp>

namespace _ENGINE_NAMESPACE {



Renderer2DefaultBrushes::Renderer2DefaultBrushes( Renderer2& renderer, _ENGINE_COMMS_ECHO_NO_DFT_ARG ) {
    for( auto enum_bytes : { 
        RENDERER2_DFT_BRUSH_VOLATILE,
        RENDERER2_DFT_BRUSH_RED,
        RENDERER2_DFT_BRUSH_GREEN, 
        RENDERER2_DFT_BRUSH_BLUE,
        RENDERER2_DFT_BRUSH_WHITE,
        RENDERER2_DFT_BRUSH_BLACK  
    } ) {
        uint64_t bytes = ( ( uint64_t )( enum_bytes ) & ( uint64_t )( RENDERER2_DFT_BRUSH_RGBA_MASK ) ) >> 32; 

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


RenderSpec2& Viewport2::fill( const RGBA& rgba ) { 
    this->restrict();
    _renderer->fill( rgba ); /* Poor man's fill. */
    this->lift_restriction();

    return *this;
}

RenderSpec2& Viewport2::fill( const Brush2& brush ) { 
    auto tl = _renderer->pull_axis( this->top_left_g() );
    auto br = _renderer->pull_axis( this->bot_right_g() );

    _renderer->target()->FillRectangle(
        D2D1_RECT_F{ tl.x, tl.y, br.x, br.y },
        brush
    );

    return *this;
}

RenderSpec2& Viewport2::line(
    Crd2 c1, Crd2 c2,
    const Brush2& brush
) { 
    return this->line(
        _super_spec->pull_axis( c1 ),
        _super_spec->pull_axis( c2 ),
        brush
    );
}

RenderSpec2& Viewport2::line(
    Vec2 v1, Vec2 v2,
    const Brush2& brush
) {
    _super_spec->line(
        _super_spec->pull_axis( v1 * _size + _origin ),
        _super_spec->pull_axis( v2 * _size + _origin ),
        brush
    );

    return *this;
}



};
