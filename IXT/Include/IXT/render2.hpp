#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>
#include <IXT/surface.hpp>
#include <IXT/volatile-ptr.hpp>

namespace _ENGINE_NAMESPACE {



class RenderSpec2;
class Renderer2;
class Viewport2;

class Sweep2;



class RGBA {
public:
    RGBA() = default;

    RGBA( float r, float g, float b, float a = 1.0f )
    : r{ r }, g{ g }, b{ b }, a{ a }
    {}

    RGBA( float rgb, float a = 1.0f )
    : RGBA{ rgb, rgb, rgb, a }
    {}

    RGBA( std::integral auto r, std::integral auto g, std::integral auto b, uint8_t a = 255 )
    : r{ r / 255.0f }, g{ g / 255.0f }, b{ b / 255.0f }, a{ a / 255.0f }
    {}

    RGBA( std::integral auto rgb, uint8_t a = 255 )
    : RGBA{ rgb, rgb, rgb, a }
    {}

public:
    float   r   = .0;
    float   g   = .32;   /* dark verdian for nyjucu aka iupremacy */
    float   b   = .23;
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
      _renderer{ _super_spec->_renderer }
    {}

_ENGINE_PROTECTED:
    RenderSpec2( Renderer2& renderer )
    : _renderer{ &renderer, weak_link_t{} }
    {}

public:
    virtual ~RenderSpec2() = default;

_ENGINE_PROTECTED:
    VPtr< RenderSpec2 >   _super_spec    = nullptr;
    VPtr< Renderer2 >     _renderer      = nullptr;

public:
    virtual RenderSpec2& fill( const RGBA& ) = 0;

    virtual RenderSpec2& fill( const Sweep2& ) = 0;

    virtual RenderSpec2& line( Crd2, Crd2, const Sweep2& ) = 0;

    virtual RenderSpec2& line( Vec2, Vec2, const Sweep2& ) = 0;

public:
    virtual Crd2 coord() const = 0;

    virtual Vec2 origin() const = 0;

    virtual Vec2 size() const = 0;

public:
    RenderSpec2& super_spec() {
        return *_super_spec;
    }

    Renderer2& renderer() {
        return *_renderer;
    }

    Surface& surface();

};



enum RENDERER2_DFT_SWEEP {
    RENDERER2_DFT_SWEEP_VOLATILE = 0x00'00'00'FF'00'00'00'00,
    RENDERER2_DFT_SWEEP_RED      = 0xFF'00'00'FF'00'00'00'01,
    RENDERER2_DFT_SWEEP_GREEN    = 0x00'FF'00'FF'00'00'00'02, 
    RENDERER2_DFT_SWEEP_BLUE     = 0x00'00'FF'FF'00'00'00'03,
    RENDERER2_DFT_SWEEP_WHITE    = 0xFF'FF'FF'FF'00'00'00'04,
    RENDERER2_DFT_SWEEP_BLACK    = 0x00'00'00'FF'00'00'00'05,

    RENDERER2_DFT_SWEEP_RGBA_MASK = 0xFF'FF'FF'FF'00'00'00'00,
    RENDERER2_DFT_SWEEP_IDX_MASK  = ~RENDERER2_DFT_SWEEP_RGBA_MASK,

    _RENDERER2_DFT_SWEEP_FORCE_QWORD = 0xFF'FF'FF'FF'FF'FF'FF'FF
};

class Renderer2DefaultSweeps {
public:
    Renderer2DefaultSweeps() = default;

    Renderer2DefaultSweeps( Renderer2& renderer, _ENGINE_COMMS_ECHO_ARG );

public:
    Renderer2DefaultSweeps& operator = ( Renderer2DefaultSweeps&& other ) {
        _default_sweeps = std::move( other._default_sweeps );
        return *this;
    }

_ENGINE_PROTECTED:
    std::vector< VPtr< Sweep2 > >   _default_sweeps   = {};

public:
    Sweep2& pull( RENDERER2_DFT_SWEEP idx ) {
        return *_default_sweeps[ idx & RENDERER2_DFT_SWEEP_IDX_MASK ];
    }

    Sweep2& operator [] ( RENDERER2_DFT_SWEEP idx ) {
        return this->pull( idx );
    }

};

class Renderer2 : public RenderSpec2,
                  public Renderer2DefaultSweeps
{
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


        *( Renderer2DefaultSweeps* )( this ) = Renderer2DefaultSweeps{ *this, echo };


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

    Renderer2& splash() {
        _target->EndDraw();

        return *this;
    }

public:
    Crd2 coord()  const override { return { 0, 0 }; }
    Vec2 origin() const override { return { 0, 0 }; }
    Vec2 size()   const override { return _surface->size(); }

public:
    RenderSpec2& fill( const RGBA& rgba ) override;

    RenderSpec2& fill( const Sweep2& sweep ) override;

public:
    RenderSpec2& line(
        Crd2 c1, Crd2 c2,
        const Sweep2& sweep
    ) override;

    RenderSpec2& line(
        Vec2 v1, Vec2 v2,
        const Sweep2& sweep
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
        VPtr< RenderSpec2 >   render_spec,
        Vec2                  org,
        Vec2                  sz,
        _ENGINE_COMMS_ECHO_ARG
    )
    : RenderSpec2{ render_spec },
      _origin{ org }, _size{ sz }, _size2{ sz / 2}
    {
        echo( this, ECHO_STATUS_OK ) << "Created.";
    }

    Viewport2(
        VPtr< RenderSpec2 >   render_spec,
        Crd2                  crd,
        Vec2                  sz,
        _ENGINE_COMMS_ECHO_ARG
    )
    : Viewport2{ render_spec, pull_axis( crd ), sz, echo }
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
    Crd2 coord()  const override { return pull_axis( _origin ); }
    Vec2 origin() const override { return _origin; }
    Vec2 size()   const override { return _size; }

public:
    Viewport2& relocate( Vec2 vec ) {
        _origin = vec;

        return *this;
    }

    Viewport2& relocate( Crd2 crd ) {
        return this->relocate( pull_axis( crd ) );
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

        auto tl = pull_axis( this->top_left_g() );
        auto br = pull_axis( this->bot_right_g() );

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

        .socket_plug< SURFACE_EVENT_POINTER >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, Vec2 lvec, auto& trace ) -> void {
                if( !this->contains_g( vec ) ) return;

                this->invoke_sequence< SURFACE_EVENT_POINTER >( trace, vec - _origin, lvec - _origin );
            }
        )

        .socket_plug< SURFACE_EVENT_SCROLL >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, SURFSCROLL_DIRECTION dir, auto& trace ) -> void {
                if( !this->contains_g( vec ) ) return;

                this->invoke_sequence< SURFACE_EVENT_SCROLL >( trace, vec - _origin, dir );
            }
        )

        .socket_plug< SURFACE_EVENT_KEY >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( SurfKey key, SURFKEY_STATE state, auto& trace ) -> void {
                this->invoke_sequence< SURFACE_EVENT_KEY >( trace, key, state );
            }
        );

        return *this;
    }

    Viewport2& downlink() {
        _renderer->surface().socket_unplug( this->xtdx() );

        return *this;
    }

public:
    RenderSpec2& fill(
        const RGBA& rgba
    ) override;

    RenderSpec2& fill(
        const Sweep2& sweep
    ) override;

    RenderSpec2& line(
        Crd2 c1, Crd2 c2,
        const Sweep2& sweep
    ) override;

    RenderSpec2& line(
        Vec2 v1, Vec2 v2,
        const Sweep2& sweep
    ) override;

};



class Sweep2 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Sweep2" );

public:
    Sweep2() = default;

    Sweep2( float w )
    : _width( w )
    {}

    virtual ~Sweep2() {}

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
    virtual ID2D1Brush* sweep() const = 0;

    operator ID2D1Brush* () const {
        return this->sweep();
    }

};



class SolidSweep2 : public Sweep2 {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "SolidSweep2" );

public:
    SolidSweep2() = default;

    SolidSweep2(
        Renderer2& renderer,
        RGBA       rgba     = {},
        float      w        = 1.0,
        Echo       echo     = {}
    )
    : Sweep2{ w }
    {
        if( renderer.target()->CreateSolidColorBrush( rgba, &_sweep ) != S_OK ) {
            echo( this, ECHO_STATUS_ERROR ) << "<constructor>: renderer.target()->CreateSolidColorBrush() failure.";
            return;
        }

        echo( this, ECHO_STATUS_OK ) << "Created.";
    }

public:
    virtual ~SolidSweep2() {
        if( _sweep ) _sweep->Release();
    }

_ENGINE_PROTECTED:
    ID2D1SolidColorBrush*   _sweep   = nullptr;

public:
    virtual ID2D1Brush* sweep() const override {
        return _sweep;
    }

public:
    RGBA rgba() const {
        auto [ r, g, b, a ] = _sweep->GetColor();
        return { r, g, b, a };
    }

    float r() const {
        return this->rgba().r;
    }

    float g() const {
        return this->rgba().g;
    }

    float b() const {
        return this->rgba().b;
    }

    float a() const {
        return this->rgba().a;
    }

public:
    SolidSweep2& rgba( RGBA c ) {
        _sweep->SetColor( c );

        return *this;
    }

    SolidSweep2& r( float value ) {
        return rgba( { value, g(), b() } );
    }

    SolidSweep2& g( float value ) {
        return rgba( { r(), value, b() } );
    }

    SolidSweep2& b( float value ) {
        return rgba( { r(), g(), value } );
    }

    SolidSweep2& a( float value ) {
        _sweep->SetOpacity( value );

        return *this;
    }

};



};
