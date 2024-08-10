/*
*/
#pragma once

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>
#include <IXT/aritm.hpp>



namespace _ENGINE_NAMESPACE {



enum SURFKEY_STATE {
    SURFKEY_STATE_UP   = 0,
    SURFKEY_STATE_DOWN = 1
};

class SurfKey {
public:
    enum VALUE {
        NONE = 0,

        LMB = 259, RMB, MMB,

        _0 = 48, _1, _2, _3, _4, _5, _6, _7, _8, _9,

        A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        F1 = 112, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        CTRL = 17, SHIFT = 16, ALT = 18, TAB = 9, CAPS = 20, ESC = 27, BACKSPACE = 8, ENTER = 13,

        SPACE = 32, DOT = 190, COMMA = 188, COLON = 186, APOSTH = 222, DASH = 189, EQUAL = 187, UNDER_ESC = 192,

        OPEN_BRACKET = 219, CLOSED_BRACKET = 221, BACKSLASH = 220, SLASH = 191,

        LEFT = 37, UP, RIGHT, DOWN
    };

    static constexpr size_t   COUNT   = 262;

public:
    SurfKey() = default;

    SurfKey( short key )
    : value( key )
    {}

public:
    short   value   = NONE;

public:
    operator short () const {
        return value;
    }

public:
    bool operator == ( const SurfKey& other ) const {
        return value == other.value;
    }

    bool operator == ( VALUE val ) const {
        return value == val;
    }

    std::strong_ordering operator <=> ( const SurfKey& other ) const {
        return value <=> other.value;
    }

    std::strong_ordering operator <=> ( VALUE val ) const {
        return value <=> val;
    }

public:
#if defined( _ENGINE_UNIQUE_SURFACE )
    template< typename ...Keys > static size_t any_down( Keys... keys );

    template< typename ...Keys > static size_t tgl_down( Keys... keys );

    template< typename ...Keys > static bool all_down( Keys... keys );

    static bool down( SurfKey key );
#endif

};



enum SURFSCROLL_DIRECTION {
    SURFSCROLL_DIRECTION_UP, 
    SURFSCROLL_DIRECTION_DOWN, 
    SURFSCROLL_DIRECTION_LEFT, 
    SURFSCROLL_DIRECTION_RIGHT
};



enum SURFACE_EVENT {
    SURFACE_EVENT_KEY, 
    SURFACE_EVENT_MOUSE, 
    SURFACE_EVENT_SCROLL, 
    SURFACE_EVENT_FILEDROP, 
    SURFACE_EVENT_MOVE, 
    SURFACE_EVENT_RESIZE,
    SURFACE_EVENT_DESTROY,

    _SURFACE_EVENT_DESTROY = 69100,
    _SURFACE_EVENT_CURSOR_HIDE, 
    _SURFACE_EVENT_CURSOR_SHOW,
    _SURFACE_EVENT_FORCE
};

enum SURFACE_THREAD {
    SURFACE_THREAD_ACROSS,
    SURFACE_THREAD_THROUGH
};

enum SURFACE_SOCKET_PLUG {
    SURFACE_SOCKET_PLUG_AT_ENTRY = 0,
    SURFACE_SOCKET_PLUG_AT_EXIT  = 1
};

struct SurfaceTrace {
    SurfaceTrace() = default;

    struct Result {
        XtDx                xtdx     = {};
        std::bitset< 16 >   result   = 0;
    };

    std::bitset< 64 >       master   = 0;
    std::vector< Result >   plugs    = {};

    void reset() {
        master.reset();
        plugs.clear();
    }

    std::bitset< 64 >::reference operator [] ( size_t idx ) {
        return master[ idx ];
    }

};

typedef   std::function< void( Vec2, Vec2, SurfaceTrace& ) >                   SurfaceOnPointer;
typedef   std::function< void( SurfKey, SURFKEY_STATE, SurfaceTrace& ) >       SurfaceOnKey;
typedef   std::function< void( Vec2, SURFSCROLL_DIRECTION, SurfaceTrace& ) >   SurfaceOnScroll;
typedef   std::function< void( std::vector< std::string >, SurfaceTrace& ) >   SurfaceOnFiledrop;
typedef   std::function< void( Crd2, Crd2, SurfaceTrace& ) >                   SurfaceOnMove;
typedef   std::function< void( Vec2, Vec2, SurfaceTrace& ) >                   SurfaceOnResize;
typedef   std::function< void() >                                              SurfaceOnDestroy;


class SurfaceEventSentry {
_ENGINE_PROTECTED:
    SurfaceOnPointer                     _on_ptr               = {};
    SurfaceOnKey                         _on_key               = {};
    SurfaceOnScroll                      _on_scroll            = {};
    SurfaceOnFiledrop                    _on_filedrop          = {};
    SurfaceOnMove                        _on_move              = {};
    SurfaceOnResize                      _on_resize            = {};
    SurfaceOnDestroy                     _on_destroy           = {};

    std::map< XtDx, SurfaceOnPointer >   _sckt_mouse[ 2 ]      = {};
    std::map< XtDx, SurfaceOnKey >       _sckt_key[ 2 ]        = {};
    std::map< XtDx, SurfaceOnScroll >    _sckt_scroll[ 2 ]     = {};
    std::map< XtDx, SurfaceOnFiledrop >  _sckt_filedrop[ 2 ]   = {};
    std::map< XtDx, SurfaceOnMove >      _sckt_move[ 2 ]       = {};
    std::map< XtDx, SurfaceOnResize >    _sckt_resize[ 2 ]     = {};
    std::map< XtDx, SurfaceOnDestroy >   _sckt_destroy[ 2 ]    = {};

public:
    template< typename Master, typename ...Args >
    void invoke_sequence( SurfaceTrace& trace, Args&&... args ) {
        auto [ on, sckt ] = this->_seq_from_type< Master >();

        this->_invoke_sequence( on, sckt, trace, std::forward< Args >( args )... );
    }

public:
    template< typename Master, typename Socket, typename ...Args >
    void _invoke_sequence( const Master& master, const Socket& socket, SurfaceTrace& trace, Args&&... args ) {
        for( auto& [ xtdx, sckt ] : socket[ 0 ] )
            std::invoke( sckt, std::forward< Args >( args )..., trace );

        if( master )
            std::invoke( master, std::forward< Args >( args )..., trace );

        for( auto& [ xtdx, sckt ] : socket[ 1 ] )
            std::invoke( sckt, std::forward< Args >( args )..., trace );
    }

public:
    template< SURFACE_EVENT event, typename Function >
    SurfaceEventSentry& on( Function function ) {
        this->_seq_from_event< event >().first = function; 
        return *this;
    }

    template< SURFACE_EVENT event, typename Function >
    SurfaceEventSentry& socket_plug( const XtDx& xtdx, SURFACE_SOCKET_PLUG at, Function function ) {
        this->_seq_from_event< event >().second[ at ].insert( { xtdx, function } );
        return *this;
    }

    SurfaceEventSentry& socket_unplug( const XtDx& xtdx, std::optional< SURFACE_SOCKET_PLUG > at = {} ) {
        this->_unplug( xtdx, at, _sckt_mouse );
        this->_unplug( xtdx, at, _sckt_key );
        this->_unplug( xtdx, at, _sckt_scroll );
        this->_unplug( xtdx, at, _sckt_filedrop );
        this->_unplug( xtdx, at, _sckt_move );
        this->_unplug( xtdx, at, _sckt_resize );
        return *this;
    }

    template< SURFACE_EVENT event >
    SurfaceEventSentry& socket_unplug( const XtDx& xtdx, std::optional< SURFACE_SOCKET_PLUG > at = {} ) {
        this->_unplug( xtdx, at, this->_seq_from_event< event >().second );
        return *this;
    }

_ENGINE_PROTECTED:
    template< typename Socket >
    void _unplug( const XtDx& xtdx, std::optional< SURFACE_SOCKET_PLUG > at, Socket& socket ) {
        if( at.has_value() )
            socket[ at.value() ].erase( xtdx );
        else {
            socket[ SURFACE_SOCKET_PLUG_AT_ENTRY ].erase( xtdx );
            socket[ SURFACE_SOCKET_PLUG_AT_EXIT ] .erase( xtdx );
        }
    }

_ENGINE_PROTECTED:
    template< typename Master >
    auto _seq_from_type() {
        if constexpr( std::is_same_v< Master, SurfaceOnPointer > ) 
            return std::make_pair( std::ref( _on_ptr ), std::ref( _sckt_mouse ) );

        if constexpr( std::is_same_v< Master, SurfaceOnKey > ) 
            return std::make_pair( std::ref( _on_key ), std::ref( _sckt_key ) );

        if constexpr( std::is_same_v< Master, SurfaceOnScroll > ) 
            return std::make_pair( std::ref( _on_scroll ), std::ref( _sckt_scroll ) );

        if constexpr( std::is_same_v< Master, SurfaceOnFiledrop > ) 
            return std::make_pair( std::ref( _on_filedrop ), std::ref( _sckt_filedrop ) );

        if constexpr( std::is_same_v< Master, SurfaceOnMove > ) 
            return std::make_pair( std::ref( _on_move ), std::ref( _sckt_move ) );

        if constexpr( std::is_same_v< Master, SurfaceOnResize > ) 
            return std::make_pair( std::ref( _on_resize ), std::ref( _sckt_resize ) );

        if constexpr( std::is_same_v< Master, SurfaceOnDestroy > ) 
            return std::make_pair( std::ref( _on_destroy ), std::ref( _sckt_destroy ) );
    }

    template< SURFACE_EVENT event >
    auto _seq_from_event() {
        if constexpr( event == SURFACE_EVENT_MOUSE ) 
            return std::make_pair( std::ref( _on_ptr ), std::ref( _sckt_mouse ) );

        if constexpr( event == SURFACE_EVENT_KEY ) 
            return std::make_pair( std::ref( _on_key ), std::ref( _sckt_key ) );

        if constexpr( event == SURFACE_EVENT_SCROLL ) 
            return std::make_pair( std::ref( _on_scroll ), std::ref( _sckt_scroll ) );

        if constexpr( event == SURFACE_EVENT_FILEDROP ) 
            return std::make_pair( std::ref( _on_filedrop ), std::ref( _sckt_filedrop ) );

        if constexpr( event == SURFACE_EVENT_MOVE ) 
            return std::make_pair( std::ref( _on_move ), std::ref( _sckt_move ) );

        if constexpr( event == SURFACE_EVENT_RESIZE ) 
            return std::make_pair( std::ref( _on_resize ), std::ref( _sckt_resize ) );

        if constexpr( event == SURFACE_EVENT_DESTROY ) 
            return std::make_pair( std::ref( _on_destroy ), std::ref( _sckt_destroy ) );
    }

};

enum SURFACE_STYLE {
    SURFACE_STYLE_LIQUID = WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE,
    SURFACE_STYLE_SOLID  = WS_POPUP | WS_VISIBLE

};

class Surface : public Descriptor,
                public SurfaceEventSentry 
{
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Surface" );

_ENGINE_PROTECTED:
    friend class SurfKey;
    friend class SurfPointer;

public:
    Surface() = default;

    Surface(
        std::string_view title,
        Crd2             pos      = { 0, 0 },
        Vec2             size     = { 512, 512 },
        SURFACE_STYLE    style    = SURFACE_STYLE_LIQUID,
        _ENGINE_COMMS_ECHO_ARG
    )
    : _title{ title.data() }, _position( pos ), _size( size ), _style{ style }
    {
        echo( this, ECHO_STATUS_OK ) << "Set.";
    }


    Surface( const Surface& ) = delete;

    Surface( Surface&& ) = delete;


    ~Surface() {
        this->downlink();
    }

_ENGINE_PROTECTED:
    typedef   std::array< SURFKEY_STATE, SurfKey::COUNT >   _SurfKeyArray;

_ENGINE_PROTECTED:
#if defined( _ENGINE_UNIQUE_SURFACE )
    inline static Surface*   _ptr            = nullptr;
#endif
    HWND                     _hwnd           = NULL;
    WNDCLASSEX               _wnd_class      = {};
    std::thread              _thread         = {};

    Crd2                     _position       = {};
    Vec2                     _size           = {};
    SURFACE_STYLE            _style          = SURFACE_STYLE_LIQUID;
    std::string              _title          = {};

    SurfaceTrace             _trace          = {};

    Vec2                     _pointer        = {};
    Vec2                     _prev_pointer   = {};
    _SurfKeyArray            _key_array      = { SURFKEY_STATE_UP };

_ENGINE_PROTECTED:
    void _main( std::binary_semaphore* sync, _ENGINE_COMMS_ECHO_ARG ) {
        if( !RegisterClassEx( &_wnd_class ) ) {
            echo( this, ECHO_STATUS_ERROR ) << "Bad window class registration.";

            if( sync ) sync->release(); return;
        }

        
        RECT rect{ 
            static_cast< decltype( RECT::bottom ) >( _position.x ), 
            static_cast< decltype( RECT::bottom ) >( _position.y ),
            static_cast< decltype( RECT::bottom ) >( _position.x + _size.x ), 
            static_cast< decltype( RECT::bottom ) >( _position.y + _size.y ) 
        };

        if( !AdjustWindowRect( &rect, _style, false ) )
            echo( this, ECHO_STATUS_WARNING ) << "Bad window size adjustment.";


        _hwnd = CreateWindowEx(
            WS_EX_ACCEPTFILES,

            _wnd_class.lpszClassName, _wnd_class.lpszClassName,

            _style,

            rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,

            NULL, NULL,

            GetModuleHandle( NULL ),

            this
        );

        if( !_hwnd ) {
            echo( this, ECHO_STATUS_ERROR ) << "Bad window handle.";

            if( sync ) sync->release(); return;
        }

        SetWindowText( _hwnd, _wnd_class.lpszClassName );


        echo( this, ECHO_STATUS_OK ) << ( sync ? "Created across." : "Created through." );

        if( sync ) sync->release();


        MSG event;

        while( GetMessage( &event, NULL, 0, 0 ) > 0 ) {
            TranslateMessage( &event );
            DispatchMessage( &event );
        }

    }

    static LRESULT CALLBACK event_proc_router_1( HWND hwnd, UINT event, WPARAM w_param, LPARAM l_param ) {
        if( event == WM_NCCREATE ) {
            Surface* ptr = static_cast< Surface* >(
                reinterpret_cast< LPCREATESTRUCT >( l_param )->lpCreateParams
            );

            ptr->_hwnd = hwnd;

            #if !defined( _ENGINE_UNIQUE_SURFACE )
                SetWindowLongPtr( hwnd, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( ptr ) );
            #endif

            SetWindowLongPtr( hwnd, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( event_proc_router_2 ) );
        }
            
        return DefWindowProc( hwnd, event, w_param, l_param );
    }

    static LRESULT CALLBACK event_proc_router_2( HWND hwnd, UINT event, WPARAM w_param, LPARAM l_param ) {
        #if defined( _ENGINE_UNIQUE_SURFACE )
            return Surface::get()->event_proc( hwnd, event, w_param, l_param );
        #else
            return reinterpret_cast< Surface* >( GetWindowLongPtr( hwnd, GWLP_USERDATA ) )->event_proc( hwnd, event, w_param, l_param );
        #endif
    }

    LRESULT CALLBACK event_proc( HWND hwnd, UINT event, WPARAM w_param, LPARAM l_param ) {
        auto key_proc = [ this ] ( SURFKEY_STATE state, WPARAM w_param ) -> void {
            SurfKey key = static_cast< SurfKey >( w_param );

            _key_array[ key ] = state;

            this->invoke_sequence< SurfaceOnKey >( _trace, key, state );
        };

        _trace.reset();

        switch( event ) {
            case WM_CREATE: {

            break; }

            case WM_DESTROY: {
                
            break; }

            case _SURFACE_EVENT_DESTROY: {
                DestroyWindow( hwnd );
                PostQuitMessage( 0 );
            break; }

            case _SURFACE_EVENT_CURSOR_HIDE: {
                while( ShowCursor( false ) >= 0 );
            break; }

            case _SURFACE_EVENT_CURSOR_SHOW: {
                while( ShowCursor( true ) >= 0 );
            break; }

            case _SURFACE_EVENT_FORCE :{
                this->invoke_sequence< SurfaceOnPointer >( _trace, _pointer, _pointer );
            break; }


            case WM_MOUSEMOVE: {
                Vec2 new_pointer = this->pull_axis( Crd2{ LOWORD( l_param ), HIWORD( l_param ) } );

                this->invoke_sequence< SurfaceOnPointer >( _trace, new_pointer, _pointer_l = std::exchange( _pointer, new_pointer ) );

            break; }

            case WM_MOUSEWHEEL: {
                this->invoke_sequence< SurfaceOnScroll >(
                    _trace,
                    _pointer,
                    GET_WHEEL_DELTA_WPARAM( w_param ) < 0
                    ?
                    SURFSCROLL_DIRECTION_DOWN : SURFSCROLL_DIRECTION_UP
                );

                break;
            }


            case WM_LBUTTONDOWN: {
                key_proc( SURFKEY_STATE_DOWN, SurfKey::LMB );

                break;
            }

            case WM_LBUTTONUP: {
                key_proc( SURFKEY_STATE_UP, SurfKey::LMB );

                break;
            }

            case WM_RBUTTONDOWN: {
                key_proc( SURFKEY_STATE_DOWN, SurfKey::RMB );

                break;
            }

            case WM_RBUTTONUP: {
                key_proc( SURFKEY_STATE_UP, SurfKey::RMB );

                break;
            }

            case WM_MBUTTONDOWN: {
                key_proc( SURFKEY_STATE_DOWN, SurfKey::MMB );

                break;
            }

            case WM_MBUTTONUP: {
                key_proc( SURFKEY_STATE_UP, SurfKey::MMB );

                break;
            }

            case WM_KEYDOWN: {
                if( l_param & ( 1 << 30 ) ) break;

                key_proc( SURFKEY_STATE_DOWN, w_param );

                break;
            }

            case WM_KEYUP: {
                key_proc( SURFKEY_STATE_UP, w_param );

                break;
            }


            case WM_DROPFILES: {
                size_t file_count = DragQueryFile( reinterpret_cast< HDROP >( w_param ), -1, 0, 0 );

                std::vector< std::string > files;

                for( size_t n = 0; n < file_count; ++ n ) {
                    TCHAR file[ MAX_PATH ];

                    DragQueryFile( reinterpret_cast< HDROP >( w_param ), n, file, MAX_PATH );

                    files.emplace_back( file );
                }

                this->invoke_sequence< SurfaceOnFiledrop >( _trace, std::move( files ) );

            break; }


            case WM_MOVE: {
                Crd2 new_pos = { 
                    static_cast< ggfloat_t >( ( int16_t )LOWORD( l_param ) ), 
                    static_cast< ggfloat_t >( ( int16_t )HIWORD( l_param ) )
                };

                this->invoke_sequence< SurfaceOnMove >( _trace, new_pos, std::exchange( _position, new_pos ) );

            break; }

            case WM_SIZE: {
                Vec2 new_size = { LOWORD( l_param ), HIWORD( l_param ) };

                this->invoke_sequence< SurfaceOnResize >( _trace, new_size, std::exchange( _size, new_size ) );

            break; }

        }

        return DefWindowProc( hwnd, event, w_param, l_param );
    }

public:
    Surface& uplink( SURFACE_THREAD launch = SURFACE_THREAD_THROUGH, _ENGINE_COMMS_ECHO_ARG ) {
        #if defined( _ENGINE_UNIQUE_SURFACE )
            _ptr = this;
        #endif

        _wnd_class.cbSize        = sizeof( WNDCLASSEX );
        _wnd_class.hInstance     = GetModuleHandle( NULL );
        _wnd_class.lpfnWndProc   = event_proc_router_1;
        _wnd_class.lpszClassName = _title.data();
        _wnd_class.hbrBackground = HBRUSH( COLOR_INACTIVECAPTIONTEXT );
        _wnd_class.hCursor       = LoadCursor( NULL, IDC_ARROW );


        switch( launch ) {
            case SURFACE_THREAD_THROUGH: goto L_THREAD_THROUGH;

            case SURFACE_THREAD_ACROSS: goto L_THREAD_ACROSS;

            default: echo( this, ECHO_STATUS_ERROR ) << "Bad thread launch argument."; return *this;
        }


        L_THREAD_THROUGH: {
            std::invoke( _main, this, nullptr, echo );
        } return *this;


        L_THREAD_ACROSS: {
            std::binary_semaphore sync{ 0 };

            _thread = std::thread( _main, this, &sync, echo );

            if( _thread.joinable() ) {
                echo( this, ECHO_STATUS_PENDING ) << "Waiting for across window creation...";

                sync.acquire();
            } else
                echo( this, ECHO_STATUS_ERROR ) << "Main thread bad invoke.";
        } return *this;
    }

    void downlink( _ENGINE_COMMS_ECHO_ARG ) {
        SendMessage( _hwnd, _SURFACE_EVENT_DESTROY, WPARAM{}, LPARAM{} );

        if( !UnregisterClassA( _wnd_class.lpszClassName, GetModuleHandle( NULL ) ) )
            echo( this, ECHO_STATUS_ERROR ) << "Bad window class unregistration.";

        if( _thread.joinable() )
            _thread.join();

        #if defined( _ENGINE_UNIQUE_SURFACE )
            _ptr = nullptr;
        #endif
    }

public:
    Vec2 pull_axis( const Crd2 crd ) const {
        return { crd.x - _size.x / 2.0_ggf, _size.y / 2.0_ggf - crd.y };
    }

    Crd2 pull_axis( const Vec2& vec ) const {
        return { vec.x + _size.x / 2.0f, _size.y / 2.0f - vec.y };
    }

    void push_axis( Crd2& crd ) const {
        crd.x -= _size.x / 2.0;
        crd.y = _size.y / 2.0 - crd.y;
    }

    void push_axis( Vec2& vec ) const {
        vec.x += _size.x / 2.0;
        vec.y = _size.y / 2.0 - vec.y;
    }

public:
    auto& localize( is_vec2_base auto& vec ) const {
        vec.x *= _size.x;
        vec.y *= _size.y;

        return vec;
    }

    template< is_vec2_base T >
    T localized( const T vec ) const {
        T res{ vec };

        return this->localize( res );
    }

    auto& globalize( is_vec2_base auto& vec ) const {
        vec.x /= _size.x;
        vec.y /= _size.y;

        return vec;
    }

    template< is_vec2_base T >
    T globalized( const T vec ) const {
        T res{ vec };

        return this->globalize( res );
    }

public:
    Crd2 pos() const {
        return _position;
    }

    Vec2 size() const {
        return _size;
    }

    ggfloat_t width() const {
        return _size.x;
    }

    ggfloat_t height() const {
        return _size.y;
    }

public:
    Surface& solidify() {
        SetWindowLongPtr( _hwnd, GWL_STYLE, _style = SURFACE_STYLE_SOLID );

        return *this;
    }

    Surface& liquify() {
        SetWindowLongPtr( _hwnd, GWL_STYLE, _style = SURFACE_STYLE_LIQUID );

        return *this;
    }

    Surface& move_to( Vec2 crd ) {
        _position = crd;

        SetWindowPos(
            _hwnd,
            0,
            crd.x, crd.y,
            0, 0,
            SWP_NOSIZE
        );

        return *this;
    }

    Surface& size_to( Vec2 size ) {
        _size = size;

        SetWindowPos(
            _hwnd,
            0,
            0, 0,
            size.x, size.y,
            SWP_NOMOVE
        );

        return *this;
    }

public:
    Surface& hide_cursor() {
        SendMessage( _hwnd, _SURFACE_EVENT_CURSOR_HIDE, WPARAM{}, LPARAM{} );

        return *this;
    }

    Surface& show_cursor() {
        SendMessage( _hwnd, _SURFACE_EVENT_CURSOR_SHOW, WPARAM{}, LPARAM{} );

        return *this;
    }

public:
    Vec2 ptr_vl() const {
        return _pointer;
    }

    Vec2 ptr_vg() const {
        return this->globalized( _pointer );
    }

    Vec2 ptr_pvl() const {
        return _prev_pointer;
    }

    Vec2 ptr_pvg() const {
        return this->globalized( _prev_pointer );
    }
    

    Crd2 ptr_cl() const {
        return this->pull_axis( this->ptr_vl() );
    }

    Crd2 ptr_cg() const {
        return this->globalized( this->ptr_cl() );
    }

    Crd2 ptr_pcl() const {
        return this->pull_axis( this->ptr_pvl() );
    }

    Crd2 ptr_pcg() const {
        return this->globalized( this->ptr_pcl() );
    }

public:
    template< typename ...Keys >
    size_t any_down( Keys... keys ) const {
        size_t count = 0;

        ( ( count += ( _key_array[ keys ] == SURFKEY_STATE_DOWN ) ), ... );

        return count;
    }

    template< typename ...Keys >
    size_t tgl_down( Keys... keys ) const {
        size_t sum = 0;
        size_t at = 1;

        ( ( sum +=
            std::exchange( at, at << 1 )
            &
            ( _key_array[ keys ] == SURFKEY_STATE_DOWN )
        ), ... );

        return sum;
    }

    template< typename ...Keys >
    bool all_down( Keys... keys ) const {
        return this->any_down( keys... ) == sizeof...( Keys );
    }

    bool down( SurfKey key ) const {
        return _key_array[ key ] == SURFKEY_STATE_DOWN;
    }

public:
    HWND hwnd() {
        return _hwnd;
    }

public:
    Surface& force_plug() {
        SendMessage( _hwnd, _SURFACE_EVENT_FORCE, WPARAM{}, LPARAM{} );

        return *this;
    }

public:
#if defined( _ENGINE_UNIQUE_SURFACE )
    static Surface* get() {
        return _ptr;
    }
#endif

};



class Mouse {
public:
#if defined( _ENGINE_UNIQUE_SURFACE )
    static Vec2 pointer() {
        return Surface::get()->_pointer;
    }

    static Crd2 crd() {
        return Surface::get()->pull_crd( pointer() );
    }
#endif

public:
    static Vec2 g_vec() {
        auto [ x, y ] = g_crd();

        return { 
            x ,//- Env::width_2(), 
            -y //+ Env::height_2()
        };
    }

    static Crd2 g_crd() {
        POINT p;
        GetCursorPos( &p );

        return { p.x, p.y };
    }

};



};
