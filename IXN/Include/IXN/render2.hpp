#pragma once
/*
*/

#include <IXN/descriptor.hpp>
#include <IXN/comms.hpp>
#include <IXN/surface.hpp>
#include <IXN/hyper-vector.hpp>
#include <IXN/utility-helper.hpp>

#if defined( _ENGINE_GL_DIRECT_2D1 )

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



using render_spec2_tmx_t = D2D1::Matrix3x2F;

class RenderSpec2 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "RenderSpec2" );

public:
    using target_t = ID2D1HwndRenderTarget;

public:
    RenderSpec2() = default;

    RenderSpec2( HVEC< RenderSpec2 > other ) 
    : _super_spec{ std::move( other ) },
      _renderer{ _super_spec->_renderer }
    {}

_ENGINE_PROTECTED:
    RenderSpec2( Renderer2& renderer )
    : _renderer{ renderer }
    {}

public:
    virtual ~RenderSpec2() = default;

_ENGINE_PROTECTED:
    HVEC< RenderSpec2 >   _super_spec    = nullptr;
    HVEC< Renderer2 >     _renderer      = nullptr;

public:
    virtual RenderSpec2& rs2_uplink() = 0;

    virtual RenderSpec2& rs2_downlink() = 0;

public:
    virtual RenderSpec2& fill( const RGBA& ) = 0;

    virtual RenderSpec2& fill( const Sweep2& ) = 0;

    virtual RenderSpec2& line( Crd2, Crd2, const Sweep2& ) = 0;

    virtual RenderSpec2& line( Vec2, Vec2, const Sweep2& ) = 0;

public:
    virtual Crd2 crd() const = 0;

    virtual Vec2 vec() const = 0;

    virtual Vec2 size() const = 0;

    virtual render_spec2_tmx_t tmx() const = 0;

public:
    virtual DWORD deep_dive( render_spec2_tmx_t& ) const = 0;

public:
    RenderSpec2& super_spec() {
        return *_super_spec;
    }

    Renderer2& renderer() {
        return *_renderer;
    }

    Surface& surface();

public:
    target_t* target();

};

class BareRenderSlave2 {
public:
    virtual DWORD rend2( RenderSpec2& spec, void* args ) = 0;

public:
    template< typename T >
    static T& rend2_args_as( void* args ) {
        return *reinterpret_cast< T* >( args ); 
    }

};


enum RENDERER2_DFT_SWEEP {
    RENDERER2_DFT_SWEEP_VOLATILE = 0x00'00'00'FF'00'00'00'00,
    RENDERER2_DFT_SWEEP_RED      = 0xFF'00'00'FF'00'00'00'01,
    RENDERER2_DFT_SWEEP_GREEN    = 0x00'FF'00'FF'00'00'00'02, 
    RENDERER2_DFT_SWEEP_BLUE     = 0x00'00'FF'FF'00'00'00'03,
    RENDERER2_DFT_SWEEP_WHITE    = 0xFF'FF'FF'FF'00'00'00'04,
    RENDERER2_DFT_SWEEP_BLACK    = 0x00'00'00'FF'00'00'00'05,
    RENDERER2_DFT_SWEEP_MAGENTA  = 0xFF'00'FF'FF'00'00'00'06,
    RENDERER2_DFT_SWEEP_YELLOW   = 0xFF'FF'00'FF'00'00'00'07,

    RENDERER2_DFT_SWEEP_DFT = RENDERER2_DFT_SWEEP_MAGENTA,

    RENDERER2_DFT_SWEEP_RGBA_MASK = 0xFF'FF'FF'FF'00'00'00'00,
    RENDERER2_DFT_SWEEP_IDX_MASK  = ~RENDERER2_DFT_SWEEP_RGBA_MASK,

    _RENDERER2_DFT_SWEEP_FORCE_QWORD = 0xFF'FF'FF'FF'FF'FF'FF'FF
};

struct Renderer2DefaultSweeps {
public:
    Renderer2DefaultSweeps() = default;

    Renderer2DefaultSweeps( Renderer2& renderer, _ENGINE_COMMS_ECHO_ARG );

public:
    Renderer2DefaultSweeps& operator = ( Renderer2DefaultSweeps&& other ) {
        _default_sweeps = std::move( other._default_sweeps );
        return *this;
    }

_ENGINE_PROTECTED:
    std::vector< HVEC< Sweep2 > >   _default_sweeps   = {};

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

    Renderer2( HVEC< Surface > surface, _ENGINE_COMMS_ECHO_ARG )
    : RenderSpec2{ *this }, _surface{ std::move( surface ) }
    {
        if( CoInitialize( nullptr ) != S_OK ) { 
            echo( this, EchoLevel_Error ) << "CoInitialize() failure.";
            return;
        }    

        if(
            CoCreateInstance(
                CLSID_WICImagingFactory,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IWICImagingFactory,
                ( void** )&_wic_factory
            ) != S_OK 
        ) {
            echo( this, EchoLevel_Error ) << "CoCreateInstance() failure.";
            return;
        }

        if( 
            D2D1CreateFactory( 
                D2D1_FACTORY_TYPE_MULTI_THREADED, 
                &_factory
            ) != S_OK 
        ) {
            echo( this, EchoLevel_Error ) << "D2D1CreateFactory() failure.";
            return;
        }

        if(
            _factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    _surface->handle(),
                    D2D1::SizeU( _surface->width(), _surface->height() ),
                    D2D1_PRESENT_OPTIONS_IMMEDIATELY
                ),
                &_target
            ) != S_OK
        ) {
            echo( this, EchoLevel_Error ) << "ID2D1Factory::CreateHwndRenderTarget() failure.";
            return;
        }


        _tmxs[ 0 ] = render_spec2_tmx_t::Identity();
        this->deep_dive( _tmxs[ 0 ] );
        _target->SetTransform( _tmxs[ 0 ] );

        *( Renderer2DefaultSweeps* )( this ) = Renderer2DefaultSweeps{ *this, echo };


        echo( this, EchoLevel_Ok ) << "Created.";
    }


    Renderer2( const Renderer2& ) = delete;

    Renderer2( Renderer2&& ) = delete;

public:
    ~Renderer2() override {
        if( _factory ) _factory->Release();

        if( _wic_factory ) _wic_factory->Release();

        if( _target ) _target->Release();
    }

public:
    inline static constexpr DWORD   TMX_MAX_COUNT   = 6;

_ENGINE_PROTECTED:
    HVEC< Surface >       _surface                 = nullptr;

    ID2D1Factory*         _factory                 = nullptr;
    IWICImagingFactory*   _wic_factory             = nullptr;

    target_t*             _target                  = nullptr;

    render_spec2_tmx_t    _tmxs[ TMX_MAX_COUNT ]   = {};
    DWORD                 _tmxsdx                  = 0; 

public:
    Surface& surface() {
        return *_surface;
    }

public:
    ID2D1Factory*          factory()     { return _factory; }
    ID2D1HwndRenderTarget* target()      { return _target; }
    IWICImagingFactory*    wic_factory() { return _wic_factory; }

public:
    virtual RenderSpec2& rs2_uplink() override {
        _target->BeginDraw();
        return *this;
    }

    virtual RenderSpec2& rs2_downlink() override {
        _target->EndDraw();
        return *this;
    }

public:
    Renderer2& push_tmx( const render_spec2_tmx_t& tmx, bool chain, _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD sdx = -1;

        if( sdx = _tmxsdx + 1; sdx == TMX_MAX_COUNT ) {
            echo( this, EchoLevel_Warning ) << "Pushing TMX to stack would cause overflow. Aborted.";
            return *this;
        }
        
        _tmxs[ sdx ] = tmx * ( chain ? _tmxs[ _tmxsdx ] : _tmxs[ 0 ] );
        _tmxsdx      = sdx;

        _target->SetTransform( _tmxs[ _tmxsdx ] );

        return *this;
    }

    Renderer2& pop_tmx( _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD sdx = -1;

        if( sdx = _tmxsdx - 1; sdx < 0 ) {
            echo( this, EchoLevel_Warning ) << "Popping TMX would cause underflow. Aborted.";
            return *this;
        }

        _tmxsdx = sdx;

        _target->SetTransform( _tmxs[ _tmxsdx ] );

        return *this;
    }

    render_spec2_tmx_t& top_tmx() {
        return _tmxs[ _tmxsdx ];
    }

public:
    Crd2 crd() const override { return { 0, 0 }; }
    Vec2 vec() const override { return { .5, .5 }; }
    Vec2 size() const override { return _surface->size(); }
    render_spec2_tmx_t tmx() const override { return render_spec2_tmx_t::Identity(); }

public:
    DWORD deep_dive( render_spec2_tmx_t& tmx ) const override {
        tmx = tmx * render_spec2_tmx_t::Scale( _surface->width(), _surface->height() );
        return 0;
    }

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
    
};



class Viewport2 : public RenderSpec2,
                  public SurfaceEventSentry,
                  public SurfacePointerSentry
{
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Viewport2" );

public:
    Viewport2() = default;

    Viewport2(
        HVEC< RenderSpec2 >   render_spec,
        Crd2                  crd,
        Vec2                  sz,
        _ENGINE_COMMS_ECHO_ARG
    )
    : RenderSpec2{ render_spec },
      _origin{ crd }, _size{ sz }, _size2{ sz / 2 }
    {
        this->_refresh_tmxs( echo );

        echo( this, EchoLevel_Ok ) << "Created.";
    }


    Viewport2(
        HVEC< RenderSpec2 >   render_spec,
        Vec2                  org,
        Vec2                  sz,
        _ENGINE_COMMS_ECHO_ARG
    )
    : Viewport2{ render_spec, pull_normal_axis( org ), sz, echo }
    {}

    Viewport2( const Viewport2& ) = delete;
    Viewport2( Viewport2&& ) = delete;

_ENGINE_PROTECTED:
    Crd2                 _origin   = {};
    Vec2                 _size     = {};
    Vec2                 _size2    = {};
    render_spec2_tmx_t   _tmx      = { render_spec2_tmx_t::Identity() };
    render_spec2_tmx_t   _tmx_i    = { render_spec2_tmx_t::Identity() };

public:
    virtual RenderSpec2& rs2_uplink() override {
        return _renderer->push_tmx( _tmx, false );
    }

    virtual RenderSpec2& rs2_downlink() override {
        return _renderer->pop_tmx();
    }

public:
    Surface& surface() {
        return _renderer->surface();
    }

    SurfaceEventSentry& event_sentry() {
        if( auto ptr = dynamic_cast< SurfaceEventSentry* >( _super_spec.get() ) )
            return *ptr;
        else
            return this->surface();
    }

public:
    Crd2 crd() const override { return _origin; }
    Vec2 vec() const override { return pull_normal_axis( _origin ); }
    Vec2 size() const override { return _size; }
    render_spec2_tmx_t tmx() const override { return _tmx; }

_ENGINE_PROTECTED:
    void _refresh_tmxs( _ENGINE_COMMS_ECHO_RT_ARG ) {
        _tmx = render_spec2_tmx_t::Scale( _size.x, _size.y ) 
               * 
               render_spec2_tmx_t::Translation( _origin.x, _origin.y )
               *
               _super_spec->tmx();

        if( !( _tmx_i = _tmx ).Invert() ) echo( this, EchoLevel_Warning ) << "TMX not invertible.";
    }

public:
    DWORD deep_dive( render_spec2_tmx_t& tmx ) const override {
        Crd2 c = this->crd();

        tmx = tmx
              *
              render_spec2_tmx_t::Scale( _size.x, _size.y ) 
              * 
              render_spec2_tmx_t::Translation( c.x, c.y );

        return _super_spec->deep_dive( tmx ); 
    }

public:
    Viewport2& relocate( Crd2 crd ) {
        _origin = crd;
        return this->relocate( pull_normal_axis( crd ) );
    }

    Viewport2& relocate( Vec2 vec ) {
        _origin = vec;
        return *this;
    }

public:
    Viewport2& restrict() {
        _renderer->target()->PushAxisAlignedClip(
            D2D1::RectF( 0.0, 0.0, 1.0, 1.0 ),
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
        );
        return *this;
    }

    Viewport2& lift_restrict() {
        _renderer->target()->PopAxisAlignedClip();
        return *this;
    }

public:
    bool cages( Crd2 crd ) {
        D2D1_POINT_2F ext = D2D1_POINT_2F{ crd.x, crd.y } * _tmx_i;

        return is_between( ext.x, 0.0, 1.0 ) && is_between( ext.y, 0.0, 1.0 );
    }

    bool cages( Vec2 vec ) {
        return this->cages( pull_normal_axis( vec ) );
    }

public:
    Viewport2& srf_uplink() {
        this->event_sentry()
        .socket_plug< SURFACE_EVENT_POINTER >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, Vec2 prev_vec, auto& trace ) -> void {
                if( !this->cages( vec ) ) return;

                _pointer      = ( vec - _origin ) / _size;
                _prev_pointer = ( prev_vec - _origin ) / _size;

                this->invoke_sequence< SURFACE_EVENT_POINTER >( trace, vec, prev_vec );
            }
        )
        .socket_plug< SURFACE_EVENT_SCROLL >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, SURFSCROLL_DIRECTION dir, auto& trace ) -> void {
                if( !this->cages( vec ) ) return;

                this->invoke_sequence< SURFACE_EVENT_SCROLL >( trace, vec, dir );
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

    Viewport2& srf_downlink() {
        _renderer->surface().socket_unplug( this->xtdx() );

        return *this;
    }

public:
    Viewport2& splash_bounds( RENDERER2_DFT_SWEEP sweep_idx = RENDERER2_DFT_SWEEP_DFT );

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



using sweep2_gc_node_t = std::pair< RGBA, float >;

struct sweep2_gc_t : std::vector< D2D1_GRADIENT_STOP > {
    sweep2_gc_t() = default;

    sweep2_gc_t( std::initializer_list< sweep2_gc_node_t > ls ) {
        this->reserve( ls.size() );

        for( auto& l : ls )
            this->emplace_back( D2D1_GRADIENT_STOP{
                position: l.second,
                color: l.first
            } );
    }

    sweep2_gc_t& push( sweep2_gc_node_t gs_pair ) {
        this->emplace_back( D2D1_GRADIENT_STOP{
            position: gs_pair.second,
            color: gs_pair.first
        } );

        return *this;
    }
};

class Sweep2 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Sweep2" );

public:
    Sweep2() = default;

    Sweep2( float width )
    : _width( width )
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

public:
    float a() const {
        return this->sweep()->GetOpacity();
    }

    Sweep2& a( float value ) {
        this->sweep()->SetOpacity( value );
        return *this;
    }


};

class SldSweep2 : public Sweep2 {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "SldSweep2" );

public:
    SldSweep2() = default;

    SldSweep2(
        Renderer2& renderer,
        RGBA       rgba     = {},
        float      width    = 0.01,
        Echo       echo     = {}
    )
    : Sweep2{ width }
    {
        if( renderer.target()->CreateSolidColorBrush( rgba, &_sweep ) != S_OK ) {
            echo( this, EchoLevel_Error ) << "Renderer2::target()->CreateSolidColorBrush() failure.";
            return;
        }

        echo( this, EchoLevel_Ok ) << "Created.";
    }

public:
    virtual ~SldSweep2() override {
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

public:
    SldSweep2& rgba( RGBA c ) {
        _sweep->SetColor( c );

        return *this;
    }

    SldSweep2& r( float value ) {
        return rgba( { value, g(), b() } );
    }

    SldSweep2& g( float value ) {
        return rgba( { r(), value, b() } );
    }

    SldSweep2& b( float value ) {
        return rgba( { r(), g(), value } );
    }

};

class LnrSweep2 : public Sweep2 {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "LnrSweep2" );

public:
    LnrSweep2() = default;

    LnrSweep2(
        HVEC< RenderSpec2 >   render_spec,
        Vec2                  launch,
        Vec2                  land,
        const sweep2_gc_t&    chain,
        float                 width    = 0.01,
        float                 alpha    = 1.0,
        _ENGINE_COMMS_ECHO_ARG
    )
    : Sweep2{ width }, _render_spec{ std::move( render_spec ) }
    {
        if(
            _render_spec->target()->CreateGradientStopCollection(
                chain.data(),
                chain.size(),
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &_grads
            ) != S_OK
        ) {
            echo( this, EchoLevel_Error ) << "_render_spec->target()->CreateGradientStopCollection() failure.";
            return;
        }

        if(
            _render_spec->target()->CreateLinearGradientBrush(
                D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES{
                    pull_normal_axis( launch ), pull_normal_axis( land )
                },
                D2D1_BRUSH_PROPERTIES{ alpha, render_spec2_tmx_t::Identity() },
                _grads,
                &_sweep
            ) != S_OK
        ) {
            echo( this, EchoLevel_Error ) << "_render_spec->target()->CreateLinearGradientBrush() failure.";
            return;
        }

        echo( this, EchoLevel_Ok ) << "Created.";
    }

public:
    virtual ~LnrSweep2() override {
        if( _sweep ) _sweep->Release();

        if( _grads ) _grads->Release();
    }

_ENGINE_PROTECTED:
    HVEC< RenderSpec2 >            _render_spec   = nullptr;

    ID2D1LinearGradientBrush*      _sweep         = nullptr;
    ID2D1GradientStopCollection*   _grads         = nullptr;

public:
    virtual ID2D1Brush* sweep() const override {
        return _sweep;
    }

public:
    Vec2 launch() const {
        auto [ x, y ] = _sweep->GetStartPoint();
        return {};
    }

    Vec2 land() const {
        auto [ x, y ] = _sweep->GetEndPoint();
        return {};
    }

public:
    LnrSweep2& launch_at( Vec2 vec ) {
        _sweep->SetStartPoint( {} );
        return *this;
    }

    LnrSweep2& land_at( Vec2 vec ) {
        _sweep->SetEndPoint( {} );
        return *this;
    }

};

class RdlSweep2 : public Sweep2 {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "RdlSweep2" );

public:
    RdlSweep2() = default;

    RdlSweep2(
        HVEC< RenderSpec2 >   render_spec,
        Vec2                  org,
        Vec2                  off,
        Vec2                  rad,
        const sweep2_gc_t     chain,
        float                 width     = 0.01,
        float                 alpha     = 1.0,
        _ENGINE_COMMS_ECHO_ARG
    )
    : Sweep2{ width }, _render_spec{ std::move( render_spec ) }
    {
        if(
            _render_spec->target()->CreateGradientStopCollection(
                chain.data(),
                chain.size(),
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &_grads
            ) != S_OK
        ) {
            echo( this, EchoLevel_Error ) << "_render_spec->target()->CreateGradientStopCollection failure.";
            return;
        }

        if(
            _render_spec->target()->CreateRadialGradientBrush(
                D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES{
                    Crd2{ pull_normal_axis( org ) }, 
                    Crd2{ Vec2{ off.x, -off.y } },
                    rad.x, rad.y
                },
                D2D1_BRUSH_PROPERTIES{ alpha, render_spec2_tmx_t::Identity() },
                _grads,
                &_sweep
            ) != S_OK
        ) {
            echo( this, EchoLevel_Error ) << "_render_spec->target()->CreateRadialGradientBrush failure.";
            return;
        }

        echo( this, EchoLevel_Ok ) << "Created.";
    }

public:
    virtual ~RdlSweep2() override {
        if( _sweep ) _sweep->Release();

        if( _grads ) _grads->Release();
    }

private:
    HVEC< RenderSpec2 >            _render_spec   = nullptr;

    ID2D1RadialGradientBrush*      _sweep         = nullptr;
    ID2D1GradientStopCollection*   _grads         = nullptr;

public:
    virtual ID2D1Brush* sweep() const override {
        return _sweep;
    }

public:
    Vec2 org() const {
        auto [ x, y ] = _sweep->GetCenter();
        return {};
    }

    Vec2 off() const {
        auto [ x, y ] = _sweep->GetGradientOriginOffset();
        return {};
    }

    float radx() const {
        return _sweep->GetRadiusX();
    }

    float rady() const {
        return _sweep->GetRadiusY();
    }

    Vec2 rad() const {
        return { this->radx(), this->rady() };
    }

public:
    RdlSweep2& org_at( Vec2 vec ) {
        _sweep->SetCenter( pull_normal_axis( vec ) );
        return *this;
    }

    RdlSweep2& off_at( Vec2 off ) {
        _sweep->SetGradientOriginOffset( Crd2{ off.x, -off.y } );
        return *this;
    }

    RdlSweep2& radx_at( ggfloat_t rx ) {
        _sweep->SetRadiusX( rx );
        return *this;
    }

    RdlSweep2& rady_at( ggfloat_t ry ) {
        _sweep->SetRadiusY( ry );
        return *this;
    }

    RdlSweep2& rad_at( Vec2 vec ) {
        return this->radx_at( vec.x ).rady_at( vec.y );
    }

};




class Sprite2 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Sprite2" );

public:
    Sprite2() = default;

    Sprite2(
        HVEC< RenderSpec2 >   render_spec,
        std :: string_view    path,
        _ENGINE_COMMS_ECHO_ARG
    ) 
    : _path{ path }
    {
        struct Tools {
            Tools() = default;

            ~Tools() {
                if( wic_converter ) wic_converter->Release();
                if( wic_decoder ) wic_decoder->Release();
                if( wic_frame ) wic_frame->Release(); 
            }

            IWICBitmapDecoder*     wic_decoder   = nullptr;
            IWICBitmapFrameDecode* wic_frame     = nullptr;
            IWICFormatConverter*   wic_converter = nullptr;

        } tools{};

        if( UDWORD res = 
            render_spec->renderer().wic_factory()->CreateDecoderFromFilename(
                std::wstring{ path.begin(), path.end() }.c_str(),
                nullptr,
                GENERIC_READ,
                WICDecodeOptions::WICDecodeMetadataCacheOnLoad,
                &tools.wic_decoder
            ); res != S_OK
        ) {
            echo( this, EchoLevel_Error ) << "render_spec->renderer().wic_factory()->CreateDecoderFromFilename failure: #" << res << " on: \"" << _path << "\".";
            return;
        }

        if( UDWORD res = tools.wic_decoder->GetFrame( 0, &tools.wic_frame ); res != S_OK ) {
            echo( this, EchoLevel_Error ) << "tools.wic_decoder->GetFrame failure: #" << res << ".";
            return;
        }

        if( UDWORD res = render_spec->renderer().wic_factory()->CreateFormatConverter( &tools.wic_converter ); res != S_OK ) {
            echo( this, EchoLevel_Error ) << "render_spec->renderer().wic_factory()->CreateFormatConverter failure: #" << res << ".";
            return;
        }

        if( UDWORD res =
            tools.wic_converter->Initialize(
                tools.wic_frame,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeCustom
            ); res != S_OK
        ) {
            echo( this, EchoLevel_Error ) << "tools.wic_converter->Initialize failure: #" << res << ".";
            return;
        }

        ID2D1Bitmap* tmp_bmp = nullptr;

        if( UDWORD res =
            render_spec->renderer().target()->CreateBitmapFromWicBitmap(
                tools.wic_converter,
                NULL,
                &tmp_bmp
            )
        ) {
            echo( this, EchoLevel_Error ) << "render_spec->renderer().target()->CreateBitmapFromWicBitmap failure: #" << res << ".";
            return;
        }

        _bitmap.vector( tmp_bmp, hvec_soft_t{} );


        UDWORD w = 0;
        UDWORD h = 0;

        tools.wic_frame->GetSize( &w, &h );

        if( w == 0 || h == 0 || w >= 10'000 || h >= 10'000 )
            echo( this, EchoLevel_Warning ) << "Abnormal dimensions: w: " << w << ", h: " << h << ".";

        echo( this, EchoLevel_Ok ) << "Created from: \"" << _path << "\".";
    }

public:
    ~Sprite2() {
        if( _bitmap.count() == 1 ) _bitmap->Release();
    }

_ENGINE_PROTECTED:
    HVEC< ID2D1Bitmap >   _bitmap   = nullptr;
    std::string           _path     = {};

};




};

#else
    #warning Compiling for DIRECT-2D1 without choosing this GL.
#endif  
