#pragma region DETAILS
/*
    [ GENERAL ]
        Engine:      Nine Tailed Works

        Version:     n/a

        Hackerman:   Mipsan

        C++:         2023's standard

        OSs:         Windows

        GLs:         DirectX

    [ PRE-DEFINES ]
        _9T_ECHO --- logs stuff.
        _9T_UNIQUE_SURFACE --- Enables faster event routing when using only one surface.

    [ GCC FLAGS ]
        -std=gnu++23
        -static-libgcc
        -static-libc++

    [ LINKERS ]
        -lwinmm
        -lwindowscodecs
        -lole32
        -lComdlg32

        -ld2d1

*/
#pragma endregion DETAILS



#define _9T_OS_WINDOWS
#define _9T_GL_DIRECT



#pragma region DEFINES



#define PI 3.141592653


#define _ENGINE_NAMESPACE _9T

#define _ENGINE_UTH_ADD_PREFIX( type ) "_9T::" type

#define _ENGINE_UTH_IDENTIFY_METHOD( type ) virtual std::string_view type_name() const override { return _ENGINE_UTH_ADD_PREFIX( type ); }


#if defined( _9T_ECHO )
        #define _ENGINE_ECHO
#endif

#if defined( _9T_UNIQUE_SURFACE )
    #define _ENGINE_UNIQUE_SURFACE
#endif

#if defined( _9T_THROW_ON_FAULT )
    #define ENGINE_THROW_ON_FAULT
#endif


#if defined( _9T_OS_WINDOWS )
    #define _ENGINE_OS_WINDOWS
#endif

#if defined( _9T_GL_DIRECT )
    #define _ENGINE_GL_DIRECT
#endif



#pragma endregion DEFINES



#pragma region INCLUDES



#include <iostream>
#include <fstream>
#include <filesystem>

#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <map>
#include <unordered_map>
#include <set>

#include <algorithm>
#include <utility>
#include <cmath>

#include <functional>
#include <concepts>
#include <optional>
#include <tuple>
#include <variant>
#include <bitset>
#include <string>
#include <string_view>

#include <memory>
#include <thread>
#include <chrono>
#include <future>
#include <semaphore>
#include <atomic>
#include <condition_variable>

#include <type_traits>
#include <typeindex>

#if defined( _ENGINE_OS_WINDOWS )
    #include <windows.h>
    #include <wincodec.h>
#endif

#if defined( _ENGINE_GL_DIRECT )
    #include <d2d1.h>
    /*#include <d3d12.h>*/
#endif





#pragma endregion INCLUDES



#pragma region OS
namespace OS {



#if defined( _ENGINE_OS_WINDOWS )
    enum CONSOLE_COLOR_CODE {
        DARK_GRAY = 8,
        DARK_BLUE = 9,
        GREEN = 10,
        RED = 12,
        PINK = 13,
        YELLOW = 14,
        WHITE = 15
    };



    void console_color_to( const auto& code ) {
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), code );
    }
#endif


};
#pragma endregion OS



namespace _ENGINE_NAMESPACE {



class UTH;

class Deg;
class Rad;

class Vec2;
class Ray2;
class Clust2;

class Key;
class Mouse;
class Surface;
class Renderer2;
class Chroma;
class Brush2;
class SolidBrush2;

class Audio;
class Sound;
class Synth;



#pragma region Echo



#if defined( _ENGINE_ECHO )
    #define _ECHO_LITERAL( op, code ) \
        const char* operator "" op ( const char* str, [[ maybe_unused ]] size_t unused ) { \
            OS::console_color_to( code ); \
            return str; \
        }

    _ECHO_LITERAL( _echo_normal, OS::CONSOLE_COLOR_CODE::WHITE )
    _ECHO_LITERAL( _echo_highlight, OS::CONSOLE_COLOR_CODE::DARK_GRAY )
    _ECHO_LITERAL( _echo_special, OS::CONSOLE_COLOR_CODE::DARK_BLUE )
#endif

enum ECHO_LOG {
    ECHO_LOG_FAULT   = OS::CONSOLE_COLOR_CODE::RED, 
    ECHO_LOG_WARNING = OS::CONSOLE_COLOR_CODE::YELLOW, 
    ECHO_LOG_OK      = OS::CONSOLE_COLOR_CODE::GREEN, 
    ECHO_LOG_PENDING = OS::CONSOLE_COLOR_CODE::DARK_BLUE, 
    ECHO_LOG_HEADSUP = OS::CONSOLE_COLOR_CODE::PINK
};

class Echo {
public:
    Echo() = default;

    Echo( const Echo& other )
    : _depth{ other._depth + 1 }
    {}

    ~Echo() {
        if( _depth == 0 )
            std::cout << '\n';
    }

protected:
    size_t   _depth   = 0;

public:
    const Echo& operator () (
        UTH*               invoker,
        ECHO_LOG           log_type,
        std::string_view   message
    ) const {
        return std::invoke( _route, this, invoker, log_type, message );
    }

protected:
    const Echo& _echo(
        UTH*               invoker,
        ECHO_LOG           log_type,
        std::string_view   message
    ) const;

    const Echo& _nop(
        UTH*               invoker,
        ECHO_LOG           log_type,
        std::string_view   message
    ) const {
        return *this;
    }

protected:
    inline static auto   _route   = &_echo;

public:
    static void high() {
        _route = &_echo;
    }

    static void low() {
        _route = &_nop;
    }

protected:
    static std::string_view _log_type_name( ECHO_LOG log_type ) {
        switch( log_type ) {
            case ECHO_LOG_FAULT:   return "FAULT";
            case ECHO_LOG_WARNING: return "WARNING";
            case ECHO_LOG_OK:      return "OK";
            case ECHO_LOG_PENDING: return "PENDING";
            case ECHO_LOG_HEADSUP: return "HEADSUP";
        }

        return "N/A";
    }

    static std::string_view _log_type_fill( ECHO_LOG log_type ) {
        switch( log_type ) {
            case ECHO_LOG_FAULT:   return "  ";
            case ECHO_LOG_WARNING: return "";
            case ECHO_LOG_OK:      return "     ";
            case ECHO_LOG_PENDING: return "";
            case ECHO_LOG_HEADSUP: return "";
        }

        return "";
    }

};



#if defined( ENGINE_THROW_ON_FAULT )
    #define ECHO_ASSERT_AND_THROW( cond, message ) { \
        if( !( cond ) ) { \
            echo( this, ECHO_LOG_FAULT, message ); \
            throw std::runtime_error{ message }; \
        } \
    }
#else
    #define ECHO_ASSERT_AND_THROW( cond, message ) { \
        if( !( cond ) ) { \
            echo( this, ECHO_LOG_FAULT, message ); \
            return; \
        } \
    }
#endif



#pragma endregion Echo



#pragma region Tricks


    
template< typename Type >
using Ref = Type&;

template< typename Type >
using Ptr = Type*;



template< template< typename Type > typename Smart_ptr, typename Type, typename Derived_ptr >
class _Smart_ptr_extended : public Smart_ptr< Type > {
public:
    using _Base = Smart_ptr< Type >;
    
public:
    using _Base::_Base;

public:
    inline static constexpr bool   is_array   = std::is_array_v< Type >;

public:
    typedef   std::conditional_t< is_array, std::decay_t< Type >, Type* >   Type_ptr;
    typedef   std::remove_pointer_t< Type_ptr >&                              Type_ref;

public:
    Derived_ptr& operator = ( const Type_ptr ptr ) {
        this->reset( ptr );

        return static_cast< Derived_ptr& >( *this );
    }

public:
    operator Type_ptr () {
        return this->get();
    }

    template< bool isnt_array = !is_array >
    operator std::enable_if_t< isnt_array, Type_ref > () {
        return *this->get();
    }

public:
    Type_ptr operator + ( ptrdiff_t offset ) const {
        return this->get() + offset;
    }

    Type_ptr operator - ( ptrdiff_t offset ) const {
        return this->get() - offset;
    }

};


template< typename Type >
class Unique : public _Smart_ptr_extended< std::unique_ptr, Type, Unique< Type > > {
public:
    typedef   _Smart_ptr_extended< std::unique_ptr, Type, Unique< Type > >   _Base;

public:
    using _Base::_Base;

    using _Base::operator =;

};


template< typename Type >
class Shared : public _Smart_ptr_extended< std::shared_ptr, Type, Shared< Type > > {
public:
    typedef   _Smart_ptr_extended< std::shared_ptr, Type, Shared< Type > >   _Base;

public:
    using _Base::_Base;

    using _Base::operator =;

};



struct VoidStruct {
    template< typename ...Args >
    VoidStruct( Args&&... ) {}
};



#pragma endregion Tricks



#pragma region Syncs



template< typename T >
requires ( std::is_arithmetic_v< T > )
void wait_for( T duration ) {
    if constexpr( std::is_floating_point_v< T > )
        std::this_thread::sleep_for( std::chrono::milliseconds( static_cast< int64_t >( duration * 1000.0 ) ) );
    else
        std::this_thread::sleep_for( std::chrono::milliseconds( static_cast< int64_t >( duration ) ) );
}



class Clock {
public:
    Clock()
    : _create{ std::chrono::high_resolution_clock::now() },
    _last_lap{ std::chrono::high_resolution_clock::now() }
    {}

public:
    inline static constexpr double M[] = { 
        1'000'000'000.0, 1'000'000, 1'000,
        1.0, 
        1.0 / 60.0, 1.0 / 3'600.0 
    };

    enum Unit {
        NANOS = 0, MICROS, MILLIS, SECS, MINS, HOURS
    };

protected:
    std::chrono::high_resolution_clock::time_point   _create     = {};
    std::chrono::high_resolution_clock::time_point   _last_lap   = {};

public:
    template< Unit unit = SECS >
    double up_time() const {
        using namespace std::chrono;

        return duration< double >( high_resolution_clock::now() - _create ).count() * M[ unit ];
    }

    template< Unit unit = SECS >
    double peek_lap() const {
        using namespace std::chrono; 

        return duration< double >( high_resolution_clock::now() - _last_lap ).count() * M[ unit ];
    }

    template< Unit unit = SECS >
    double lap(){
        using namespace std::chrono;

        auto now = high_resolution_clock::now();

        return duration< double >( now - std::exchange( _last_lap, now ) ).count() * M[ unit ];
    }

protected:
    template< Unit unit >
    struct _Map {
        typedef std::conditional_t< unit == NANOS,  std::chrono::nanoseconds,
                std::conditional_t< unit == MICROS, std::chrono::microseconds,
                std::conditional_t< unit == MILLIS, std::chrono::milliseconds,
                std::conditional_t< unit == SECS,   std::chrono::seconds,
                std::conditional_t< unit == MINS,   std::chrono::minutes,
                std::conditional_t< unit == HOURS,  std::chrono::hours, 
                void > > > > > > type;
    };

public:
    template< Unit unit = SECS >
    static auto epoch() {
        using namespace std::chrono;

        return duration_cast< typename _Map< unit >::type >(
            high_resolution_clock::now().time_since_epoch()
        ).count();
    }

};



template< typename T >
class Controller {
public:
    Controller() = default;


    Controller( const T& val )
        : _value( val )
    {}

    Controller( T&& val ) noexcept
        : _value( std::move( val ) )
    {}


    Controller( const Controller< T >& other ) = delete;

    Controller( Controller< T >&& other ) = delete;


    ~Controller() {
        release();
    }

protected:
    typedef std::tuple<
                Unique< std::mutex >,
                Unique< std::condition_variable >,
                std::function< bool( const T& ) >
            > Entry;

    enum _TUPLE_ACCESS_INDEX {
        _MTX = 0, _CND = 1, _OP = 2
    };

protected:
    mutable T            _value      = {};
    std::mutex           _sync_mtx   = {};
    std::list< Entry >   _entries    = {};

public:
    operator typename std::enable_if_t< std::is_copy_constructible_v< T >, T > () const {
        return _value;
    }

public:
    Controller& operate( std::function< bool( T& ) > op ) {
        std::unique_lock< std::mutex > sync_lock( _sync_mtx );

        std::invoke( op, _value );

        for( Entry& entry : _entries )
            if( std::get< _OP >( entry )( _value ) )
                std::get< _CND >( entry )->notify_all();

        return *this;
    }

    Controller& operator () ( std::function< bool( T& ) > op ) {
        return operate( op );
    }

public:
    Controller& wait_until( std::function< bool( const T& ) > cnd ) {
        std::unique_lock< std::mutex > sync_lock( _sync_mtx );

        if( std::invoke( cnd, _value ) ) return *this;


        _entries.emplace_back(
            std::make_unique< std::mutex >(),
            new std::condition_variable,
            cnd
        );

        auto entry = _entries.rbegin();

        sync_lock.unlock();


        std::unique_lock< std::mutex > lock( *std::get< _MTX >( *entry ) );

        std::get< _CND >( *entry )->wait( lock );

        lock.unlock();
        lock.release();

        _entries.erase( entry );

        return *this;
    }

public:
    Controller& release() {
        for( Entry& entry : _entries )
            std::get< _CND >( entry )->notify_all();

        _entries.clear();
    }

};



#pragma endregion Syncs



#pragma region Utility



template< typename T = float >
struct Coord {
    Coord() = default;

    Coord( T x, T y )
    : x{ x }, y{ y }
    {}

    template< typename TOther >
    Coord( const Coord< TOther >& other )
    : x{ static_cast< T >( other.x ) }, y{ static_cast< T >( other.y ) }
    {}


    T   x   = {};
    T   y   = {};


    template< bool is_float = std::is_same_v< float, T > >
    operator std::enable_if_t< is_float, const D2D1_POINT_2F& > () const {
        return *reinterpret_cast< const D2D1_POINT_2F* >( this );
    }

    template< bool is_float = std::is_same_v< float, T > >
    operator std::enable_if_t< is_float, D2D1_POINT_2F& > () {
        return *reinterpret_cast< D2D1_POINT_2F* >( this );
    }

};

template< typename T = float >
struct Size {
    Size() = default;

    Size( T width, T height )
        : width( width ), height( height )
    {}

    template< typename TOther >
    Size( const Size< TOther >& other )
        : width( static_cast< T >( other.width ) ), height( static_cast< T >( other.height ) )
    {}

    T   width    = {};
    T   height   = {};

};



class File {
    public:
        static std::string dir_of( std::string_view path ) {
            return path.substr( 0, path.find_last_of( "/\\" ) ).data();
        }

        static std::string name_of( std::string_view path ) {
            return path.substr( path.find_last_of( "/\\" ) + 1, path.size() - 1 ).data();
        }

    public:
        static size_t size( std::string_view path ) {
            std::ifstream file( path.data(), std::ios_base::binary );

            return size( file );
        }

        static size_t size( std::ifstream& file ) {
            file.seekg( 0, std::ios_base::end );

            size_t sz = file.tellg();

            file.seekg( 0, std::ios_base::beg );

            return sz;
        }

    public:
        static std::string browse( std::string_view title ) {
            char path[ MAX_PATH ];

            OPENFILENAME hf;

            std::fill_n( path, sizeof( path ), 0 );
            std::fill_n( reinterpret_cast< char* >( &hf ), sizeof( hf ), 0 );

            hf.lStructSize = sizeof( hf );
            hf.hwndOwner   = GetFocus();
            hf.lpstrFile   = path;
            hf.nMaxFile    = MAX_PATH;
            hf.lpstrTitle  = title.data();
            hf.Flags       = OFN_EXPLORER | OFN_NOCHANGEDIR;

            GetOpenFileName( &hf );

            return path;
        }

        static std::string save( std::string_view title ) {
            char path[ MAX_PATH ];

            OPENFILENAME hf;

            std::fill_n( path, sizeof( path ), 0 );
            std::fill_n( reinterpret_cast< char* >( &hf ), sizeof( hf ), 0 );

            hf.lStructSize = sizeof( hf );
            hf.hwndOwner   = GetFocus();
            hf.lpstrFile   = path;
            hf.nMaxFile    = MAX_PATH;
            hf.lpstrTitle  = title.data();
            hf.Flags       = OFN_EXPLORER | OFN_NOCHANGEDIR;

            GetSaveFileName( &hf );

            return path;
        }

    public:
        template< typename Itr >
        static std::optional< ptrdiff_t > next_idx(
            std::ifstream& file, std::string& str,
            Itr begin, Itr end
        ) {
            if( !( file >> str ) ) return {};

            return std::distance(
                begin,
                std::find_if( begin, end, [ &str ] ( const decltype( *begin )& entry ) -> bool {
                    return str == entry;
                } )
            );
        }

        template< typename Itr >
        static void auto_nav(
            std::ifstream& file,
            Itr begin, Itr end,
            std::function< void( ptrdiff_t, std::string& ) > func
        ) {
            std::string str = {};

            for(
                auto idx = next_idx( file, str, begin, end );
                idx.has_value();
                idx = next_idx( file, str, begin, end )
            ) {
                std::invoke( func, idx.value(), str );
            }
        }

    };



class Bytes {
public:
    enum Endianess {
        LITTLE, BIG
    };

public:
    template< typename T >
    static T as( char* array, size_t byte_count, Endianess end ) {
        char bytes[ sizeof( T ) ];

        const bool is_negative =
            ( *reinterpret_cast< char* >( array + ( end == LITTLE ? byte_count - 1 : 0 ) ) ) >> 7
            &&
            std::is_signed_v< T >;

        for( size_t n = byte_count; n < sizeof( T ); ++n )
            bytes[ n ] = is_negative ? -1 : 0;

        for( size_t n = 0; n < byte_count && n < sizeof( T ); ++n )
            bytes[ n ] = array[ end == LITTLE ? n : byte_count - n - 1 ];

        return *reinterpret_cast< T* >( &bytes );
    }
};



class Env {
public:
    static int width() {
        static int value = ( [] () -> int {
            RECT rect;
            GetWindowRect( GetDesktopWindow(), &rect );

            return rect.right;
        } )();

        return value;
    }

    static int width_2() {
        return width() / 2;
    }

    static int width_3() {
        return width() / 3;
    }

    static int width_4() {
        return width() / 4;
    }

    static int height() {
        static int value = ( [] () -> int {
            RECT rect;
            GetWindowRect( GetDesktopWindow(), &rect );

            return rect.bottom;
        } )();

        return value;
    }

    static int height_2() {
        return height() / 2;
    }

    static int height_3() {
        return height() / 3;
    }

    static int height_4() {
        return height() / 4;
    }

    static float diag() {
        static float value = sqrt( width() * width() + height() * height() );

        return value;
    }

    static float diag_2() {
        return diag() / 2.0;
    }

    static float aspect() {
        return static_cast< float >( width() ) / height();
    }

    static std::string_view dir() {
        static std::string value = ( [] () -> std::string {
            char path[ MAX_PATH ];

            GetModuleFileNameA( GetModuleHandle( NULL ), path, MAX_PATH );

            return File::dir_of( path );
        } )();

        return value;
    }

    static std::string_view process() {
        static std::string value = ( [] () -> std::string {
            char path[ MAX_PATH ];

            GetModuleFileNameA( GetModuleHandle( NULL ), path, MAX_PATH );

            return File::name_of( path );
        } )();

        return value;
    }
};



#pragma endregion Utility



#pragma region UTH



typedef   const void*   GUID;

class UTH {
public:
    virtual std::string_view type_name() const {
        return _ENGINE_UTH_ADD_PREFIX( "UTH" );
    }

public:
    GUID guid() const { 
        return static_cast< GUID >( this ); 
    }

public:
    virtual void trigger( UTH&, void* ) {}

};



#pragma endregion UTH



#pragma region Space



enum HEADING {
    HEADING_NORTH = 0, 
    HEADING_EAST, 
    HEADING_SOUTH, 
    HEADING_WEST
};

enum SYSTEM {
    SYSTEM_LOCAL = 0, 
    SYSTEM_GLOBAL
};



class Deg {
public:
    static double pull( double theta ) {
        return theta * ( 180.0 / PI );
    }

    static void push( double& theta ) {
        theta *= ( 180.0 / PI );
    }
};

class Rad {
public:
    static double pull( double theta ) {
        return theta * ( PI / 180.0 );
    }

    static void push( double& theta ) {
        theta *= ( PI / 180.0 );
    }
};



#pragma region D2



class Vec2 {
public:
    Vec2() = default;

    Vec2( double x, double y )
    : x{ x }, y{ y }
    {}

    Vec2( double x )
    : Vec2{ x, x }
    {}

public:
    double   x   = 0.0;
    double   y   = 0.0;

public:
    double dot( const Vec2& other ) const {
        return x * other.x + y * other.y;
    }

public:
    double mag_sq() const {
        return x * x + y * y;
    }

    double mag() const {
        return sqrt( this->mag_sq() );
    }

    double angel() const {
        return Deg::pull( atan2( y, x ) );
    }

public:
    double dist_sq_to( const Vec2& other ) const {
        return ( other.x - x ) * ( other.x - x ) + ( other.y - y ) * ( other.y - y );
    }

    double dist_to( const Vec2& other ) const {
        return sqrt( this->dist_sq_to( other ) );
    }

public:
    Vec2 respect_to( const Vec2& other ) const {
        return { x - other.x, y - other.y };
    }

    Vec2 operator () ( const Vec2& other ) const {
        return this->respect_to( other );
    }

public:
    Vec2& polar( double angel, double dist ) {
        Rad::push( angel );

        x += cos( angel ) * dist;
        y += sin( angel ) * dist;

        return *this;
    }

    Vec2 polared( double angel, double dist ) const {
        return Vec2{ *this }.polar( angel, dist );
    }


    Vec2& approach( const Vec2 other, double dist ) {
        return this->polar( other.respect_to( *this ).angel(), dist );
    }

    Vec2 approached( const Vec2 other, double dist ) const {
        return Vec2{ *this }.approach( other, dist );
    }


    Vec2& spin( double theta ) {
        Rad::push( theta );

        double nx = x * cos( theta ) - y * sin( theta );
        y = x * sin( theta ) + y * cos( theta );
        x = nx;

        return *this;
    }

    Vec2& spin( double theta, const Vec2& other ) {
        *this = this->respect_to( other ).spin( theta ) + other;

        return *this;
    }

    Vec2 spinned( double theta ) const {
        return Vec2{ *this }.spin( theta );
    }

    Vec2 spinned( double theta, const Vec2& other ) const {
        return Vec2{ *this }.spin( theta, other );
    }

public:
    bool is_further_than( const Vec2& other, HEADING heading ) const {
        switch( heading ) {
            case HEADING_NORTH: return y > other.y;
            case HEADING_EAST:  return x > other.x;
            case HEADING_SOUTH: return y < other.y;
            case HEADING_WEST:  return x < other.x;
        }

        return false;
    }

public:
    template< typename XType >
    auto X( const Ray2& ray ) const;

    template< typename XType >
    auto X( const Clust2& clust ) const;

public:
    bool operator == ( const Vec2& other ) const {
        return x == other.x && y == other.y;
    }

    Vec2 operator + ( const Vec2& other ) const {
        return { x + other.x, y + other.y };
    }

    Vec2 operator - ( const Vec2& other ) const {
        return { x - other.x, y - other.y };
    }

    Vec2 operator * ( const Vec2& other ) const {
        return { x * other.x, y * other.y };
    }

    Vec2 operator / ( const Vec2& other ) const {
        return { x / other.x, y / other.y };
    }

    Vec2 operator + ( double delta ) const {
        return { x + delta, y + delta };
    }

    Vec2 operator - ( double delta ) const {
        return { x - delta, y - delta };
    }

    Vec2 operator * ( double delta ) const {
        return { x * delta, y * delta };
    }

    Vec2 operator / ( double delta ) const {
        return { x / delta, y / delta };
    }

    Vec2 operator >> ( double delta ) const {
        return { x + delta, y };
    }

    Vec2 operator ^ ( double delta ) const {
        return { x, y + delta };
    }

    Vec2& operator += ( const Vec2& other ) {
        x += other.x;
        y += other.y;

        return *this;
    }

    Vec2& operator -= ( const Vec2& other ) {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    Vec2& operator *= ( const Vec2& other ) {
        x *= other.x;
        y *= other.y;

        return *this;
    }

    Vec2& operator /= ( const Vec2& other ) {
        x /= other.x;
        y /= other.y;

        return *this;
    }

    Vec2& operator *= ( double delta ) {
        x *= delta;
        y *= delta;

        return *this;
    }

    Vec2& operator /= ( double delta ) {
        x /= delta;
        y /= delta;

        return *this;
    }

    Vec2& operator >>= ( double delta ) {
        x += delta;

        return *this;
    }

    Vec2& operator ^= ( double delta ) {
        y += delta;

        return *this;
    }

    Vec2 operator - () const {
        return ( *this ) * -1.0;
    }

public:
    static Vec2 O() {
        return { 0.0, 0.0 };
    }

};



class Ray2 {
public:
    Ray2() = default;

    Ray2( Vec2 org, Vec2 v )
    : origin( org ), vec( v )
    {}

public:
    Vec2   origin   = {};
    Vec2   vec      = {};

public:
    Vec2 drop() const {
        return origin + vec;
    }

public:
    double slope() const {
        return ( this->drop().y - origin.y ) / ( this->drop().x - origin.x );
    }

    std::tuple< double, double, double > coeffs() const {
        return { vec.y, -vec.x, vec.y * origin.x - vec.x * origin.y };
    }

public:
    template< typename XType >
    auto X( const Vec2& vec ) const {
        this->X< XType >( Ray2{ Vec2::O(), vec } );
    }

    template< typename XType >
    auto X( const Ray2& other ) const {
        if constexpr( std::is_same_v< bool, XType > )
            return this->_intersect_vec( other ).has_value();
        else if constexpr( std::is_same_v< Vec2, XType > )
            return this->_intersect_vec( other );
    }

    bool Xprll( const Ray2& other ) const {
        static auto has_any_normal_component = [] ( Vec2 normalized ) -> bool {
            return ( normalized.x >= 0.0 && normalized.x <= 1.0 )
                    ||
                    ( normalized.y >= 0.0 && normalized.y <= 1.0 );
        };


        if( this->slope() != other.slope() ) return false;


        return has_any_normal_component( other.origin( origin ) / vec )
                ||
                has_any_normal_component( other.drop()( origin ) / vec )
                ||
                has_any_normal_component( origin( other.origin ) / other.vec )
                ||
                has_any_normal_component( this->drop()( other.origin ) / other.vec );
    }

    template< typename XType >
    auto X( const Clust2& clust ) const;

protected:
    std::optional< Vec2 > _intersect_vec( const Ray2& other ) const {
        /* Hai noroc nea' Peter +respect. */

        static auto has_any_normal_component = [] ( Vec2 normalized ) -> bool {
            return ( normalized.x >= 0.0 && normalized.x <= 1.0 )
                    ||
                    ( normalized.y >= 0.0 && normalized.y <= 1.0 );
        };

        auto [ alpha, bravo, charlie ] = this->coeffs();
        auto [ delta, echo, foxtrot ] = other.coeffs();

        double golf = alpha * echo - bravo * delta;

        if( golf == 0.0 ) return {};

        Vec2 X_vec = {
            ( charlie * echo - bravo * foxtrot ) / golf,
            ( alpha * foxtrot - delta * charlie ) / golf
        };

        if( !has_any_normal_component( X_vec( origin ) / vec ) ) return {};

        if( !has_any_normal_component( X_vec( other.origin ) / other.vec ) ) return {};

        return X_vec;
    }

};



template< typename OriginType >
concept is_clust2_origin = (
    std::is_same_v< std::decay_t< OriginType >, Vec2 >
    ||
    std::is_same_v< std::decay_t< OriginType >, Clust2* >
);

class Clust2 {
public:
    Clust2() = default;

    template< typename Iterator >
    Clust2( Iterator begin, Iterator end ) {
        _vrtx.reserve( abs( std::distance( begin, end ) ) );

        for( ; begin != end; ++begin )
            _vrtx.emplace_back( *begin, *begin );
    }

    template< typename Container >
    Clust2( const Container& container )
    : Clust2( container.begin(), container.end() )
    {}

    template< typename OriginType, typename Iterator >
    requires ( is_clust2_origin< OriginType > )
    Clust2( OriginType org, Iterator begin, Iterator end, Vec2 offs = {} )
    : Clust2( begin, end )
    {
        if constexpr( std::is_same_v< OriginType, Vec2 > )
            _origin = org;
        else
            _origin = { org, offs };
    }

    template< typename OriginType, typename Container >
    requires ( is_clust2_origin< OriginType > )
    Clust2( OriginType org, const Container& container, Vec2 offs = {} )
    : Clust2( org, container.begin(), container.end(), offs )
    {}

public:
    Clust2( const Clust2& other )
        : _origin( other._origin ),
            _vrtx  ( other._vrtx ),
            _scaleX( other._scaleX ),
            _scaleY( other._scaleY ),
            _angel ( other._angel )
    {}

    Clust2& operator = ( const Clust2& other ) {
        _origin = other._origin;
        _vrtx   = other._vrtx;
        _scaleX = other._scaleX;
        _scaleY = other._scaleY;
        _angel  = other._angel;

        return *this;
    }

    Clust2( Clust2&& other ) noexcept
    : _origin{ std::move( other._origin ) },
        _vrtx  { std::move( other._vrtx ) },
        _scaleX{ other._scaleX },
        _scaleY{ other._scaleY },
        _angel { other._angel }
    {}

    Clust2& operator = ( Clust2&& other ) noexcept {
        _origin = std::move( other._origin );
        _vrtx   = std::move( other._vrtx );
        _scaleX = other._scaleX;
        _scaleY = other._scaleY;
        _angel  = other._angel;

        return *this;
    }

protected:
    typedef   std::variant< Vec2, std::pair< Clust2*, Vec2 > >   Origin;
    typedef   std::pair< Vec2, Vec2 >                            Vrtx;

    enum ORIGIN_VARIANT_ACCESS_INDEX {
        OVAI_VEC  = 0,
        OVAI_HOOK = 1
    };

protected:
    Origin                _origin   = Vec2{ 0.0, 0.0 };
    std::vector< Vrtx >   _vrtx     = {};

    double                _scaleX   = 1.0;
    double                _scaleY   = 1.0;
    double                _angel    = 0.0;

public:
    Vec2 origin() const {
        return this->is_hooked() ?
            const_cast< Clust2* >( this )->hook().origin()
            +
            const_cast< Clust2* >( this )->hook_offs()
            :
            std::get< OVAI_VEC >( _origin );
    }

    Vec2& origin_ref() {
        return this->is_hooked() ?
            this->hook().origin_ref()
            :
            std::get< OVAI_VEC >( _origin );
    }

    operator Vec2 () const {
        return this->origin();
    }

    Vec2 operator () () const {
        return this->operator Vec2();
    }

public:
    bool is_hooked() const {
        return _origin.index() == OVAI_HOOK;
    }

    bool is_hookable_to( Clust2& other ) {
        static bool ( *propagate )( Clust2&, Clust2* ) = [] ( Clust2& clust, Clust2* invoker ) -> bool {
            if( !clust.is_hooked() ) return true;

            if( &clust.hook() == invoker ) return false;

            return propagate( clust.hook(), invoker );
        };

        return propagate( other, this );
    }

    Vec2& hook_offs() {
        return std::get< OVAI_HOOK >( _origin ).second;
    }

    Clust2& hook_offs_to( const Vec2& vec ) {
        this->hook_offs() = vec;

        return *this;
    }

    Clust2& hook() {
        return *std::get< OVAI_HOOK >( _origin ).first;
    }

    Clust2& hook_to( Clust2& other, std::optional< Vec2 > offs = {} ) {
        _origin = std::make_pair(
            &other,
            offs.value_or( this->origin()( other.origin() ) )
        );

        return *this;
    }

    Clust2& dehook() {
        _origin = this->origin();

        return *this;
    }

public:
    Vec2& vrtx_b( size_t idx ) {
        return _vrtx[ idx ].second;
    }

    Vec2& operator [] ( size_t idx ) {
        return _vrtx[ idx ].first;
    }

    Vec2 operator() ( size_t idx ) const {
        return _vrtx[ idx ].first + this->origin();
    }

    size_t vrtx_count() const {
        return _vrtx.size();
    }

public:
    double angel() const {
        return _angel;
    }

    double scaleX() const {
        return _scaleX;
    }

    double scaleY() const {
        return _scaleY;
    }

    double scale() const {
        return this->scaleX();
    }

public:
    Clust2& relocate_at( const Vec2& vec ) {
        this->origin_ref() = vec;

        return *this;
    }

    Clust2& operator = ( const Vec2& vec ) {
        return this->relocate_at( vec );
    }

    Clust2& relocate_by( size_t idx, const Vec2& vec ) {
        this->origin_ref() += vec.respect_to( this->operator()( idx ) );

        return *this;
    }

public:
    Clust2& spin_with( double theta ) {
        _angel += theta;

        this->_refresh();

        return *this;
    }

    Clust2& spin_at( double theta ) {
        _angel = theta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleX_with( double delta ) {
        _scaleX *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleY_with( double delta ) {
        _scaleY *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scale_with( double delta ) {
        _scaleX *= delta;
        _scaleY *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleX_at( double delta ) {
        _scaleX = delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleY_at( double delta ) {
        _scaleY = delta;

        this->_refresh();

        return *this;
    }

    Clust2& scale_at( double delta ) {
        _scaleX = _scaleY = delta;

        this->_refresh();

        return *this;
    }

public:
    static Clust2 triangle( double edge_length ) {
        Vec2 vrtx = { 0.0, edge_length * sqrt( 3.0 ) / 3.0 };

        return std::vector< Vec2 >( {
            vrtx,
            vrtx.spinned( 120.0 ),
            vrtx.spinned( -120.0 )
        } );
    }

    static Clust2 square( double edge_length ) {
        edge_length /= 2.0;

        return std::vector< Vec2 >( {
            { edge_length, edge_length },
            { edge_length, -edge_length },
            { -edge_length, -edge_length },
            { -edge_length, edge_length }
        } );
    }

    static Clust2 circle( double radius, size_t precision ) {
        std::vector< Vec2 > vrtx;
        vrtx.reserve( precision );

        vrtx.emplace_back( 0.0, radius );

        for( size_t n = 1; n < precision; ++n )
            vrtx.push_back( vrtx.front().spinned( 360.0 / precision * n ) );

        return vrtx;
    }

    static Clust2 rect( Vec2 tl, Vec2 br ) {
        return std::vector< Vec2 >( {
            tl, Vec2( br.x, tl.y ), br, Vec2( tl.x, br.y )
        } );
    }

    template< typename T >
    requires std::is_invocable_v< T >
    static Clust2 random(
        double min_dist, double max_dist,
        size_t min_ec, size_t max_ec,
        const T& generator
    ) {
        static auto scalar = [] ( const auto& generator, double min ) -> double {
            return ( static_cast< double >( std::invoke( generator ) % 10001 ) / 10000 )
                    * ( 1.0 - min ) + min;
        };

        size_t edge_count = std::invoke( generator ) % ( max_ec - min_ec + 1 ) + min_ec;

        Vec2 vrtx[ edge_count ];

        vrtx[ 0 ] = { 0.0, max_dist };

        double diff = 360.0 / edge_count;


        for( size_t n = 1; n < edge_count; ++n )
            vrtx[ n ] = vrtx[ 0 ].spinned( diff * n + ( scalar( generator, min_dist / max_dist ) - 0.5 ) * diff )
                        *
                        scalar( generator, min_dist / max_dist );


        return { vrtx, vrtx + edge_count };
    }

public:
    size_t extreme_idx( HEADING heading ) {
        size_t ex_idx = 0;

        for( size_t idx = 0; idx < this->vrtx_count(); ++idx )
            if( _vrtx[ idx ].first.is_further_than( _vrtx[ ex_idx ].first, heading ) )
                ex_idx = idx;

        return ex_idx;
    }

    Vec2& extreme_ref( HEADING heading ) {
        return _vrtx[ this->extreme_idx( heading ) ].first;
    }

    Vec2 extreme( HEADING heading, SYSTEM system = SYSTEM_GLOBAL ) const {
        return const_cast< Clust2* >( this )->extreme_ref( heading )
                +
                ( system == SYSTEM_GLOBAL ? this->origin() : Vec2::O() );
    }

public:
    template< typename XType >
    auto X( const Vec2& vec ) const {
        return this->X< XType >( Ray2{ Vec2::O(), vec } );
    }

    template< typename XType >
    auto X( const Ray2& ray ) const {
        if constexpr( std::is_same_v< bool, XType > )
            return this->_intersect_ray_bool( ray );
        else if constexpr( std::is_same_v< Vec2, XType > )
            return this->_intersect_ray_vec( ray );
    }

    template< typename XType >
    auto X( const Clust2& other ) const {
        if constexpr( std::is_same_v< bool, XType > )
            return this->_intersect_bool( other );
        else if constexpr( std::is_same_v< Vec2, XType > )
            return this->_intersect_vec( other );
    }

protected:
    bool _intersect_ray_bool( const Ray2& ray ) const {
        for( size_t idx = 0; idx < this->vrtx_count(); ++idx )
            if( this->_mkray( idx ).X< bool >( ray ) )
                return true;

        return false;
    }

    std::vector< Vec2 > _intersect_ray_vec( const Ray2& ray ) const {
        std::vector< Vec2 > Xs{};

        for( size_t idx = 0; idx < this->vrtx_count(); ++idx ) {
            auto vec = this->_mkray( idx ).X< Vec2 >( ray );

            if( vec.has_value() )
                Xs.push_back( vec.value() );
        }

        return Xs;
    }

    bool _intersect_bool( const Clust2& other ) const {
        for( size_t idx = 0; idx < other.vrtx_count(); ++idx )
            if( this->X< bool >( other._mkray( idx ) ) )
                return true;

        return false;
    }

    std::vector< Vec2 > _intersect_vec( const Clust2& other ) const {
        std::vector< Vec2 > Xs{};

        for( size_t idx = 0; idx < other.vrtx_count(); ++idx ) {
            auto vecs = this->X< Vec2 >( other._mkray( idx ) );

            Xs.insert( Xs.end(), vecs.begin(), vecs.end() );
        }

        return Xs;
    }

public:
    bool contains( const Vec2& vec ) const {
        Ray2 strike = {
            vec,
            ( vec >> ( vec.dist_sq_to( this->extreme( HEADING_EAST, SYSTEM_GLOBAL ) ) + 10.0 ) )( vec )
        };

        size_t intersections = 0;

        std::pair< bool, bool > skip = { false, false };


        for( size_t idx = 0; idx < this->vrtx_count(); ++idx ) {
            Ray2 edge = this->_mkray( idx, ( idx + 1 ) % this->vrtx_count() );

            if( skip.first ) {
                skip.first = false;

                if( skip.second ? edge.drop().y <= strike.origin.y : edge.drop().y >= strike.origin.y )
                    continue;
            }

            if( strike.X< bool >( edge ) ) {
                ++intersections;

                if( ( skip.first = ( edge.drop().y == strike.origin.y ) ) )
                    skip.second = edge.origin.y > strike.origin.y;
            }
        }

        if( skip.first && !strike.Xprll( this->_mkray( 0, 1 ) ) )
            --intersections;

        return intersections % 2;
    }

protected:
    void _refresh() {
        for( Vrtx& v : _vrtx )
            v.first = v.second.spinned( _angel ) * Vec2{ _scaleX, _scaleY };
    }

    Ray2 _mkray( size_t idx ) const {
        return { ( *this )( idx ), ( *this )( ( idx + 1 ) % this->vrtx_count() )( ( *this )( idx ) ) };
    }

    Ray2 _mkray( size_t idx1, size_t idx2 ) const {
        return { ( *this )( idx1 ), ( *this )( idx2 )( ( *this )( idx1 ) ) };
    }

public:
    static Clust2 from_file( std::string_view path ) {
        std::ifstream file( path.data() );

        if( !file ) return {};

        Vec2 org = {};

        std::vector< Vec2 > vrtx = {};

        {
            bool has_origin = false;

            file >> has_origin;

            if( has_origin )
                file >> org.x >> org.y;
        }

        for( Vec2 vec; file >> vec.x >> vec.y; )
            vrtx.push_back( vec );

        return { org, vrtx };
    }

};



template< typename XType >
auto Vec2::X( const Ray2& ray ) const {
    return ray.X< XType >( *this );
}

template< typename XType >
auto Vec2::X( const Clust2& clust ) const {
    return clust.X< XType >( *this );
}


template< typename XType >
auto Ray2::X( const Clust2& clust ) const {
    return clust.X< XType >( *this );
}



#pragma endregion D2



#pragma endregion Space



#pragma region Graphics



#pragma region Surface



enum KEY_STATE {
    KEY_STATE_UP   = 0,
    KEY_STATE_DOWN = 1
};

class Key {
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
    Key() = default;

    Key( short key )
    : value( key )
    {}

public:
    short   value   = NONE;

public:
    operator short () const {
        return value;
    }

public:
    bool operator == ( const Key& other ) const {
        return value == other.value;
    }

    bool operator == ( VALUE val ) const {
        return value == val;
    }

    std::strong_ordering operator <=> ( const Key& other ) const {
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

    static bool down( Key key );
#endif

};



enum SCROLL_DIRECTION {
    SCROLL_DIRECTION_UP, 
    SCROLL_DIRECTION_DOWN, 
    SCROLL_DIRECTION_LEFT, 
    SCROLL_DIRECTION_RIGHT
};



enum SURFACE_EVENT {
    SURFACE_EVENT_KEY, 
    SURFACE_EVENT_MOUSE, 
    SURFACE_EVENT_SCROLL, 
    SURFACE_EVENT_FILEDROP, 
    SURFACE_EVENT_MOVE, 
    SURFACE_EVENT_RESIZE,

    SURFACE_EVENT__DESTROY = 69100,
    SURFACE_EVENT__CURSOR_HIDE, 
    SURFACE_EVENT__CURSOR_SHOW,
    SURFACE_EVENT__FORCE
};

enum SURFACE_PLUG_SOCKET {
    SURFACE_PLUG_SOCKET_AT_ENTRY = 0,
    SURFACE_PLUG_SOCKET_AT_EXIT  = 1
};

class Surface : public UTH {
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "Surface" );

protected:
    friend class Key;
    friend class Mouse;

public:
    Surface() = default;

    Surface(
        std::string_view title,
        Coord< int >     crd   = { 0, 0 },
        Size< int >      size  = { 512, 512 },
        Echo             echo  = {}
    )
    : _coord( crd ), _size( size )
    {
        #if defined( _ENGINE_UNIQUE_SURFACE )
            _ptr = this;
        #endif

        _wnd_class.cbSize        = sizeof( WNDCLASSEX );
        _wnd_class.hInstance     = GetModuleHandle( NULL );
        _wnd_class.lpfnWndProc   = event_proc_router_1;
        _wnd_class.lpszClassName = title.data();
        _wnd_class.hbrBackground = HBRUSH( COLOR_INACTIVECAPTIONTEXT );
        _wnd_class.hCursor       = LoadCursor( NULL, IDC_ARROW );


        std::binary_semaphore sync{ 0 };

        _thread = std::thread( _main, this, &sync, echo );

        if( _thread.joinable() ) {
            echo( this, ECHO_LOG_PENDING, "Awaiting window creation..." );

            sync.acquire();
        } else
            echo( this, ECHO_LOG_FAULT, "Window thread launch failed." );
    }


    Surface( const Surface& ) = delete;

    Surface( Surface&& ) = delete;


    ~Surface() {
        SendMessage( _hwnd, SURFACE_EVENT__DESTROY, WPARAM{}, LPARAM{} );

        if( _thread.joinable() )
            _thread.join();

        #if defined( _ENGINE_UNIQUE_SURFACE )
            _ptr = nullptr;
        #endif
    }

public:
    struct Trace {
        Trace() = default;

        struct Result {
            GUID           guid     = {};
            std::bitset< 16 >   result   = 0;
        };

        std::bitset< 64 >       master   = 0;
        std::vector< Result >   plugs    = {};

        void clear() {
            master.reset();
            plugs.clear();
        }

        std::bitset< 64 >::reference operator [] ( size_t idx ) {
            return master[ idx ];
        }
    };

public:
    typedef   std::function< void( Vec2, Vec2, Trace& ) >                   OnMouse;
    typedef   std::function< void( Key, KEY_STATE, Trace& ) >               OnKey;
    typedef   std::function< void( SCROLL_DIRECTION, Trace& ) >             OnScroll;
    typedef   std::function< void( std::vector< std::string >, Trace& ) >   OnFiledrop;
    typedef   std::function< void( Coord< int >, Coord< int >, Trace& ) >   OnMove;
    typedef   std::function< void( Size< int >, Size< int >, Trace& ) >     OnResize;

    typedef   std::array< KEY_STATE, Key::COUNT >                           Keys;

protected:
    static constexpr auto   LIQUID_STYLE   = WS_OVERLAPPED | WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
    static constexpr auto   SOLID_STYLE    = WS_POPUP | WS_VISIBLE;

protected:
#if defined( _ENGINE_UNIQUE_SURFACE )
    inline static Surface*        _ptr                  = nullptr;
#endif
    HWND                          _hwnd                 = NULL;
    WNDCLASSEX                    _wnd_class            = {};
    std::thread                   _thread               = {};
    Coord< int >                  _coord                = {};
    Size< int >                   _size                 = {};

    Trace                         _trace                = {};

    OnMouse                       _on_mouse             = {};
    OnKey                         _on_key               = {};
    OnScroll                      _on_scroll            = {};
    OnFiledrop                    _on_filedrop          = {};
    OnMove                        _on_move              = {};
    OnResize                      _on_resize            = {};

    std::map< GUID, OnMouse >     _plug_mouse[ 2 ]      = {};
    std::map< GUID, OnKey >       _plug_key[ 2 ]        = {};
    std::map< GUID, OnScroll >    _plug_scroll[ 2 ]     = {};
    std::map< GUID, OnFiledrop >  _plug_filedrop[ 2 ]   = {};
    std::map< GUID, OnMove >      _plug_move[ 2 ]       = {};
    std::map< GUID, OnResize >    _plug_resize[ 2 ]     = {};


    Vec2                          _mouse                = {};
    Vec2                          _mouse_l              = {};
    Keys                          _keys                 = {};

protected:
    void _main( std::binary_semaphore* sync, Echo echo = {} ) {
        if( !RegisterClassEx( &_wnd_class ) ) {
            echo( this, ECHO_LOG_FAULT, "Window class registration failed." );

            sync->release(); return;
        }


        _hwnd = CreateWindowEx(
            WS_EX_ACCEPTFILES,

            _wnd_class.lpszClassName, _wnd_class.lpszClassName,

            SOLID_STYLE,

            _coord.x, _coord.y, _size.width, _size.height,

            NULL, NULL,

            GetModuleHandle( NULL ),

            this
        );

        if( !_hwnd ) {
            echo( this, ECHO_LOG_FAULT, "Window creation failed." );

            sync->release(); return;
        }

        SetWindowText( _hwnd, _wnd_class.lpszClassName );


        echo( this, ECHO_LOG_OK, "Created." );

        sync->release();


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
        static auto key_proc = [ this ] ( KEY_STATE state, WPARAM w_param ) -> void {
            Key key = static_cast< Key >( w_param );

            _keys[ key ] = state;

            this->invoke_ons< OnKey >( key, state );
        };

        switch( event ) {
            case WM_CREATE: {

            break; }


            case SURFACE_EVENT__DESTROY: {
                DestroyWindow( hwnd );
                PostQuitMessage( 0 );
            break; }

            case SURFACE_EVENT__CURSOR_HIDE: {
                while( ShowCursor( false ) >= 0 );
            break; }

            case SURFACE_EVENT__CURSOR_SHOW: {
                while( ShowCursor( true ) >= 0 );
            break; }

            case SURFACE_EVENT__FORCE :{
                this->invoke_ons< OnMouse >( _mouse, _mouse );
            break; }


            case WM_MOUSEMOVE: {
                Vec2 new_mouse = this->pull_vec( { LOWORD( l_param ), HIWORD( l_param ) } );

                this->invoke_ons< OnMouse >( new_mouse, _mouse_l = std::exchange( _mouse, new_mouse ) );

            break; }

            case WM_MOUSEWHEEL: {
                this->invoke_ons< OnScroll >(
                    GET_WHEEL_DELTA_WPARAM( w_param ) < 0
                    ?
                    SCROLL_DIRECTION_DOWN : SCROLL_DIRECTION_UP
                );

                break;
            }


            case WM_LBUTTONDOWN: {
                key_proc( KEY_STATE_DOWN, Key::LMB );

                break;
            }

            case WM_LBUTTONUP: {
                key_proc( KEY_STATE_UP, Key::LMB );

                break;
            }

            case WM_RBUTTONDOWN: {
                key_proc( KEY_STATE_DOWN, Key::RMB );

                break;
            }

            case WM_RBUTTONUP: {
                key_proc( KEY_STATE_UP, Key::RMB );

                break;
            }

            case WM_MBUTTONDOWN: {
                key_proc( KEY_STATE_DOWN, Key::MMB );

                break;
            }

            case WM_MBUTTONUP: {
                key_proc( KEY_STATE_UP, Key::MMB );

                break;
            }

            case WM_KEYDOWN: {
                if( l_param & ( 1 << 30 ) ) break;

                key_proc( KEY_STATE_DOWN, w_param );

                break;
            }

            case WM_KEYUP: {
                key_proc( KEY_STATE_UP, w_param );

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

                this->invoke_ons< OnFiledrop >( std::move( files ) );

            break; }


            case WM_MOVE: {
                Coord< int > new_coord = { LOWORD( l_param ), HIWORD( l_param ) };

                this->invoke_ons< OnMove >( new_coord, std::exchange( _coord, new_coord ) );

            break; }

            case WM_SIZE: {
                Size< int > new_size = { LOWORD( l_param ), HIWORD( l_param ) };

                this->invoke_ons< OnResize >( new_size, std::exchange( _size, new_size ) );

            break; }

        }

        return DefWindowProc( hwnd, event, w_param, l_param );
    }

    template< typename On, typename ...Args >
    void invoke_ons( Args&&... args ) {
        if constexpr( std::is_same_v< On, OnMouse > )
            this->invoke_ons( _on_mouse, _plug_mouse, std::forward< Args >( args )... );

        else if constexpr( std::is_same_v< On, OnKey > )
            this->invoke_ons( _on_key, _plug_key, std::forward< Args >( args )... );

        else if constexpr( std::is_same_v< On, OnScroll > )
            this->invoke_ons( _on_scroll, _plug_scroll, std::forward< Args >( args )... );

        else if constexpr( std::is_same_v< On, OnFiledrop > )
            this->invoke_ons( _on_filedrop, _plug_filedrop, std::forward< Args >( args )... );

        else if constexpr( std::is_same_v< On, OnMove > )
            this->invoke_ons( _on_move, _plug_move, std::forward< Args >( args )... );

        else if constexpr( std::is_same_v< On, OnResize > )
            this->invoke_ons( _on_resize, _plug_resize, std::forward< Args >( args )... );

    }

    template< typename M, typename P, typename ...Args >
    void invoke_ons( M& master, P& plugs, Args&&... args ) {
        _trace.clear();

        for( auto& pair : plugs[ 0 ] )
            std::invoke( pair.second, std::forward< Args >( args )..., _trace );

        if( master )
            std::invoke( master, std::forward< Args >( args )..., _trace );

        for( auto& pair : plugs[ 1 ] )
            std::invoke( pair.second, std::forward< Args >( args )..., _trace );
    }

public:
    Vec2 pull_vec( const Coord<>& crd ) const {
        return { crd.x - _size.width / 2.0, _size.height / 2.0 - crd.y };
    }

    Coord<> pull_crd( const Vec2& vec ) const {
        return { 
            static_cast< float >( vec.x ) + _size.width / 2.0f, 
            _size.height / 2.0f - static_cast< float >( vec.y ) 
        };
    }

    void push_vec( Coord<>& crd ) const {
        crd.x -= _size.width / 2.0;
        crd.y = _size.height / 2.0 - crd.y;
    }

    void push_crd( Vec2& vec ) const {
        vec.x += _size.width / 2.0;
        vec.y = _size.height / 2.0 - vec.y;
    }

public:
    Coord< int > coord() const {
        return _coord;
    }

    Coord< int > os_crd() const {
        RECT rect = {};

        GetWindowRect( _hwnd, &rect );

        return { rect.left, rect.top };
    }

    int x() const {
        return coord().x;
    }

    int y() const {
        return coord().y;
    }

    Size< int > size() const {
        return _size;
    }

    Size< int > os_size() const {
        RECT rect = {};

        GetWindowRect( _hwnd, &rect );

        return { rect.right - rect.left, rect.bottom - rect.top };
    }

    int width() const {
        return size().width;
    }

    int height() const {
        return size().height;
    }

public:
    Surface& solidify() {
        SetWindowLongPtr( _hwnd, GWL_STYLE, SOLID_STYLE );

        return *this;
    }

    Surface& liquify() {
        SetWindowLongPtr( _hwnd, GWL_STYLE, LIQUID_STYLE );

        return *this;
    }

    Surface& move_to( Coord< int > crd ) {
        SetWindowPos(
            _hwnd,
            0,
            _coord.x = crd.x, _coord.y = crd.y,
            0, 0,
            SWP_NOSIZE
        );

        return *this;
    }

    Surface& size_to( Size< int > size ) {
        SetWindowPos(
            _hwnd,
            0,
            0, 0,
            _size.width = size.width, _size.height = size.height,
            SWP_NOMOVE
        );

        return *this;
    }

public:
    Surface& hide_cursor() {
        SendMessage( _hwnd, SURFACE_EVENT__CURSOR_HIDE, WPARAM{}, LPARAM{} );

        return *this;
    }

    Surface& show_cursor() {
        SendMessage( _hwnd, SURFACE_EVENT__CURSOR_SHOW, WPARAM{}, LPARAM{} );

        return *this;
    }

public:
    Vec2 vec() const {
        return _mouse;
    }

    Vec2 l_vec() const {
        return _mouse_l;
    }

    Coord< int > crd() const {
        return this->pull_crd( vec() );
    }

    Coord< int > l_crd() const {
        return this->pull_crd( l_vec() );
    }

    template< typename ...Keys >
    size_t any_down( Keys... keys ) const {
        size_t count = 0;

        ( ( count += ( _keys[ keys ] == KEY_STATE_DOWN ) ), ... );

        return count;
    }

    template< typename ...Keys >
    size_t tgl_down( Keys... keys ) const {
        size_t sum = 0;
        size_t at = 1;

        ( ( sum +=
            std::exchange( at, at * 2 )
            *
            ( _keys[ keys ] == KEY_STATE_DOWN )
        ), ... );

        return sum;
    }

    template< typename ...Keys >
    bool all_down( Keys... keys ) const {
        return any_down( keys... ) == sizeof...( Keys );
    }

    bool down( Key key ) const {
        return _keys[ key ] == KEY_STATE_DOWN;
    }

public:
    HWND hwnd() {
        return _hwnd;
    }

public:
    Surface& force_plug() {
        SendMessage( _hwnd, SURFACE_EVENT__FORCE, WPARAM{}, LPARAM{} );

        return *this;
    }

public:
    template< SURFACE_EVENT event, typename Function >
    Surface& on( Function function ) {
        if      constexpr( event == SURFACE_EVENT_MOUSE )    _on_mouse    = function;
        else if constexpr( event == SURFACE_EVENT_KEY )      _on_key      = function;
        else if constexpr( event == SURFACE_EVENT_SCROLL )   _on_scroll   = function;
        else if constexpr( event == SURFACE_EVENT_FILEDROP ) _on_filedrop = function;
        else if constexpr( event == SURFACE_EVENT_MOVE )     _on_move     = function;
        else if constexpr( event == SURFACE_EVENT_RESIZE )   _on_resize   = function;

        return *this;
    }

    template< SURFACE_EVENT event, typename Function >
    Surface& plug( const GUID& guid, SURFACE_PLUG_SOCKET socket, Function function ) {
        if      constexpr( event == SURFACE_EVENT_MOUSE )    _plug_mouse   [ socket ].insert( std::make_pair( guid, function )  );
        else if constexpr( event == SURFACE_EVENT_KEY )      _plug_key     [ socket ].insert( std::make_pair( guid, function )  );
        else if constexpr( event == SURFACE_EVENT_SCROLL )   _plug_scroll  [ socket ].insert( std::make_pair( guid, function )  );
        else if constexpr( event == SURFACE_EVENT_FILEDROP ) _plug_filedrop[ socket ].insert( std::make_pair( guid, function )  );
        else if constexpr( event == SURFACE_EVENT_MOVE )     _plug_move    [ socket ].insert( std::make_pair( guid, function )  );
        else if constexpr( event == SURFACE_EVENT_RESIZE )   _plug_resize  [ socket ].insert( std::make_pair( guid, function )  );

        return *this;
    }

    Surface& unplug( const GUID& guid, std::optional< SURFACE_PLUG_SOCKET > socket = {} ) {
        _unplug( guid, socket, _plug_mouse );
        _unplug( guid, socket, _plug_key );
        _unplug( guid, socket, _plug_scroll );
        _unplug( guid, socket, _plug_filedrop );
        _unplug( guid, socket, _plug_move );
        _unplug( guid, socket, _plug_resize );

        return *this;
    }

    template< SURFACE_EVENT event >
    Surface& unplug( const GUID& guid, std::optional< SURFACE_PLUG_SOCKET > socket = {} ) {
        if      constexpr( event == SURFACE_EVENT_MOUSE )    _unplug( guid, socket, _plug_mouse );
        else if constexpr( event == SURFACE_EVENT_KEY )      _unplug( guid, socket, _plug_key );
        else if constexpr( event == SURFACE_EVENT_SCROLL )   _unplug( guid, socket, _plug_scroll );
        else if constexpr( event == SURFACE_EVENT_FILEDROP ) _unplug( guid, socket, _plug_filedrop );
        else if constexpr( event == SURFACE_EVENT_MOVE )     _unplug( guid, socket, _plug_move );
        else if constexpr( event == SURFACE_EVENT_RESIZE )   _unplug( guid, socket, _plug_resize );

        return *this;
    }

protected:
    template< typename Plug >
    void _unplug( const GUID& guid, std::optional< SURFACE_PLUG_SOCKET > socket, Plug& plug ) {
        if( socket.has_value() )
            plug[ socket.value() ].erase( guid );
        else {
            plug[ SURFACE_PLUG_SOCKET_AT_ENTRY ].erase( guid );
            plug[ SURFACE_PLUG_SOCKET_AT_EXIT ] .erase( guid );
        }
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
    static Vec2 vec() {
        return Surface::get()->_mouse;
    }

    static Coord< int > crd() {
        return Surface::get()->pull_crd( vec() );
    }
#endif

public:
    static Vec2 g_vec() {
        auto [ x, y ] = g_crd();

        return { 
            x - static_cast< double >( Env::width_2() ), 
            static_cast< double >( Env::height_2() ) - y 
        };
    }

    static Coord< int > g_crd() {
        POINT p;
        GetCursorPos( &p );

        return { p.x, p.y };
    }

};



#if defined( _ENGINE_UNIQUE_SURFACE )
    template< typename ...Keys >
    size_t Key::any_down( Keys... keys ) {
        return Surface::get()->any_down( keys... );
    }

    template< typename ...Keys >
    size_t Key::tgl_down( Keys... keys ) {
        return Surface::get()->tgl_down( keys... );
    }

    template< typename ...Keys >
    bool Key::all_down( Keys... keys ) {
        return Surface::get()->all_down( keys... );
    }

    bool Key::down( Key key ) {
        return Surface::get()->down( key );
    }
#endif



#pragma endregion Surface



#pragma region Renderer



class Renderer2 : public UTH {
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "Renderer2" );

public:
    Renderer2() = default;

    Renderer2( Surface& surface, Echo echo = {} )
    : _surface{ &surface }
    {
        ECHO_ASSERT_AND_THROW( CoInitialize( nullptr ) == S_OK, "<constructor>: CoInitialize." );


        ECHO_ASSERT_AND_THROW(
            CoCreateInstance(
                CLSID_WICImagingFactory,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IWICImagingFactory,
                ( LPVOID* ) &_wic_factory
            ) == S_OK,

            "<constructor>: CoCreateInstance."
        );

        
        ECHO_ASSERT_AND_THROW(
            D2D1CreateFactory(
                D2D1_FACTORY_TYPE_MULTI_THREADED,
                &_factory
            ) == S_OK,

            "<constructor>: D2D1CreateFactory."
        );

        ECHO_ASSERT_AND_THROW( 
            _factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    _surface->hwnd(),
                    D2D1::SizeU( _surface->width(), _surface->height() ),
                    D2D1_PRESENT_OPTIONS_IMMEDIATELY
                ),
                &_target
            ) == S_OK,

            "<constructor>: _factory->CreateHwndRenderTarget"
        );


        echo( this, ECHO_LOG_OK, "Created." );
    }


    Renderer2( const Renderer2& ) = delete;

    Renderer2( Renderer2&& ) = delete;

public:
    ~Renderer2() {
        if( _factory ) _factory->Release();

        if( _wic_factory ) _wic_factory->Release();

        if( _target ) _target->Release();
    }

protected:
    Surface*                    _surface       = nullptr;

    ID2D1Factory*               _factory       = nullptr;
    IWICImagingFactory*         _wic_factory   = nullptr;

    ID2D1HwndRenderTarget*      _target        = nullptr;

    D2D1::Matrix3x2F            _transform     = D2D1::Matrix3x2F::Identity();

public:
    auto factory() { return _factory; }
    auto target() { return _target; }
    auto wic_factory() { return _wic_factory; }

public:
    auto coord() const {
        return _surface->coord();
    }

    auto size() const {
        return _surface->size();
    }

public:
    Vec2 pull_vec( const Coord<>& crd ) const {
        return _surface->pull_vec( crd );
    }

    Coord<> pull_crd( const Vec2& vec ) const {
        return _surface->pull_crd( vec );
    }

    void push_vec( Coord<>& crd ) const {
        _surface->push_vec( crd );
    }

    void push_crd( Vec2& vec ) const {
        _surface->push_crd( vec );
    }

public:
    Renderer2& open() {
        _target->BeginDraw();

        return *this;
    }

    Renderer2& execute() {
        _target->EndDraw();

        return *this;
    }

public:
    Renderer2& push( double angel, double scaleX, double scaleY, Coord<> crd ) {
        _target->SetTransform( 
            D2D1::Matrix3x2F::Rotation( angel, crd )
            *
            D2D1::Matrix3x2F::Scale( scaleX, scaleY, crd )
            *
            _transform
        );

        return *this;
    }

    Renderer2& pop() {
        _target->SetTransform( _transform );

        return *this;
    }

    Renderer2& push_beneath( double angel, double scaleX, double scaleY, Coord<> crd ) {
        _transform = D2D1::Matrix3x2F::Rotation( angel, crd )
                     *
                     D2D1::Matrix3x2F::Scale( scaleX, scaleY, crd );

        return *this;
    }

    Renderer2& pop_beneath() {
        _transform = D2D1::Matrix3x2F::Identity();

        return *this;
    }

public:
    Renderer2& fill( const Chroma& chroma );

public:
    Renderer2& system( 
        const Brush2& brush, 
        float         div, 
        float         div_half_width
    );

public:
    Renderer2& line(
        Coord<> c1, Coord<> c2,
        const Brush2& brush
    );

    Renderer2& line(
        Vec2 v1, Vec2 v2,
        const Brush2& brush
    );

public:
    template< typename Type, typename ...Args >
    Renderer2& operator () ( const Type& thing, Args&&... args ) {
        thing.render( *this, std::forward< Args >( args )... );

        return *this;
    }

};



class Viewport2 : public UTH {
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "Veiwport2" );

public:
    Viewport2() = default;

    Viewport2(
        Renderer2&   renderer,
        Vec2         org,
        Size<>       sz,
        Echo         echo       = {}
    )
    : _renderer{ &renderer },
      _origin{ org }, _size{ sz }, _size2{ sz.width / 2.0f, sz.height / 2.0f }
    {
        echo( this, ECHO_LOG_OK, "Created." );
    }

    Viewport2(
        Renderer2&   renderer,
        Coord<>      org,
        Size<>       sz,
        Echo         echo       = {}
    )
    : Viewport2{ renderer, renderer.pull_vec( org ) + Vec2{ sz.width / 2.0, -sz.height / 2.0}, sz, echo }
    {}


    Viewport2( Viewport2&& ) = delete;

protected:
    Renderer2*   _renderer   = nullptr;
    Vec2         _origin     = {};
    Size<>       _size       = {};
    Size<>       _size2      = {};

public:
    Renderer2& renderer() {
        return *_renderer;
    }

public:
    Vec2 origin() const {
        return _origin;
    }

    Size<> size() const {
        return _size;
    }

    Size<> size2() const {
        return _size2;
    }

public:
    Vec2 top_left() const {
        return _origin + Vec2{ -_size2.width, _size2.height };
    }

    Vec2 bot_right() const {
        return _origin + Vec2{ _size2.width, -_size2.height };
    }

    double east() const {
        return _origin.x + _size2.width;
    }

    double west() const {
        return _origin.x - _size2.width;
    }

    double north() const {
        return _origin.y + _size2.height;
    }

    double south() const {
        return _origin.y - _size2.height;
    }

    double east_reach() const {
        return _size2.width;
    }

    double west_reach() const {
        return -_size2.width;
    }

    double north_reach() const {
        return _size2.height;
    }

    double south_reach() const {
        return -_size2.height;
    }

};



#pragma endregion Renderer



#pragma region Chroma



class Chroma {
public:
    Chroma() = default;

    Chroma( float r, float g, float b, float a = 1.0 )
    : r( r ), g( g ), b( b ), a( a )
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



#pragma endregion Chroma



#pragma region Brushes



class Brush2 : public UTH {
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "Brush2" );

public:
    Brush2() = default;

    Brush2( float w )
    : _width( w )
    {}

protected:
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

};



class SolidBrush2 : public Brush2 {
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "SolidBrush2" );

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
        ECHO_ASSERT_AND_THROW(
            renderer.target()->CreateSolidColorBrush(
                chroma,
                &_brush
            ) == S_OK,

            "<constructor>: renderer.target()->CreateSolidColorBrush"
        );

        echo( this, ECHO_LOG_OK, "Created." );
    }

public:
    ~SolidBrush2() {
        if( _brush ) _brush->Release();
    }

protected:
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



#pragma endregion Brushes



#pragma endregion Graphics



#pragma region Audio



class Wave {
public:
    friend class Audio;

public:
    typedef   std::function< double( double, size_t ) >   Filter;

public:
    Wave() = default;

    Wave( const Wave& ) = default;

    Wave( Wave&& ) = delete;

protected:
    Audio*   _audio   = nullptr;

public:
    bool is_playing() const;

    void play();

    virtual void prepare_play() = 0;

    virtual void stop() = 0;

    virtual bool done() const = 0;

public:
    void lock_on( Audio& audio ) {
        _audio = &audio;
    }

    void release_lock() {
        _audio = nullptr;
    }

public:
    Audio& lock() const {
        return *_audio;
    }


protected:
    virtual double _sample( size_t channel, bool advance ) = 0;

};

enum WAVE_BEHAVIOUR_DESC {
    WAVE_BEHAVIOUR_DESC_HAS_VOLUME   = 1,
    WAVE_BEHAVIOUR_DESC_PAUSABLE     = 2,
    WAVE_BEHAVIOUR_DESC_MUTABLE      = 4,
    WAVE_BEHAVIOUR_DESC_LOOPABLE     = 8,
    WAVE_BEHAVIOUR_DESC_HAS_VELOCITY = 16,
    WAVE_BEHAVIOUR_DESC_HAS_FILTER   = 32
};

template< typename DerivedWave, int desc >
class WaveBehaviourDescriptor {
protected:
    std::conditional_t< desc & WAVE_BEHAVIOUR_DESC_HAS_VOLUME,   double,       VoidStruct >   _volume     = 1.0;
    std::conditional_t< desc & WAVE_BEHAVIOUR_DESC_PAUSABLE,     bool,         VoidStruct >   _paused     = false;
    std::conditional_t< desc & WAVE_BEHAVIOUR_DESC_MUTABLE,      bool,         VoidStruct >   _muted      = false;
    std::conditional_t< desc & WAVE_BEHAVIOUR_DESC_LOOPABLE,     bool,         VoidStruct >   _looping    = false;
    std::conditional_t< desc & WAVE_BEHAVIOUR_DESC_HAS_VELOCITY, double,       VoidStruct >   _velocity   = 1.0;
    std::conditional_t< desc & WAVE_BEHAVIOUR_DESC_HAS_FILTER,   Wave::Filter, VoidStruct >   _filter     = {};

public:
    bool volume() const {
        return _volume;
    }

public:
    DerivedWave& volume_to( double vlm ) {
        _volume = std::clamp( vlm, -1.0, 1.0 );

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& tweak_volume( double twk ) {
        _volume = std::clamp( _volume + twk, -1.0, 1.0 );

        return static_cast< DerivedWave& >( *this );
    }

public:
    bool is_paused() const {
        return _paused;
    }

public:
    DerivedWave& pause() {
        _paused = true;

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& resume() {
        _paused = false;

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& tweak_pause() {
        _paused ^= true;

        return static_cast< DerivedWave& >( *this );
    }

public:
    bool is_muted() const {
        return _muted;
    }

public:
    DerivedWave& mute() {
        _muted = true;

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& unmute() {
        _muted = false;

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& tweak_mute() {
        _muted ^= true;

        return static_cast< DerivedWave& >( *this );
    }

public:
    bool is_looping() const {
        return _looping;
    }

public:
    DerivedWave& loop() {
        _looping = true;

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& unloop() {
        _looping = false;

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& tweak_loop() {
        _looping ^= true;

        return static_cast< DerivedWave& >( *this );
    }

public:
    double velocity() const {
        return _velocity;
    }

public:
    DerivedWave& velocity_to( double vlc ) {
        _velocity = vlc;

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& tweak_velocity( double twk ) {
        _velocity += twk;

        return static_cast< DerivedWave& >( *this );
    }

public:
    Wave::Filter filter() const {
        return _filter;
    }

public:
    DerivedWave& filter_to( const Wave::Filter& flt ) {
        _filter = flt;

        return static_cast< DerivedWave& >( *this );
    }

    DerivedWave& remove_filter() {
        _filter = nullptr;

        return static_cast< DerivedWave& >( *this );
    }

};



class Audio : public UTH,
              public WaveBehaviourDescriptor< Audio,
                  WAVE_BEHAVIOUR_DESC_HAS_VOLUME   |
                  WAVE_BEHAVIOUR_DESC_PAUSABLE     |
                  WAVE_BEHAVIOUR_DESC_MUTABLE      |
                  WAVE_BEHAVIOUR_DESC_HAS_VELOCITY |
                  WAVE_BEHAVIOUR_DESC_HAS_FILTER
              >
{
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "Audio" );

protected:
    friend class Wave;

public:
    Audio() = default;

    Audio(
        std::string_view   device,
        size_t             sample_rate          = 48000,
        size_t             channel_count        = 1,
        size_t             block_count          = 16,
        size_t             block_sample_count   = 256,
        Echo               echo                 = {}
    )
    : _sample_rate       { sample_rate },
      _time_step         { 1.0 / _sample_rate },
      _channel_count     { channel_count },
      _block_count       { block_count },
      _block_sample_count{ block_sample_count },
      _block_current     { 0 },
      _block_memory      { nullptr },
      _wave_headers      { nullptr },
      _device            { device.data() },
      _free_block_count  { block_count }
    {
        uint32_t dev_idx = 0;

        auto devs = devices();

        for( auto& dev : devs ) {
            if( dev == _device ) break;

            ++dev_idx;
        }

        if( dev_idx == devs.size() ) {
            echo( this, ECHO_LOG_FAULT, "Device does not exist." ); return;
        }


        WAVEFORMATEX wave_format;

        wave_format.wFormatTag      = WAVE_FORMAT_PCM;
        wave_format.nSamplesPerSec  = _sample_rate;
        wave_format.wBitsPerSample  = sizeof( int ) * 8;
        wave_format.nChannels       = _channel_count;
        wave_format.nBlockAlign     = ( wave_format.wBitsPerSample / 8 ) * wave_format.nChannels;
        wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
        wave_format.cbSize          = 0;


        auto result = waveOutOpen(
            &_wave_out, dev_idx, &wave_format,
            ( DWORD_PTR ) event_proc_router,
            ( DWORD_PTR ) this,
            CALLBACK_FUNCTION
        );

        if( result != S_OK ) {
            echo( this, ECHO_LOG_FAULT, "Wave open link failed." ); return;
        }


        _block_memory = new int[ _block_count * _block_sample_count ];

        if( !_block_memory ) {
            echo( this, ECHO_LOG_FAULT, "Block alloc failed." ); return;
        }

        std::fill_n( _block_memory.get(), _block_count * _block_sample_count, 0 );


        _wave_headers = new WAVEHDR[ _block_count ];

        if( !_wave_headers ) {
            echo( this, ECHO_LOG_FAULT, "Wave headers alloc failed." ); return;
        }

        std::fill_n( ( char* ) _wave_headers.get(), sizeof( WAVEHDR ) * _block_count, 0 );


        for( size_t n = 0; n < _block_count; ++n ) {
            _wave_headers[ n ].dwBufferLength = sizeof( int ) * _block_sample_count;
            _wave_headers[ n ].lpData = ( char* ) ( _block_memory + ( n * _block_sample_count ) );
        }


        _powered = true;

        _thread = std::thread( _main, this );

        if( !_thread.joinable() ) {
            echo( this, ECHO_LOG_FAULT, "Thread launch failed." ); return;
        }

        std::unique_lock< std::mutex > lock{ _mtx };
        _cnd_var.notify_one();

        echo( this, ECHO_LOG_OK, "Created." );
    }


    Audio( const Audio& ) = delete;

    Audio( Audio&& ) = delete;


    ~Audio() {
        _powered = false;

        _cnd_var.notify_one();

        if( _thread.joinable() )
            _thread.join();

        waveOutReset( _wave_out );
        waveOutClose( _wave_out );
    }

protected:
    volatile bool             _powered              = false;

    size_t                    _sample_rate          = 0;
    double                    _time_step            = 0.0;
    size_t                    _channel_count        = 0;
    size_t                    _block_count          = 0;
    size_t                    _block_sample_count   = 0;
    size_t                    _block_current        = 0;
    Unique< int[] >           _block_memory         = nullptr;

    Unique< WAVEHDR[] >       _wave_headers         = nullptr;
    HWAVEOUT                  _wave_out             = nullptr;
    std::string               _device               = {};

    std::thread               _thread               = {};

    std::atomic< size_t >     _free_block_count     = 0;
    std::condition_variable   _cnd_var              = {};
    std::mutex                _mtx                  = {};

    std::list< Wave* >        _waves                = {};

protected:


    void _main() {
        constexpr double max_sample = static_cast< double >(
            std::numeric_limits< int >::max()
        );

        
        auto sample = [ this ] ( size_t channel ) -> double {
            double amp = 0.0;

            if( _paused ) return amp;
           
            bool advance = ( channel == _channel_count - 1 );

            for( Wave* wave : _waves )
                amp += wave->_sample( channel, advance );

            return _filter ? _filter( amp, channel ) : amp
                   * _volume * !_muted;
        };

        
        while( _powered ) {
            if( _free_block_count.fetch_sub( 1, std::memory_order_relaxed ) == 0 ) {
                std::unique_lock< std::mutex > lock{ _mtx };
                _cnd_var.wait( lock );
            }

           
            if( _wave_headers[ _block_current ].dwFlags & WHDR_PREPARED )
                waveOutUnprepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );


            _waves.remove_if( [] ( Wave* wave ) {
                return wave->done();
            } );
            
            
            size_t current_block = _block_current * _block_sample_count;

            for( size_t n = 0; n < _block_sample_count; n += _channel_count )
                for( size_t ch = 0; ch < _channel_count; ++ch )
                    _block_memory[ current_block + n + ch ] = static_cast< int >( 
                        std::clamp( sample( ch ), -1.0, 1.0 ) * max_sample 
                    );
            
            
            waveOutPrepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );
            waveOutWrite( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );


            ++_block_current;
            _block_current %= _block_count;
        }

    }

protected:
    static void event_proc_router( HWAVEOUT hwo, UINT event, DWORD_PTR instance, DWORD w_param, DWORD l_param ) {
        reinterpret_cast< Audio* >( instance )->event_proc( hwo, event, w_param, l_param);
    }

    void event_proc( HWAVEOUT hwo, UINT event, DWORD w_param, DWORD l_param ) {
        switch( event ) {
            case WOM_OPEN: {

            break; }

            case WOM_DONE: {
                _free_block_count.fetch_add( 1, std::memory_order_relaxed );

                std::unique_lock< std::mutex > lock{ _mtx };
                _cnd_var.notify_one();
            break; }

            case WOM_CLOSE: {
                /* Here were the uniques delete[]'d */
            break; }
        }
    }

public:
    static std::vector< std::string > devices() {
        WAVEOUTCAPS woc;

        std::vector< std::string > devs;

        for( decltype( waveOutGetNumDevs() ) n = 0; n < waveOutGetNumDevs(); ++n ) {
            if( waveOutGetDevCaps( n, &woc, sizeof( WAVEOUTCAPS ) ) != S_OK ) continue;

            devs.emplace_back( woc.szPname );
        }

        return devs;
    }

    std::string_view device() const {
        return _device;
    }

    void device_to( std::string_view dev ) {
        
    }

public:
    size_t sample_rate() const {
        return _sample_rate;
    }

    double time_step() const {
        return _time_step;
    }

    size_t channel_count() const {
        return _channel_count;
    }

};



class Sound : public UTH,
              public Wave, 
              public WaveBehaviourDescriptor< Sound,
                  WAVE_BEHAVIOUR_DESC_HAS_VOLUME   |
                  WAVE_BEHAVIOUR_DESC_PAUSABLE     |
                  WAVE_BEHAVIOUR_DESC_MUTABLE      |
                  WAVE_BEHAVIOUR_DESC_LOOPABLE     |
                  WAVE_BEHAVIOUR_DESC_HAS_VELOCITY |
                  WAVE_BEHAVIOUR_DESC_HAS_FILTER
              >
{
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "Sound" );

public:
    Sound() = default;

    Sound( 
        Audio&             audio, 
        std::string_view   path, 
        Echo               echo   = {} 
    )
    : Sound{ path, echo }
    {
        _audio = &audio;

        if( _sample_rate != _audio->sample_rate() )
            echo( this, ECHO_LOG_WARNING, "Sample rate does not match with locked on audio's." );


        if( _channel_count != _audio->channel_count() )
            echo( this, ECHO_LOG_WARNING, "Channel count does not match with locked on audio's." );
    }

    Sound( 
        std::string_view   path,
        Echo               echo   = {}
    ) {
        using namespace std::string_literals;


        std::ifstream file{ path.data(), std::ios_base::binary };

        if( !file ) {
            echo( this, ECHO_LOG_FAULT, "Open file: "s + path.data() ); return;
        }


        size_t file_size = File::size( file );

        Unique< char[] > file_stream{ new char[ file_size ] };

        if( !file_stream ) {
            echo( this, ECHO_LOG_FAULT, "File stream allocation." ); return;
        }


        file.read( file_stream, file_size );


        _sample_rate = Bytes::as< unsigned int >( file_stream + 24, 4, Bytes::LITTLE );


        _bits_per_sample = Bytes::as< unsigned short >( file_stream + 34, 2, Bytes::LITTLE );

        size_t bytes_per_sample = _bits_per_sample / 8;

        _sample_count = Bytes::as< size_t >( file_stream + 40, 4, Bytes::LITTLE )
                        /
                        bytes_per_sample;


        _stream = new double[ _sample_count ];

        if( !_stream ) {
            echo( this, ECHO_LOG_FAULT, "Sound stream allocation." ); return;
        }


        double max_sample = static_cast< double >( 1 << ( _bits_per_sample - 1 ) );

        for( size_t n = 0; n < _sample_count; ++n )
            _stream[ n ] = static_cast< double >(
                                Bytes::as< int >( file_stream + 44 + n * bytes_per_sample, bytes_per_sample, Bytes::LITTLE )
                            ) / max_sample;


        _channel_count = Bytes::as< unsigned short >( file_stream + 22, 2, Bytes::LITTLE );


        if( _sample_count % _channel_count != 0 )
            echo( this, ECHO_LOG_WARNING, "Samples do not condense." );

        
        _sample_count /= _channel_count;


        echo( this, ECHO_LOG_OK, "Created from: "s + path.data() );
    }

    Sound( const Sound& ) = default;

    Sound( Sound&& ) = delete;


    ~Sound() {
        stop();
    }

protected:
    Shared< double[] >    _stream             = nullptr;

    std::list< double >   _needles            = {};

    size_t                _sample_rate        = 0;
    size_t                _bits_per_sample    = 0;
    size_t                _sample_count       = 0;
    size_t                _channel_count      = 0;

public:
    virtual void prepare_play() override {
        _needles.push_back( 0.0 );
    }

    virtual void stop() override {
        _needles.clear();
    }

    virtual bool done() const override {
        return _needles.empty();
    }

protected:
    virtual double _sample( size_t channel, bool advance ) override {
        double amp = 0.0;

        if( _paused ) return amp;

        _needles.remove_if( [ this, &amp, &channel, &advance ] ( double& at ) {
            double raw = _stream[ static_cast< size_t >( at ) * _channel_count + channel ];

            amp +=  _filter ? _filter( raw, channel ) : raw
                    *
                    _volume * !_muted;

            
            if( advance ) {
                double tweak = _velocity * _audio->velocity();

                at += tweak;

                if( static_cast< size_t >( at ) >= _sample_count ) {
                    at = tweak >= 0.0 ? 0.0 : _sample_count - 1.0;

                    return !_looping;
                }
            }

            return false;
        } );

        return amp;
    }

public:
    bool is_locked() const {
        return _audio != nullptr;
    }

    bool has_stream() const {
        return _stream != nullptr;
    }

    operator bool () const {
        return this->is_locked() && this->has_stream();
    }

public:
    size_t sample_rate() const {
        return _sample_rate;
    }

    size_t channel_count() const {
        return _channel_count;
    }

    size_t sample_count() const {
        return _sample_count;
    }

    double duration() const {
        return static_cast< double >( _sample_count ) / _sample_rate;
    }

};



class Synth : public UTH,
              public Wave, 
              public WaveBehaviourDescriptor< Synth,
                  WAVE_BEHAVIOUR_DESC_HAS_VOLUME   |
                  WAVE_BEHAVIOUR_DESC_PAUSABLE     |
                  WAVE_BEHAVIOUR_DESC_MUTABLE      |
                  WAVE_BEHAVIOUR_DESC_HAS_VELOCITY |
                  WAVE_BEHAVIOUR_DESC_HAS_FILTER
              >
{
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "Synth" );

public:
    typedef   std::function< double( double, size_t ) >   Function;

public:
    Synth() = default;

    Synth( 
        Audio&     audio,
        Function   function,
        Echo       echo       = {}
    )
    : Synth{ function, echo }
    {
        _audio = &audio;

        _decay_step = 1.0 / _audio->sample_rate();
    }

    Synth(
        Function   function,
        Echo       echo       = {}
    )
    : _function( function )
    {
        echo( this, ECHO_LOG_OK, "Created from source function." );
    }


    Synth( const Synth& ) = default;

    Synth( Synth&& ) = delete;

protected:
    Function   _function        = {};

    double     _elapsed         = 0.0;

    double     _decay           = 1.0;
    double     _decay_step      = 0.0;

public:
    virtual void prepare_play() override {
        _elapsed = 0.0;
        _decay   = 1.0;
    }

    virtual void stop() override {
        _decay = 0.0;
    }

    virtual bool done() const override {
        return _decay == 0.0;
    }


protected:
    virtual double _sample( size_t channel, bool advance ) override {
        if( advance )
            _elapsed += _audio->time_step();

        _decay = std::clamp( _decay - _decay_step, 0.0, 1.0 ); 

        return _decay * _function( _elapsed, channel ) * _volume;
    }

public:
    Synth& decay_in( double secs ) {
        _decay_step = 1.0 / ( secs * _audio->sample_rate() );

        return *this;
    }

};



bool Wave::is_playing() const {
    return std::find( _audio->_waves.begin(), _audio->_waves.end(), this )
           !=
           _audio->_waves.end();
}

void Wave::play() {
    this->prepare_play();

    if( !is_playing() )
        _audio->_waves.push_back( this );
}



#pragma endregion Audio



#pragma region After



#pragma region Echo



const Echo& Echo::_echo(
    UTH*               invoker,
    ECHO_LOG           log_type,
    std::string_view   message
) const {
    #if defined( _ENGINE_ECHO )
        std::cout << " [ "_echo_normal;

        OS::console_color_to( log_type );

        std::cout << _log_type_name( log_type ) << " ] "_echo_normal << _log_type_fill( log_type ) << "   "_echo_special;

        for( size_t l = 0; l < _depth; ++l )
            std::cout << ">> ";

        std::cout
        << "From "_echo_normal
        << "[ "
        << ""_echo_highlight
        << invoker->type_name()
        << " ][ "_echo_normal
        << ""_echo_highlight
        << static_cast< void* >( invoker )
        << " ]  "_echo_normal
        << "->  "_echo_special
        << ""_echo_normal
        << message
        << "\n"_echo_normal;
    #endif

    return *this;
}



#pragma endregion Echo



#pragma region Renderer



Renderer2& Renderer2::line(
    Coord<> c1, Coord<> c2,
    const Brush2& brush
) {
    _target->DrawLine(
        c1, c2,
        brush.brush(),
        brush.width()
    );

    return *this;
}

Renderer2& Renderer2::line(
    Vec2 v1, Vec2 v2,
    const Brush2& brush
) {
    return line(
        this->pull_crd( v1 ),
        this->pull_crd( v2 ),
        brush
    );
}

Renderer2& Renderer2::fill( const Chroma& chroma = {} ) {
    _target->Clear( chroma );

    return *this;
}

Renderer2& Renderer2::system( 
    const Brush2& brush, 
    float         div, 
    float         div_half_width
) {
    Size< float > size{
        static_cast< float >( this->size().width ),
        static_cast< float >( this->size().height )
    };

    Coord<> mid{ size.width / 2.0f, size.height / 2.0f };

    this->line( 
        Coord<>{ mid.x, 0.0f }, Coord<>{ mid.x, size.height },
        brush
    );

    this->line( 
        Coord<>{ 0.0f, mid.y }, Coord<>{ size.width, mid.y },
        brush
    );


    for( float off = div; off <= mid.x; off += div ) {
        this->line(
            Coord<>{ mid.x + off, mid.y - div_half_width },
            Coord<>{ mid.x + off, mid.y + div_half_width },
            brush  
        );

        this->line(
            Coord<>{ mid.x - off, mid.y - div_half_width },
            Coord<>{ mid.x - off, mid.y + div_half_width },
            brush  
        );
    }

    for( float off = div; off <= mid.y; off += div ) {
        this->line(
            Coord<>{ mid.x - div_half_width, mid.y + off },
            Coord<>{ mid.x + div_half_width, mid.y + off },
            brush  
        );

        this->line(
            Coord<>{ mid.x - div_half_width, mid.y - off },
            Coord<>{ mid.x + div_half_width, mid.y - off },
            brush  
        );
    }


    return *this;
}



#pragma endregion Renderer



#pragma endregion After



#pragma region Builds



class System2 : public UTH {
public:
    _ENGINE_UTH_IDENTIFY_METHOD( "System2" );

public:
    System2() = default;

    System2(
        Viewport2&      vwprt,
        double          div_every,
        double          div_hstk,
        double          div_mean,
        const Chroma&   axis_chroma,
        float           axis_width,
        Echo            echo          = {}
    )
    : _viewport{ &vwprt }, _div_every{ div_every }, _div_mean{ div_mean }, _div_hstk{ div_hstk },
      _brush{ vwprt.renderer(), axis_chroma, axis_width, echo }
    {
        echo( this, ECHO_LOG_OK, "Created." );
    }

public:
    class Link {
    public:
        Link() = default;

    public:
        typedef   std::function< double( double ) >   Function;
        typedef   std::vector< Vec2 >                 Collection;
    
    protected:
        std::variant< Function, Collection >   _source   = {};

    public:
        bool is_function() const {
            return _source.index() == 0;
        }

        bool is_collection() const {
            return _source.index() == 1;
        }

    public:
        Function& function() {
            return std::get< 0 >( _source );
        } 

        Collection& collection() {
            return std::get< 1 >( _source );
        }

    };

private:
    Viewport2*                      _viewport    = nullptr;
    Vec2                            _offset      = {};

    double                          _div_every   = 30.0;
    double                          _div_hstk    = 15.0;
    double                          _div_mean    = 0.0;

    SolidBrush2                     _brush       = {};

    std::map< std::string, Link >   _links   = {};

public:
    Viewport2& viewport() {
        return *_viewport;
    }

public:
    System2& offset_to( Vec2 vec ) {
        _offset = vec;

        return *this;
    }

    Vec2 offset() const {
        return _offset;
    }

public:
    template< typename Source >
    requires (
        std::is_same_v< Link::Function, Source >
        ||
        std::is_same_v< Link::Collection, Source >
    )
    System2& push( std::string_view name, const Source& source ) {

    }

public:
    void render( Renderer2& renderer ) const {
        double    cst        = 0.0;
        double    cst_stk    = 0.0;
        bool      axis_out[] = { false, false, false, false };


        if( ( axis_out[ HEADING_EAST ] = ( _offset.x > _viewport->east_reach() ) )
            || 
            ( axis_out[ HEADING_WEST ] = ( _offset.x < _viewport->west_reach() ) )
        )
            goto L_SKIP_Y_AXIS;


        cst = _viewport->origin().x + _offset.x;

        renderer.line(
            Vec2{ cst, _viewport->north() }, 
            Vec2{ cst, _viewport->south() },
            _brush
        );


        L_SKIP_Y_AXIS:


        if( ( axis_out[ HEADING_NORTH ] = ( _offset.y > _viewport->north_reach() ) )
            || 
            ( axis_out[ HEADING_SOUTH ] = ( _offset.y < _viewport->south_reach() ) )
        )
            goto L_SKIP_X_AXIS;

        cst = _viewport->origin().y + _offset.y;

        renderer.line(
            Vec2{ _viewport->west(), cst }, 
            Vec2{ _viewport->east(), cst },
            _brush
        );

        
        L_SKIP_X_AXIS:


        cst     = std::clamp( _viewport->origin().x + _offset.x, _viewport->west(), _viewport->east() );
        cst_stk = _viewport->origin().y + _offset.y;

        auto strike_x_at = [ this, &renderer, &cst_stk, &axis_out ] ( double x ) -> void {
            renderer.line(
                Vec2{ 
                    x, 
                    axis_out[ HEADING_SOUTH ] ?
                        std::min( _viewport->south() + _div_hstk, _viewport->north() )
                        :
                        std::min( _viewport->north(), cst_stk + _div_hstk )
                },
                Vec2{ 
                    x, 
                    axis_out[ HEADING_NORTH ] ?
                        std::max( _viewport->north() - _div_hstk, _viewport->south() )
                        :
                        std::max( _viewport->south(), cst_stk - _div_hstk ) 
                },
                _brush
            );
        };

        for( double offs = 0.0; ( cst + offs ) <= _viewport->east(); offs += _div_every ) 
            strike_x_at( cst + offs );
        
        for( double offs = 0.0; ( cst - offs ) >= _viewport->west(); offs += _div_every ) 
            strike_x_at( cst - offs );


        cst     = std::clamp( _viewport->origin().y + _offset.y, _viewport->south(), _viewport->north() );
        cst_stk = _viewport->origin().x + _offset.x;

        auto strike_y_at = [ this, &renderer, &cst_stk, &axis_out ] ( double y ) -> void {
            renderer.line(
                Vec2{ 
                    axis_out[ HEADING_WEST ] ?
                        std::min( _viewport->west() + _div_hstk, _viewport->east() )
                        :
                        std::min( _viewport->east(), cst_stk + _div_hstk ),
                    y
                },
                Vec2{
                    axis_out[ HEADING_EAST ] ?
                        std::max( _viewport->east() - _div_hstk, _viewport->west() )
                        :
                        std::max( _viewport->west(), cst_stk - _div_hstk ),
                    y
                },
                _brush
            );
        };

        for( double offs = 0.0; ( cst + offs ) <= _viewport->north(); offs += _div_every ) 
            strike_y_at( cst + offs );

        for( double offs = 0.0; ( cst - offs ) >= _viewport->south(); offs += _div_every ) 
            strike_y_at( cst - offs );

    }

};



#pragma endregion Builds



};
