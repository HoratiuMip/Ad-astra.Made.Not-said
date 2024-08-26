/*
*/

#include <IXT/render2.hpp>

namespace _ENGINE_NAMESPACE {



Renderer2DefaultSweeps::Renderer2DefaultSweeps( Renderer2& renderer, _ENGINE_COMMS_ECHO_NO_DFT_ARG ) {
    for( auto enum_bytes : { 
        RENDERER2_DFT_SWEEP_VOLATILE,
        RENDERER2_DFT_SWEEP_RED,
        RENDERER2_DFT_SWEEP_GREEN, 
        RENDERER2_DFT_SWEEP_BLUE,
        RENDERER2_DFT_SWEEP_WHITE,
        RENDERER2_DFT_SWEEP_BLACK  
    } ) {
        uint64_t bytes = ( ( uint64_t )( enum_bytes ) & ( uint64_t )( RENDERER2_DFT_SWEEP_RGBA_MASK ) ) >> 32; 

        _default_sweeps.emplace_back( std::make_shared< SolidSweep2 >(
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

RenderSpec2& Renderer2::fill( const Sweep2& brush ) {
    _target->FillRectangle(
        D2D1_RECT_F{ 0, 0, _surface->width(), _surface->height() },
        brush
    );

    return *this;
}

RenderSpec2& Renderer2::line(
    Crd2 c1, Crd2 c2,
    const Sweep2& brush
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
    const Sweep2& brush
) {
    return this->line(
        pull_normal_axis( v1 ),
        pull_normal_axis( v2 ),
        brush
    );
}


RenderSpec2& Viewport2::fill( const RGBA& rgba ) { 
    this->restrict();
    _renderer->fill( rgba ); /* Poor man's fill. */
    this->lift_restriction();

    return *this;
}

RenderSpec2& Viewport2::fill( const Sweep2& brush ) { 
    auto tl = pull_normal_axis( this->top_left_g() );
    auto br = pull_normal_axis( this->bot_right_g() );

    _renderer->target()->FillRectangle(
        D2D1_RECT_F{ tl.x, tl.y, br.x, br.y },
        brush
    );

    return *this;
}

RenderSpec2& Viewport2::line(
    Crd2 c1, Crd2 c2,
    const Sweep2& brush
) { 
    return this->line(
        pull_normal_axis( c1 ),
        pull_normal_axis( c2 ),
        brush
    );
}

RenderSpec2& Viewport2::line(
    Vec2 v1, Vec2 v2,
    const Sweep2& brush
) {
    _super_spec->line(
        pull_normal_axis( v1 * _size + _origin ),
        pull_normal_axis( v2 * _size + _origin ),
        brush
    );

    return *this;
}



};
