/*
*/
#include <NLN/descriptor.hpp>

#if defined( _ENGINE_GL_DIRECT_2D1 )

#include <NLN/render2.hpp>

namespace _ENGINE_NAMESPACE {



RenderSpec2::target_t* RenderSpec2::target() {
    return _renderer->target();
}

Surface& RenderSpec2::surface() {
    return _renderer->surface();
}



Renderer2DefaultSweeps::Renderer2DefaultSweeps( Renderer2& renderer, _ENGINE_COMMS_ECHO_NO_DFT_ARG ) {
    for( auto enum_bytes : { 
        RENDERER2_DFT_SWEEP_VOLATILE,
        RENDERER2_DFT_SWEEP_RED,
        RENDERER2_DFT_SWEEP_GREEN, 
        RENDERER2_DFT_SWEEP_BLUE,
        RENDERER2_DFT_SWEEP_WHITE,
        RENDERER2_DFT_SWEEP_BLACK,
        RENDERER2_DFT_SWEEP_MAGENTA,
        RENDERER2_DFT_SWEEP_YELLOW  
    } ) {
        uint64_t bytes = ( ( uint64_t )( enum_bytes ) & ( uint64_t )( RENDERER2_DFT_SWEEP_RGBA_MASK ) ) >> 32; 

        _default_sweeps.emplace_back( HVEC< SldSweep2 >::allocc(
            renderer, 
            RGBA{
                ( uint8_t )( ( bytes & 0xFF'00'00'00 ) >> 24 ),
                ( uint8_t )( ( bytes & 0x00'FF'00'00 ) >> 16 ),
                ( uint8_t )( ( bytes & 0x00'00'FF'00 ) >> 8 ),
                ( uint8_t )( ( bytes & 0x00'00'00'FF ) >> 0 )
            }, 
            0.01f, 
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
        c1, c2,
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



Viewport2& Viewport2::splash_bounds( RENDERER2_DFT_SWEEP sweep_idx ) {
    Sweep2& sweep = _renderer->pull( sweep_idx );
    
    _renderer->target()->DrawRectangle( 
        D2D1::RectF( 0, 0, 1.0, 1.0 ), 
        sweep, sweep.width()
    );

    return *this;
}

RenderSpec2& Viewport2::fill( const RGBA& rgba ) { 
    this->restrict();
    _renderer->fill( rgba ); /* Poor man's fill. */
    this->lift_restrict();

    return *this;
}

RenderSpec2& Viewport2::fill( const Sweep2& brush ) { 
    _renderer->target()->FillRectangle(
        D2D1_RECT_F{ 0.0, 0.0, 1.0, 1.0 },
        brush
    );

    return *this;
}

RenderSpec2& Viewport2::line(
    Crd2 c1, Crd2 c2,
    const Sweep2& brush
) { 
    return _renderer->line( c1, c2, brush );
}

RenderSpec2& Viewport2::line(
    Vec2 v1, Vec2 v2,
    const Sweep2& brush
) {
    return _renderer->line( v1, v2, brush );
}



};

#endif  

