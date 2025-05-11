/*
*/
#pragma once

#include <IXN/descriptor.hpp>
#include <IXN/comms.hpp>
#include <IXN/geomet.hpp>
#include <IXN/env.hpp>



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

        SPACE = 32,

        F1 = 
#if defined( _ENGINE_SURFACE_GLFW )
        GLFW_KEY_F1,
#else
        112,
#endif 
        F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

#if defined( _ENGINE_SURFACE_GLFW )
        LCTRL     = GLFW_KEY_LEFT_CONTROL, 
        RCTRL     = GLFW_KEY_RIGHT_CONTROL,
        LSHIFT    = GLFW_KEY_LEFT_SHIFT, 
        RSHIFT    = GLFW_KEY_RIGHT_SHIFT,
        LALT      = GLFW_KEY_LEFT_ALT, 
        RALT      = GLFW_KEY_RIGHT_ALT,
        TAB       = GLFW_KEY_TAB,
        CAPS      = GLFW_KEY_CAPS_LOCK,
        ESC       = GLFW_KEY_ESCAPE,
        BACKSPACE = GLFW_KEY_BACKSPACE,
        ENTER     = GLFW_KEY_ENTER,
        DOT       = GLFW_KEY_PERIOD,
        COMMA     = GLFW_KEY_COMMA,
        COLON     = GLFW_KEY_SEMICOLON,
        DASH      = GLFW_KEY_MINUS,
        APOSTH    = GLFW_KEY_APOSTROPHE,
        EQUAL     = GLFW_KEY_EQUAL,
        GRAVE     = GLFW_KEY_GRAVE_ACCENT,
        LBRACKET  = GLFW_KEY_LEFT_BRACKET,
        RBRACKET  = GLFW_KEY_RIGHT_BRACKET,
        BACKSLASH = GLFW_KEY_BACKSLASH,
        SLASH     = GLFW_KEY_SLASH,
        LEFT      = GLFW_KEY_LEFT,
        UP        = GLFW_KEY_UP,
        RIGHT     = GLFW_KEY_RIGHT,
        DOWN      = GLFW_KEY_DOWN
#else
        LCTRL     = 17, 
        RCTRL     = LCTRL, 
        SHIFT     = 16, 
        ALT       = 18, 
        TAB       = 9, 
        CAPS      = 20, 
        ESC       = 27, 
        BACKSPACE = 8, 
        ENTER     = 13,
        DOT       = 190, 
        COMMA     = 188, 
        COLON     = 186, 
        APOSTH    = 222, 
        DASH      = 189, 
        EQUAL     = 187, 
        GRAVE     = 192,
        LBRACKET  = 219, 
        RBRACKET  = 221, 
        BACKSLASH = 220, 
        SLASH     = 191,
        LEFT      = 37, 
        UP        = 38, 
        RIGHT     = 39, 
        DOWN      = 40
#endif

    };

    static constexpr size_t   COUNT   = 
#if defined( _ENGINE_SURFACE_GLFW )
    GLFW_KEY_LAST
#else
    262
#endif
    ;

public:
    SurfKey() = default;

    SurfKey( int16_t key )
    : value( key )
    {}

public:
    int16_t   value   = NONE;

public:
    operator int16_t () const {
        return value;
    }

public:
    int8_t high() const {
        return static_cast< int8_t >( value >> 8 );
    }

    int8_t low() const {
        return static_cast< int8_t >( value );
    }

public:
    bool operator == ( const SurfKey& other ) const {
        return value == other.value;
    }

    bool operator == ( VALUE val ) const {
        return value == val;
    }

    bool operator == ( const char c ) const {
        return this->low() == c;
    }

    std::strong_ordering operator <=> ( const SurfKey& other ) const {
        return value <=> other.value;
    }

    std::strong_ordering operator <=> ( VALUE val ) const {
        return value <=> val;
    }

    std::strong_ordering operator <=> ( const char c ) const {
        return this->low() <=> c;
    }

public:
#if defined( _ENGINE_UNIQUE_SURFACE )
    template< typename ...Keys > static DWORD down_any( Keys... keys );

    template< typename ...Keys > static DWORD down_tgl( Keys... keys );

    template< typename ...Keys > static DWORD down_all( Keys... keys );

    static DWORD down( SurfKey key );
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
    SURFACE_EVENT_POINTER, 
    SURFACE_EVENT_SCROLL, 
    SURFACE_EVENT_FILEDROP, 
    SURFACE_EVENT_MOVE, 
    SURFACE_EVENT_RESIZE,
    SURFACE_EVENT_DESTROY,

    _SURFACE_EVENT_DESTROY = 69100,
    _SURFACE_EVENT_CURSOR_HIDE, 
    _SURFACE_EVENT_CURSOR_SHOW,
    _SURFACE_EVENT_FORCE_SOCKET_PTR
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
    template< SURFACE_EVENT event, typename ...Args >
    void invoke_sequence( SurfaceTrace& trace, Args&&... args ) {
        auto [ on, sckt ] = this->_seq_from_event< event >();

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
        if constexpr( event == SURFACE_EVENT_POINTER ) 
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

class SurfacePointerSentry {
_ENGINE_PROTECTED:
    Vec2   _pointer        = {};
    Vec2   _prev_pointer   = {};
public:
    Vec2 ptr_v() const {
        return _pointer;
    }

    Vec2 ptr_pv() const {
        return _prev_pointer;
    }

    Crd2 ptr_c() const {
        return pull_normal_axis( _pointer );
    }

    Crd2 ptr_pc() const {
        return pull_normal_axis( _prev_pointer );
    }

public:
    void ptr_reset() {
        _pointer = _prev_pointer = Vec2::O();
    }

};

enum SURFACE_STYLE {
    SURFACE_STYLE_LIQUID = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE,
    SURFACE_STYLE_SOLID  = WS_POPUP | WS_VISIBLE

};

#define _SURFACE_EPEF( name, ... ) inline static DWORD name( Surface* that, ## __VA_ARGS__ )

class Surface : public Descriptor,
                public SurfaceEventSentry,
                public SurfacePointerSentry
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
        echo( this, EchoLevel_Ok ) << "Set.";
    }

    Surface(
        std::string_view title,
        Crd2             pos      = { 0, 0 },
        Vec2             size     = { 512, 512 },
        SURFACE_THREAD   th_mode  = SURFACE_THREAD_THROUGH,
        SURFACE_STYLE    style    = SURFACE_STYLE_LIQUID,
        _ENGINE_COMMS_ECHO_ARG
    )
    : Surface{ title, pos, size, style, echo }
    {
        if( this->uplink( th_mode, echo ) ) 
            echo( this, EchoLevel_Ok ) << "Uplinked.";
        else
            echo( this, EchoLevel_Error ) << "Failure during uplink procedure.";
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

#if defined( _ENGINE_SURFACE_WIN32 )
    HWND                     _hwnd           = NULL;
    WNDCLASSEX               _wnd_class      = {};
#endif
#if defined( _ENGINE_SURFACE_GLFW )
    GLFWwindow*              _glfwnd         = nullptr;
#endif

    std::thread              _thread         = {};
    std::atomic< QWORD >     _thread_uplnk   = { false };

    Crd2                     _position       = {};
    Vec2                     _size           = {};
    SURFACE_STYLE            _style          = SURFACE_STYLE_LIQUID;
    std::string              _title          = {};

    SurfaceTrace             _trace          = {};

    _SurfKeyArray            _key_array      = { SURFKEY_STATE_UP };

_ENGINE_PROTECTED:
    void _main( std::binary_semaphore* sync, _ENGINE_COMMS_ECHO_ARG ) {
        struct _SyncAutoRelease {
            ~_SyncAutoRelease() {
                std::invoke( proc );
            }

            std::function< void( void ) >   proc   = nullptr;
        } sync_auto_release{ proc: [ &sync ] ( void ) -> void {
            if( sync ) std::exchange( sync, nullptr )->release();
        } };

    #if defined( _ENGINE_SURFACE_WIN32 )
        if( !RegisterClassEx( &_wnd_class ) ) {
            echo( this, EchoLevel_Error ) << "Bad window class registration.";
            return;
        }

        
        Vec2 adjusted = this->_adjust_size_for( _style );

        if( adjusted == Vec2::O() )
            echo( this, EchoLevel_Warning ) << "Bad window size adjustment.";


        _hwnd = CreateWindowEx(
            WS_EX_ACCEPTFILES,
            _wnd_class.lpszClassName, _wnd_class.lpszClassName,
            _style,
            _position.x, _position.y, adjusted.x, adjusted.y,
            NULL, NULL,
            GetModuleHandle( NULL ),
            this
        );

        if( !_hwnd ) {
            echo( this, EchoLevel_Error ) << "Bad window handle.";
            return;
        }

        SetWindowText( _hwnd, _wnd_class.lpszClassName );

    #endif
    #if defined( _ENGINE_SURFACE_GLFW )
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_SCALE_TO_MONITOR, GLFW_FALSE );
        glfwWindowHint( GLFW_SAMPLES, 4 );

        if( _style == SURFACE_STYLE_SOLID )
            glfwWindowHint( GLFW_DECORATED, GLFW_FALSE );

        _glfwnd = glfwCreateWindow( ( int )_size.x, ( int )_size.y, _title.c_str(), nullptr, nullptr );

        if( _glfwnd == nullptr ) {
            echo( this, EchoLevel_Error ) << "Bad window handle.";
            return;
        }

        glfwSetWindowUserPointer( _glfwnd, ( void* )this );
        glfwSetCursorPosCallback( _glfwnd, event_proc_ptr );
        glfwSetMouseButtonCallback( _glfwnd, event_proc_ptr_btn );
        glfwSetScrollCallback( _glfwnd, event_proc_scroll );
        glfwSetKeyCallback( _glfwnd, event_proc_key );
        glfwSetDropCallback( _glfwnd, event_proc_files );
        glfwSetWindowPosCallback( _glfwnd, event_proc_pos );
        glfwSetWindowSizeCallback( _glfwnd, event_proc_size );

        this->uplink_context_on_this_thread( echo );

        glewExperimental = GL_TRUE;
        if( glewInit() != GLEW_OK ) {
            echo( this, EchoLevel_Error ) << "OpenGL GLEW init fault.";
            return;
        }

    #if defined( _ENGINE_OS_WINDOWS )
        
    #endif

        this->downlink_context_on_this_thread( echo );

    #endif

        echo( this, EchoLevel_Ok ) << ( sync ? "Created across." : "Created through." );

        sync_auto_release.proc();

    #if defined( _ENGINE_SURFACE_WIN32 )
        MSG event;

        while( GetMessage( &event, NULL, 0, 0 ) > 0 ) {
            TranslateMessage( &event );
            DispatchMessage( &event );
        }
    #endif
    #if defined( _ENGINE_SURFACE_GLFW )
        while( !glfwWindowShouldClose( _glfwnd ) ) {
            glfwWaitEvents();
        }
    #endif
    }

#if defined( _ENGINE_SURFACE_WIN32 )
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
            SurfKey value = static_cast< SurfKey >( w_param );

            _EPEF::key( this, value, state );
        };

        switch( event ) {
            case WM_CREATE: {
                _EPEF::create( this );
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
                while( ShowCursor( true ) < 0 );
            break; }

            case _SURFACE_EVENT_FORCE_SOCKET_PTR :{
                this->invoke_sequence< SURFACE_EVENT_POINTER >( _trace, _pointer, _pointer );
            break; }


            case WM_MOUSEMOVE: {
                Vec2 new_pointer = pull_normal_axis( ( Crd2 )( Crd2{ LOWORD( l_param ), HIWORD( l_param ) } / _size ) );

                _EPEF::pointer( this, new_pointer );

            break; }

            case WM_MOUSEWHEEL: {
                _EPEF::scroll( this, GET_WHEEL_DELTA_WPARAM( w_param ) < 0 ? SURFSCROLL_DIRECTION_DOWN : SURFSCROLL_DIRECTION_UP );

            break; }


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

                std::vector< std::string > paths;

                for( size_t n = 0; n < file_count; ++ n ) {
                    TCHAR path[ MAX_PATH ];

                    DragQueryFile( reinterpret_cast< HDROP >( w_param ), n, path, MAX_PATH );

                    paths.emplace_back( path );
                }

                _EPEF::files( this, paths );

            break; }


            case WM_MOVE: {
                Crd2 new_pos = { 
                    static_cast< ggfloat_t >( ( int16_t )LOWORD( l_param ) ), 
                    static_cast< ggfloat_t >( ( int16_t )HIWORD( l_param ) )
                };

                _EPEF::move( this, new_pos );

            break; }

            case WM_SIZE: {
                Vec2 new_size = { LOWORD( l_param ), HIWORD( l_param ) };
                
                _EPEF::size( this, new_size );

            break; }

        }

        return DefWindowProc( hwnd, event, w_param, l_param );
    }
#endif
#if defined( _ENGINE_SURFACE_GLFW )
    static void event_proc_ptr( GLFWwindow* glfwnd, double x, double y ) {
        Surface* that = ( Surface* )glfwGetWindowUserPointer( glfwnd );

        Vec2 new_pointer = pull_normal_axis( ( Crd2 )( Crd2{ ( ggfloat_t )x, ( ggfloat_t )y } / that->_size ) );

        _EPEF::pointer( that, new_pointer );
    }

    static void event_proc_scroll( GLFWwindow* glfwnd, double x, double y ) {
        _EPEF::scroll( ( Surface* )glfwGetWindowUserPointer( glfwnd ), y < 0 ? SURFSCROLL_DIRECTION_DOWN : SURFSCROLL_DIRECTION_UP );
    }

    static void event_proc_ptr_btn( GLFWwindow* glfwnd, int btn, int action, [[maybe_unused]]int ) {
        SURFKEY_STATE state;

        switch( action ) {
            case GLFW_PRESS: state = SURFKEY_STATE_DOWN; break;
            case GLFW_RELEASE: state = SURFKEY_STATE_UP; break;
            default: return;
        }

        _EPEF::key( ( Surface* )glfwGetWindowUserPointer( glfwnd ), SurfKey::LMB + btn, state );
    }

    static void event_proc_key( GLFWwindow* glfwnd, int key, [[maybe_unused]]int, int action, [[maybe_unused]]int ) {
        SURFKEY_STATE state;

        switch( action ) {
            case GLFW_PRESS: state = SURFKEY_STATE_DOWN; break;
            case GLFW_RELEASE: state = SURFKEY_STATE_UP; break;
            default: return;
        }

        _EPEF::key( ( Surface* )glfwGetWindowUserPointer( glfwnd ), key, state );
    }

    static void event_proc_files( GLFWwindow* glfwnd, int count, const char* paths_c_str[] ) {
        std::vector< std::string > paths;

        for( int n = 0; n < count; ++n )
            paths.emplace_back( paths_c_str[ n ] );

        _EPEF::files( ( Surface* )glfwGetWindowUserPointer( glfwnd ), paths );
    }

    static void event_proc_pos( GLFWwindow* glfwnd, int x, int y ) {
        Crd2 new_pos = { ( ggfloat_t )x, ( ggfloat_t )y };	

        _EPEF::move( ( Surface* )glfwGetWindowUserPointer( glfwnd ), new_pos );
    }

    static void event_proc_size( GLFWwindow* glfwnd, int w, int h ) {
        Crd2 new_size = { ( ggfloat_t )w, ( ggfloat_t )h };	

        _EPEF::size( ( Surface* )glfwGetWindowUserPointer( glfwnd ), new_size );
    }
#endif

    struct _EPEF {
        _SURFACE_EPEF( create ) {
            return 0;
        }

        _SURFACE_EPEF( destroy ) {
            return 0;
        }
        
        _SURFACE_EPEF( pointer, Vec2 new_pointer ) {
            that->invoke_sequence< SURFACE_EVENT_POINTER >( 
                that->_trace, new_pointer, that->_prev_pointer = std::exchange( that->_pointer, new_pointer ) 
            );
            return 0;
        }

        _SURFACE_EPEF( scroll, SURFSCROLL_DIRECTION dir ) {
            that->invoke_sequence< SURFACE_EVENT_SCROLL >( that->_trace, that->_pointer, dir );
            return 0;
        }

        _SURFACE_EPEF( key, SurfKey value, SURFKEY_STATE state ) {
            that->_key_array[ value ] = state;
            that->invoke_sequence< SURFACE_EVENT_KEY >( that->_trace, value, state );
            return 0;
        }

        _SURFACE_EPEF( files, std::vector< std::string > paths ) {
            that->invoke_sequence< SURFACE_EVENT_FILEDROP >( that->_trace, std::move( paths ) );
            return 0;
        }

        _SURFACE_EPEF( move, Vec2 new_pos ) {
            that->invoke_sequence< SURFACE_EVENT_MOVE >( 
                that->_trace, new_pos, std::exchange( that->_position, new_pos ) 
            );
            return 0;
        }

        _SURFACE_EPEF( size, Vec2 new_size ) {
            that->invoke_sequence< SURFACE_EVENT_RESIZE >( 
                that->_trace, new_size, std::exchange( that->_size, new_size ) 
            );
            return 0;
        }
    };

_ENGINE_PROTECTED:
    Vec2 _adjust_size_for( SURFACE_STYLE style ) const {
    #if defined( _ENGINE_SURFACE_WIN32 )
        RECT rect{ 
            static_cast< decltype( RECT::left ) >( _position.x ), 
            static_cast< decltype( RECT::top ) >( _position.y ),
            static_cast< decltype( RECT::right ) >( _position.x + _size.x ), 
            static_cast< decltype( RECT::bottom ) >( _position.y + _size.y ) 
        };

        if( !AdjustWindowRect( &rect, style, false ) )
            return { 256 };

        return { 
            static_cast< ggfloat_t >( rect.right - rect.left ), 
            static_cast< ggfloat_t >( rect.bottom - rect.top )
        };
    #endif

        return { 0 };
    }

    void _swap_style( SURFACE_STYLE style ) {
    #if defined( _ENGINE_SURFACE_WIN32 )
        SetWindowLongPtr( _hwnd, GWL_STYLE, _style = style );
    #endif
    }

public:
    bool uplink( SURFACE_THREAD th_mode = SURFACE_THREAD_THROUGH, _ENGINE_COMMS_ECHO_ARG ) {
#if defined( _ENGINE_UNIQUE_SURFACE )
         _ptr = this;
#endif

#if defined( _ENGINE_SURFACE_WIN32 )
        _wnd_class.cbSize        = sizeof( WNDCLASSEX );
        _wnd_class.hInstance     = GetModuleHandle( NULL );
        _wnd_class.lpfnWndProc   = event_proc_router_1;
        _wnd_class.lpszClassName = _title.data();
        _wnd_class.hbrBackground = HBRUSH( COLOR_INACTIVECAPTIONTEXT );
        _wnd_class.hCursor       = LoadCursor( NULL, IDC_ARROW );
#endif

        switch( th_mode ) {
            case SURFACE_THREAD_THROUGH: goto l_thread_through;

            case SURFACE_THREAD_ACROSS: goto l_thread_across;

            default: echo( this, EchoLevel_Error ) << "Bad thread mode argument."; return false;
        }

l_thread_through: 
        std::invoke( _main, this, nullptr, echo );
        return true;

l_thread_across: 
{
        std::binary_semaphore sync{ 0 };

        _thread = std::thread( _main, this, &sync, echo );

        if( _thread.joinable() ) {
            echo( this, EchoLevel_Pending ) << "Waiting for across window creation...";

            sync.acquire();
        } else {
            echo( this, EchoLevel_Error ) << "Main thread bad invoke.";
            return false;
        }
} 
        return true;
    }

    void downlink( _ENGINE_COMMS_ECHO_ARG ) {
    #if defined( _ENGINE_SURFACE_WIN32 )
        SendMessage( _hwnd, _SURFACE_EVENT_DESTROY, WPARAM{}, LPARAM{} );

        if( !UnregisterClassA( _wnd_class.lpszClassName, GetModuleHandle( NULL ) ) )
            echo( this, EchoLevel_Error ) << "Bad window class unregistration.";
    #endif
    #if defined( _ENGINE_SURFACE_GLFW )
        glfwSetWindowShouldClose( _glfwnd, GL_TRUE );
        glfwPostEmptyEvent();
    #endif

        if( _thread.joinable() )
            _thread.join();
        
        #if defined( _ENGINE_UNIQUE_SURFACE )
            _ptr = nullptr;
        #endif
    }

public:
    DWORD uplink_context_on_this_thread( _ENGINE_COMMS_ECHO_RT_ARG ) {
        auto  this_id = std::this_thread::get_id();
        QWORD cmp_hs  = 0;
        QWORD this_hs = ( QWORD )std::hash< decltype( this_id ) >{}( this_id );

        if( false == _thread_uplnk.compare_exchange_strong( cmp_hs, this_hs, std::memory_order_relaxed ) ) {
            if( cmp_hs == this_hs )
                echo( this, EchoLevel_Warning ) << "Context uplink multiple times on thread \"" << this_id << "\". Ignored.";
            else
                echo( this, EchoLevel_Error ) << "Context uplink on thread \"" << this_id << "\", while another thread holds this surface's context.";
            return -1;
        }

    #if defined( _ENGINE_SURFACE_GLFW )
        glfwMakeContextCurrent( _glfwnd );
    #endif

        echo( this, EchoLevel_Ok ) << "Context uplink on thread \"" << this_id << "\".";

        return 0;
    }

    DWORD downlink_context_on_this_thread( _ENGINE_COMMS_ECHO_RT_ARG ) {
        auto  cmp_id  = std::this_thread::get_id();
        QWORD cmp_hs  = ( QWORD )std::hash< decltype( cmp_id ) >{}( cmp_id );
        
    #if defined( _ENGINE_SURFACE_GLFW )
        glfwMakeContextCurrent( NULL );
    #endif

        if( false == _thread_uplnk.compare_exchange_strong( cmp_hs, 0, std::memory_order_relaxed ) ) {
            if( cmp_hs == 0 )
                echo( this, EchoLevel_Warning ) << "Context donwlink multile times on thread \"" << cmp_id << "\". Ignored.";
            else
                echo( this, EchoLevel_Error ) << "Context downlink on thread " << cmp_id << ", while another thread holds this surface's context.";
            return -1;
        }

        echo( this, EchoLevel_Ok ) << "Context downlink on thread \"" << cmp_id << "\".";

        return 0;
    }

public:
    auto& localize( is_vec2_base auto& vec ) const {
        vec *= _size;
        return vec;
    }

    template< is_vec2_base T >
    T localized( const T vec ) const {
        T res{ vec };
        return this->localize( res );
    }

    auto& globalize( is_vec2_base auto& vec ) const {
        vec /= _size;
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

    ggfloat_t aspect() const {
        return _size.x / _size.y;
    }

public:
    Surface& solidify() {
        this->_swap_style( SURFACE_STYLE_SOLID );
        return *this;
    }

    Surface& liquify() {
        this->_swap_style( SURFACE_STYLE_LIQUID );
        return *this;
    }

    Surface& relocate( Vec2 pos ) {
        _position = pos;

    #if defined( _ENGINE_SURFACE_WIN32 )
        SetWindowPos(
            _hwnd,
            0,
            pos.x, pos.y,
            0, 0,
            SWP_NOSIZE
        );
    #endif
    #if defined( _ENGINE_SURFACE_GLFW )
        glfwSetWindowPos( _glfwnd, ( int )pos.x, ( int )pos.y );
    #endif

        return *this;
    }

    Surface& resize( Vec2 size ) {
        _size = size;

        auto adj = this->_adjust_size_for( _style );

    #if defined( _ENGINE_SURFACE_WIN32 )
        SetWindowPos(
            _hwnd,
            0,
            0, 0,
            adj.x, adj.y,
            SWP_NOMOVE
        );
    #endif
    #if defined( _ENGINE_SURFACE_GLFW )
        glfwSetWindowSize( _glfwnd, ( int )size.x, ( int )size.y );
    #endif

        return *this;
    }

public:
    Surface& hide_def_ptr() {
    #if defined( _ENGINE_SURFACE_WIN32 )
        SendMessage( _hwnd, _SURFACE_EVENT_CURSOR_HIDE, WPARAM{}, LPARAM{} );
    #endif
    #if defined( _ENGINE_SURFACE_GLFW )
        glfwSetInputMode( _glfwnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
    #endif

        return *this;
    }

    Surface& show_def_ptr() {
    #if defined( _ENGINE_SURFACE_WIN32 )
        SendMessage( _hwnd, _SURFACE_EVENT_CURSOR_SHOW, WPARAM{}, LPARAM{} );
    #endif
    #if defined( _ENGINE_SURFACE_GLFW )
        glfwSetInputMode( _glfwnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
    #endif

        return *this;
    }

public:
    Vec2 ptr_vl() const {
        return this->localized( _pointer );
    }

    Vec2 ptr_pvl() const {
        return this->localized( _prev_pointer );
    }
    
    Crd2 ptr_cl() const {
        return this->localized( this->ptr_c() );
    }

    Crd2 ptr_pcl() const {
        return this->localized( this->ptr_pc() );
    }

public:
    template< typename ...Keys >
    size_t down_any( Keys... keys ) const {
        size_t count = 0;

        ( ( count += ( _key_array[ keys ] == SURFKEY_STATE_DOWN ) ), ... );

        return count;
    }

    template< typename ...Keys >
    size_t down_tgl( Keys... keys ) const {
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
    DWORD down_all( Keys... keys ) const {
        return this->down_any( keys... ) == sizeof...( Keys );
    }

    DWORD down( SurfKey key ) const {
        return _key_array[ key.value ] == SURFKEY_STATE_DOWN;
    }

public:
#if defined( _ENGINE_SURFACE_WIN32 )
    HWND handle() {
        return _hwnd;
    }
#endif
#if defined( _ENGINE_SURFACE_GLFW )
    GLFWwindow* handle() {
        return _glfwnd;
    }
#endif
    operator decltype( std::declval< Surface >().handle() ) () {
        return this->handle();
    }

public:
    Surface& force_socket_ptr() {
    #if defined( _ENGINE_SURFACE_WIN32 )
        SendMessage( _hwnd, _SURFACE_EVENT_FORCE_SOCKET_PTR, WPARAM{}, LPARAM{} );
    #endif

        return *this;
    }

public:
#if defined( _ENGINE_UNIQUE_SURFACE )
    static Surface* get() {
        return _ptr;
    }
#endif

};



class SurfPtr {
public:
#if defined( _ENGINE_UNIQUE_SURFACE )
    static Vec2 vl() {
        return Surface::get()->ptr_vl();
    }

    static Vec2 vg() {
        return Surface::get()->ptr_vg();
    }

    static Vec2 pvl() {
        return Surface::get()->ptr_pvl();
    }

    static Vec2 pvg() {
        return Surface::get()->ptr_pvg();
    }
    

    static Crd2 cl() {
        return Surface::get()->ptr_cl();
    }

    static Crd2 cg() {
        return Surface::get()->ptr_cg();
    }

    static Crd2 pcl() {
        return Surface::get()->ptr_pcl();
    }

    static Crd2 pcg() {
        return Surface::get()->ptr_pcg();
    }

#endif

public:
    static Vec2 env_v() {
        auto [ x, y ] = SurfPtr::env_c();

        return { 
            x - Env::w(.5), 
            -y + Env::h(.5)
        };
    }

    static Crd2 env_c() {
        POINT p; GetCursorPos( &p );

        return { static_cast< ggfloat_t >( p.x ), static_cast< ggfloat_t >( p.y ) };
    }

    static void env_to( Vec2 vec ) {
        SetCursorPos( vec.x + Env::w(.5), -vec.y + Env::h(.5) );
    }

    static void env_to( Crd2 crd ) {
        SetCursorPos( crd.x * Env::width(), crd.y * Env::height() );
    }

};



};
