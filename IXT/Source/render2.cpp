/*
*/

#include <IXT/render2.hpp>

namespace _ENGINE_NAMESPACE {



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
