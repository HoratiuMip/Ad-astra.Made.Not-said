#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>
#include <IXT/surface.hpp>
#include <IXT/volatile_ptr.hpp>

namespace _ENGINE_NAMESPACE {



class RenderSpec2;
class Renderer2;
class Viewport2;

class Brush2;



class Chroma {
public:
    Chroma() = default;

    Chroma( float r, float g, float b, float a = 1.0f )
    : r{ r }, g{ g }, b{ b }, a{ a }
    {}

    Chroma( float rgb, float a = 1.0f )
    : Chroma{ rgb, rgb, rgb, a }
    {}

    Chroma( std::integral auto r, std::integral auto g, std::integral auto b, uint8_t a = 255 )
    : r{ r / 255.0f }, g{ g / 255.0f }, b{ b / 255.0f }, a{ a / 255.0f }
    {}

    Chroma( std::integral auto rgb, uint8_t a = 255 )
    : Chroma{ rgb, rgb, rgb, a }
    {}

public:
    float   r   = 0.0;
    float   g   = 0.32;   /* dark verdian for nyjucu aka iupremacy */
    float   b   = 0.23;
    float   a   = 1.0;

public:
    operator float* () {
        return &r;
    }

public:
    operator const D2D1_COLOR_F& () const {
        return *reinterpret_cast< const D2D1_COLOR_F* >( this );
    }

    operator D2D1_COLOR_F& () {
        return *reinterpret_cast< D2D1_COLOR_F* >( this );
    }

};



class RenderSpec2 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "RenderSpec2" );

public:
    RenderSpec2() = default;

    RenderSpec2( VPtr< RenderSpec2 > other ) 
    : _super_spec{ std::move( other ) },
      _renderer{ other->_renderer }
    {}

_ENGINE_PROTECTED:
    RenderSpec2( Renderer2& renderer )
    : _renderer{ &renderer, weak_link_t{} }
    {}

public:
    virtual ~RenderSpec2() {}

_ENGINE_PROTECTED:
    VPtr< RenderSpec2 >   _super_spec    = nullptr;
    VPtr< Renderer2 >     _renderer      = nullptr;

public:
    virtual RenderSpec2& fill( const Chroma& ) = 0;

    virtual RenderSpec2& fill( const Brush2& ) = 0;

    virtual RenderSpec2& line( Crd2, Crd2, const Brush2& ) = 0;

    virtual RenderSpec2& line( Vec2, Vec2, const Brush2& ) = 0;

public:
    virtual Crd2 coord() const = 0;

    virtual Vec2 origin() const = 0;

    virtual Vec2 size() const = 0;

public:
    Vec2 pull_axis( const Crd2 crd ) {
        return { 2.0_ggf * crd.x - 1.0_ggf, 2.0_ggf * ( 1.0_ggf - crd.y ) - 1.0_ggf };
    }

    Crd2 pull_axis( const Vec2 vec ) {
        return { ( vec.x + 1.0_ggf ) / 2.0_ggf, 1.0_ggf - ( vec.y + 1.0_ggf ) / 2.0_ggf };
    }

    void push_axis( Crd2& crd ) {
        crd = this->pull_axis( crd );
    }

    void push_axis( Vec2& vec ) {
        vec = this->pull_axis( vec );
    }

public:
    RenderSpec2& super_spec() {
        return *_super_spec;
    }

    Renderer2& renderer() {
        return *_renderer;
    }

    Surface& surface();

};



class Renderer2 : public RenderSpec2 {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Renderer2" );

public:
    Renderer2() = default;

    Renderer2( VPtr< Surface > surface, _ENGINE_COMMS_ECHO_ARG )
    : RenderSpec2{ *this }, _surface{ std::move( surface ) }
    {
        void* tmp_ptr = nullptr;

        if( CoInitialize( nullptr ) != S_OK ) { 
            echo( this, ECHO_STATUS_ERROR ) << "<constructor>: CoInitialize() failure.";
            return;
        }    

        if(
            CoCreateInstance(
                CLSID_WICImagingFactory,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IWICImagingFactory,
                &tmp_ptr
            ) != S_OK 
        ) {
            echo( this, ECHO_STATUS_ERROR ) << "<constructor>: CoCreateInstance() failure.";
            return;
        }
        _wic_factory.reset( tmp_ptr, weak_link_t{} );

        if( 
            D2D1CreateFactory( 
                D2D1_FACTORY_TYPE_MULTI_THREADED, 
                reinterpret_cast< decltype( _factory )::T** >( &tmp_ptr ) 
            ) != S_OK 
        ) {
            echo( this, ECHO_STATUS_ERROR ) << "<constructor>: D2D1CreateFactory() failure.";
            return;
        }
        _factory.reset( tmp_ptr, weak_link_t{} );

        if( 
            _factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    _surface->handle(),
                    D2D1::SizeU( _surface->width(), _surface->height() ),
                    D2D1_PRESENT_OPTIONS_IMMEDIATELY
                ),
                reinterpret_cast< decltype( _target )::T** >( &tmp_ptr )
            ) != S_OK
        ) {
            echo( this, ECHO_STATUS_ERROR ) << "<constructor>: _factory->CreateHwndRenderTarget() failure.";
            return;
        }
        _target.reset( tmp_ptr, weak_link_t{} );


        echo( this, ECHO_STATUS_OK ) << "Created.";
    }


    Renderer2( const Renderer2& ) = delete;

    Renderer2( Renderer2&& ) = delete;

public:
    ~Renderer2() override {
        if( _factory ) _factory->Release();

        if( _wic_factory ) _wic_factory->Release();

        if( _target ) _target->Release();
    }

_ENGINE_PROTECTED:
    VPtr< Surface >                 _surface       = nullptr;

    VPtr< ID2D1Factory >            _factory       = nullptr;
    VPtr< IWICImagingFactory >      _wic_factory   = nullptr;

    VPtr< ID2D1HwndRenderTarget >   _target        = nullptr;

public:
    Surface& surface() {
        return *_surface;
    }

public:
    ID2D1Factory*          factory()     { return _factory; }
    ID2D1HwndRenderTarget* target()      { return _target; }
    IWICImagingFactory*    wic_factory() { return _wic_factory; }

public:
    Renderer2& charge() {
        _target->BeginDraw();

        return *this;
    }

    Renderer2& blast() {
        _target->EndDraw();

        return *this;
    }

public:
    Crd2 coord()  const override { return { 0, 0 }; }
    Vec2 origin() const override { return { 0, 0 }; }
    Vec2 size()   const override { return _surface->size(); }

public:
    RenderSpec2& fill( const Chroma& chroma ) override;

    RenderSpec2& fill( const Brush2& brush ) override;

public:
    RenderSpec2& line(
        Crd2 c1, Crd2 c2,
        const Brush2& brush
    ) override;

    RenderSpec2& line(
        Vec2 v1, Vec2 v2,
        const Brush2& brush
    ) override;

public:
    template< typename Type, typename ...Args >
    Renderer2& operator () ( const Type& thing, Args&&... args ) {
        thing.render( *this, std::forward< Args >( args )... );

        return *this;
    }

/*
public:
    static std::optional< std::pair< Vec2, Vec2 > > clip_CohenSutherland( const Vec2& tl, const Vec2& br, Vec2 p1, Vec2 p2 ) {
        auto& u = tl.y; auto& l = tl.x;
        auto& d = br.y; auto& r = br.x;

        // UDRL 
        auto code_of = [ & ] ( const Vec2& p ) -> char {
            return ( ( p.y > u ) << 3 ) |
                   ( ( p.y < d ) << 2 ) |
                   ( ( p.x > r ) << 1 ) |
                     ( p.x < l );
        };

        char code1 = code_of( p1 );
        char code2 = code_of( p2 );

        auto move_X = [ & ] ( Vec2& mov, const Vec2& piv, char& code ) -> void {
            for( char sh = 3; sh >= 0; --sh )
                if( ( code >> sh ) & 1 ) {
                    switch( sh ) {
                        case 3: mov = { ( u - mov.y ) / ( piv.y - mov.y ) * ( piv.x - mov.x ) + mov.x, u }; break;
                        case 2: mov = { ( d - mov.y ) / ( piv.y - mov.y ) * ( piv.x - mov.x ) + mov.x, d }; break;

                        case 1: mov = { r, ( r - mov.x ) / ( piv.x - mov.x ) * ( piv.y - mov.y ) + mov.y }; break;
                        case 0: mov = { l, ( l - mov.x ) / ( piv.x - mov.x ) * ( piv.y - mov.y ) + mov.y }; break;
                    }

                    code &= ~( 1 << sh );
                
                    break;
                }
    
        };

        bool phase = 1;

        while( true ) {
            if( code1 == 0 && code2 == 0 ) return std::make_pair( p1, p2 );

            if( code1 & code2 ) return {};

            if( phase )
                move_X( p1, p2, code1 );
            else
                move_X( p2, p1, code2 );

            phase ^= 1;
        }
    }
*/
    
};



class Viewport2 : public RenderSpec2,
                  public SurfaceEventSentry
{
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Viewport2" );

public:
    Viewport2() = default;

    Viewport2(
        RenderSpec2&   render_spec,
        Vec2           org,
        Vec2           sz,
        _ENGINE_COMMS_ECHO_ARG
    )
    : RenderSpec2{ render_spec },
      _origin{ org }, _size{ sz }, _size2{ sz.x / 2.0f, sz.y / 2.0f }
    {
        echo( this, ECHO_STATUS_OK ) << "Created.";
    }

    Viewport2(
        RenderSpec2&   render_spec,
        Crd2           crd,
        Vec2           sz,
        _ENGINE_COMMS_ECHO_ARG
    )
    : Viewport2{ render_spec, render_spec.pull_axis( crd ) + Vec2{ sz.x / 2.0_ggf, -sz.y / 2.0_ggf }, sz, echo }
    {}


    Viewport2( const Viewport2& ) = delete;
    Viewport2( Viewport2&& ) = delete;

_ENGINE_PROTECTED:
    Vec2   _origin       = {};
    Vec2   _size         = {};
    Vec2   _size2        = {};

    bool   _restricted   = false;

public:
    Surface& surface() {
        return _renderer->surface();
    }

public:
    Crd2 coord()  const override { return _super_spec->pull_axis( _origin ); }
    Vec2 origin() const override { return _origin; }
    Vec2 size()   const override { return _size; }

public:
    Viewport2& relocate( Vec2 vec ) {
        _origin = vec;

        return *this;
    }

    Viewport2& relocate( Crd2 crd ) {
        return this->relocate( _super_spec->pull_axis( crd ) );
    }

public:
    Vec2 top_left_g() const {
        return _origin + Vec2{ -_size2.x, _size2.y };
    }

    Vec2 bot_right_g() const {
        return _origin + Vec2{ _size2.x, -_size2.y };
    }

public:
    ggfloat_t east_g() const {
        return _origin.x + _size2.x;
    }

    ggfloat_t west_g() const {
        return _origin.x - _size2.x;
    }

    ggfloat_t north_g() const {
        return _origin.y + _size2.y;
    }

    ggfloat_t south_g() const {
        return _origin.y - _size2.y;
    }

    ggfloat_t east() const {
        return _size2.x;
    }

    ggfloat_t west() const {
        return -_size2.x;
    }

    ggfloat_t north() const {
        return _size2.y;
    }

    ggfloat_t south() const {
        return -_size2.y;
    }

public:
    bool contains_g( Vec2 vec ) const {
        Vec2 ref = this->top_left_g();
        
        if( vec.is_further_than( ref, HEADING_NORTH ) || vec.is_further_than( ref, HEADING_WEST ) )
            return false;

        ref = this->bot_right_g();

        if( vec.is_further_than( ref, HEADING_SOUTH ) || vec.is_further_than( ref, HEADING_EAST ) )
            return false;

        return true;
    }

public:
    Viewport2& restrict() {
        if( _restricted ) return *this;

        auto tl = _super_spec->pull_axis( this->top_left_g() );
        auto br = _super_spec->pull_axis( this->bot_right_g() );

        _renderer->target()->PushAxisAlignedClip(
            D2D1::RectF( tl.x, tl.y, br.x, br.y ),
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
        );

        _restricted = true;

        return *this;
    }

    Viewport2& lift_restriction() {
        if( !_restricted ) return *this;

        _renderer->target()->PopAxisAlignedClip();

        _restricted = false;

        return *this;
    }

    bool has_restriction() const {
        return _restricted;
    }

public:
    Viewport2& uplink() {
        this->surface()

        .socket_plug< SURFACE_EVENT_MOUSE >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, Vec2 lvec, auto& trace ) -> void {
                if( !this->contains_g( vec ) ) return;

                this->invoke_sequence< SurfaceOnPointer >( trace, vec - _origin, lvec - _origin );
            }
        )

        .socket_plug< SURFACE_EVENT_SCROLL >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, SURFSCROLL_DIRECTION dir, auto& trace ) -> void {
                if( !this->contains_g( vec ) ) return;

                this->invoke_sequence< SurfaceOnScroll >( trace, vec - _origin, dir );
            }
        )

        .socket_plug< SURFACE_EVENT_KEY >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( SurfKey key, SURFKEY_STATE state, auto& trace ) -> void {
                this->invoke_sequence< SurfaceOnKey >( trace, key, state );
            }
        );

        return *this;
    }

    Viewport2& downlink() {
        _renderer->surface().socket_unplug( this->xtdx() );

        return *this;
    }

public:
    Viewport2& fill(
        const Chroma& chroma
    );

    Viewport2& fill(
        const Brush2& brush
    );

    Viewport2& line(
        Crd2 c1, Crd2 c2,
        const Brush2& brush
    );

    Viewport2& line(
        Vec2 v1, Vec2 v2,
        const Brush2& brush
    );

};



class Brush2 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Brush2" );

public:
    Brush2() = default;

    Brush2( float w )
    : _width( w )
    {}

    virtual ~Brush2() {}

_ENGINE_PROTECTED:
    float   _width   = 1.0;

public:
    float width() const {
        return _width;
    }

    void width_to( float w ) {
        _width = w;
    }

public:
    virtual ID2D1Brush* brush() const = 0;

    operator ID2D1Brush* () const {
        return this->brush();
    }

};



class SolidBrush2 : public Brush2 {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "SolidBrush2" );

public:
    SolidBrush2() = default;

    SolidBrush2(
        Renderer2& renderer,
        Chroma     chroma   = {},
        float      w        = 1.0,
        Echo       echo     = {}
    )
    : Brush2{ w }
    {
        if( renderer.target()->CreateSolidColorBrush( chroma, &_brush ) != S_OK ) {
            echo( this, ECHO_STATUS_ERROR ) << "<constructor>: renderer.target()->CreateSolidColorBrush() failure.";
            return;
        }

        echo( this, ECHO_STATUS_OK ) << "Created.";
    }

public:
    virtual ~SolidBrush2() {
        if( _brush ) _brush->Release();
    }

_ENGINE_PROTECTED:
    ID2D1SolidColorBrush*   _brush   = nullptr;

public:
    virtual ID2D1Brush* brush() const override {
        return _brush;
    }

public:
    Chroma chroma() const {
        auto [ r, g, b, a ] = _brush->GetColor();
        return { r, g, b, a };
    }

    float r() const {
        return this->chroma().r;
    }

    float g() const {
        return this->chroma().g;
    }

    float b() const {
        return this->chroma().b;
    }

    float a() const {
        return this->chroma().a;
    }

public:
    SolidBrush2& chroma_to( Chroma c ) {
        _brush->SetColor( c );

        return *this;
    }

    SolidBrush2& r_to( float value ) {
        return chroma_to( { value, g(), b() } );
    }

    SolidBrush2& g_to( float value ) {
        return chroma_to( { r(), value, b() } );
    }

    SolidBrush2& b_to( float value ) {
        return chroma_to( { r(), g(), value } );
    }

    SolidBrush2& a_to( float value ) {
        _brush->SetOpacity( value );

        return *this;
    }

};



};