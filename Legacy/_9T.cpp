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
        IXT_ECHO --- logs stuff.
        IXT_UNIQUE_SURFACE --- Enables faster event routing when using only one surface.

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



#pragma region DEFINES



#define PI 3.141592653

#define GFTYPE double


#define _ENGINE_NAMESPACE IXT

#define _ENGINE_ECHO_ADD_PREFIX( type ) "IXT::" type

#define _ENGINE_ECHO_IDENTIFY_METHOD( type ) virtual std::string_view echo_name() const override { return _ENGINE_ECHO_ADD_PREFIX( type ); }

#define _ENGINE_ECHO_DFD_ARG Echo echo = {}


#if defined( IXT_ECHO )
        #define _ENGINE_ECHO
#endif

#if defined( IXT_UNIQUE_SURFACE )
    #define _ENGINE_UNIQUE_SURFACE
#endif

#if defined( IXT_THROW_ON_FAULT )
    #define ENGINE_THROW_ON_FAULT
#endif

#if defined( IXT_ALL_PUBLIC )
    #define _ENGINE_PROTECTED public
#else
    #define _ENGINE_PROTECTED protected
#endif

#if defined( IXT_OS_WINDOWS )
    #define _ENGINE_OS_WINDOWS
#endif

#if defined( IXT_GL_DIRECT )
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

class EchoInvoker {
public:
    virtual std::string_view echo_name() const {
        return _ENGINE_ECHO_ADD_PREFIX( "EchoInvoker" );
    }

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

_ENGINE_PROTECTED:
    size_t   _depth   = 0;

public:
    const Echo& operator () (
        EchoInvoker*       invoker,
        ECHO_LOG           log_type,
        std::string_view   message
    ) const {
        return std::invoke( _route, this, invoker, log_type, message );
    }

_ENGINE_PROTECTED:
    const Echo& _echo(
        EchoInvoker*       invoker,
        ECHO_LOG           log_type,
        std::string_view   message
    ) const;

    const Echo& _nop(
        EchoInvoker*       invoker,
        ECHO_LOG           log_type,
        std::string_view   message
    ) const {
        return *this;
    }

_ENGINE_PROTECTED:
    inline static auto   _route   = &_echo;

public:
    static void high() {
        _route = &_echo;
    }

    static void low() {
        _route = &_nop;
    }

_ENGINE_PROTECTED:
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



template< typename > class Unique;
template< typename > class Shared;



template< 
    typename T, 
    template< typename > typename Smart_ptr, 
    template< typename > typename Extended_ptr 
>
class _Smart_ptr_extended : public Smart_ptr< T > {
public:
    using Base = Smart_ptr< T >;
    using Ext  = Extended_ptr< T >;
    
public:
    using Base::Base;

public:
    inline static constexpr bool   is_array   = std::is_array_v< T >;

public:
    typedef   T   type;

    typedef   std::conditional_t< is_array, std::decay_t< T >, T* >   T_ptr;
    typedef   std::remove_pointer_t< T_ptr >&                         T_ref;

public:
    Ext& operator = ( T_ptr ptr ) {
        this->reset( ptr );

        return static_cast< Ext& >( *this );
    }

public:
    operator T_ptr () const {
        return this->get();
    }

    operator T_ref () {
        return *this->get();
    }

public:
    T_ptr operator + ( ptrdiff_t offs ) const {
        return this->get() + offs;
    }

    T_ptr operator - ( ptrdiff_t offs ) const {
        return this->get() - offs;
    }

};


template< typename T >
class Unique : public _Smart_ptr_extended< T, std::unique_ptr, Unique > {
public:
    typedef   _Smart_ptr_extended< T, std::unique_ptr, Unique >   Base;

public:
    using Base::Base;

    using Base::operator =;

public:
    template< typename ...Args >
    [[ nodiscard ]] static Unique make( Args&&... args ) {
        return { std::make_unique< T >( std::forward< Args >( args )... ) };
    }

public:
    [[ nodiscard ]] Shared< T > branch();

};



template< typename T >
class Shared : public _Smart_ptr_extended< T, std::shared_ptr, Shared > {
public:
    typedef   _Smart_ptr_extended< T, std::shared_ptr, Shared >   Base;

public:
    using Base::Base;

    using Base::operator =;

public:
    template< typename ...Args >
    [[ nodiscard ]] static Shared make( Args&&... args ) {
        return { std::make_shared< T >( std::forward< Args >( args )... ) };
    }

};



template< typename T >
Shared< T > Unique< T >::branch() {
    T* raw = this->get();

    this->release();

    return { raw };
}



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

_ENGINE_PROTECTED:
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

_ENGINE_PROTECTED:
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

_ENGINE_PROTECTED:
    typedef std::tuple<
                Unique< std::mutex >,
                Unique< std::condition_variable >,
                std::function< bool( const T& ) >
            > Entry;

    enum _TUPLE_ACCESS_INDEX {
        _MTX = 0, _CND = 1, _OP = 2
    };

_ENGINE_PROTECTED:
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



template< typename Type = GFTYPE >
struct Coord {
    Coord() = default;

    Coord( Type x, Type y )
    : x{ x }, y{ y }
    {}

    template< typename TypeOther >
    Coord( const Coord< TypeOther >& other )
    : x{ static_cast< Type >( other.x ) }, y{ static_cast< Type >( other.y ) }
    {}


    Type   x   = {};
    Type   y   = {};


    template< typename TypeOther >
    Coord< Type > operator + ( const TypeOther& other ) const {
        return { x + other.x, y + other.y };
    } 

    template< typename TypeOther >
    Coord< Type > operator - ( const TypeOther& other ) const {
        return { x - other.x, y - other.y };
    } 


    template< bool is_float = std::is_same_v< float, Type > >
    operator std::enable_if_t< is_float, const D2D1_POINT_2F& > () const {
        return *reinterpret_cast< const D2D1_POINT_2F* >( this );
    }

    template< bool is_float = std::is_same_v< float, Type > >
    operator std::enable_if_t< is_float, D2D1_POINT_2F& > () {
        return *reinterpret_cast< D2D1_POINT_2F* >( this );
    }

};

template< typename Type = GFTYPE >
struct Size {
    Size() = default;

    Size( Type width, Type height )
        : width( width ), height( height )
    {}

    template< typename TypeOther >
    Size( const Size< TypeOther >& other )
        : width( static_cast< Type >( other.width ) ), height( static_cast< Type >( other.height ) )
    {}

    Type   width    = {};
    Type   height   = {};

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



class Codec {
public:
    struct Wav : public EchoInvoker {
        _ENGINE_ECHO_IDENTIFY_METHOD( "Codec::Wav" );


        Wav() = default;

        Wav( std::string_view path, _ENGINE_ECHO_DFD_ARG ) {
            using namespace std::string_literals;


            std::ifstream file{ path.data(), std::ios_base::binary };

            ECHO_ASSERT_AND_THROW( file, "Open file: "s + path.data() );


            size_t file_size = File::size( file );

            Unique< char[] > file_stream{ new char[ file_size ] };


            ECHO_ASSERT_AND_THROW( file_stream, "Buffer alloc to stream file." );


            file.read( file_stream, file_size );


            sample_rate = Bytes::as< unsigned int >( file_stream + 24, 4, Bytes::LITTLE );


            bits_per_sample = Bytes::as< unsigned short >( file_stream + 34, 2, Bytes::LITTLE );

            size_t bytes_per_sample = bits_per_sample / 8;

            sample_count = Bytes::as< size_t >( file_stream + 40, 4, Bytes::LITTLE )
                           /
                           bytes_per_sample;


            stream = new double[ sample_count ];

            ECHO_ASSERT_AND_THROW( stream, "Buffer alloc for stream." );


            double max_sample = static_cast< double >( 1 << ( bits_per_sample - 1 ) );

            for( size_t n = 0; n < sample_count; ++n )
                stream[ n ] = static_cast< double >(
                                  Bytes::as< int >( file_stream + 44 + n * bytes_per_sample, bytes_per_sample, Bytes::LITTLE )
                              ) / max_sample;


            channel_count = Bytes::as< unsigned short >( file_stream + 22, 2, Bytes::LITTLE );


            if( sample_count % channel_count != 0 )
                echo( this, ECHO_LOG_WARNING, "Samples do not fill all channels." );

            
            sample_count /= channel_count;


            echo( this, ECHO_LOG_OK, "Created from: "s + path.data() );
        }
   

        Shared< double[] >   stream            = nullptr;

        size_t               sample_rate       = 0;
        size_t               bits_per_sample   = 0;
        size_t               sample_count      = 0;
        size_t               channel_count     = 0;


        static inline Wav from_file( std::string_view path, _ENGINE_ECHO_DFD_ARG ) {
            return { path, echo };
        }

    };

};



#pragma endregion Utility



#pragma region Algorithms



template< typename Itr >
void sort_simple( Itr begin, Itr end ) {
    for( Itr at = begin; ++at != end; )
        for( Itr up = at; up != begin; --up )
            if( Itr lo = up; *--lo > *up )
                std::swap( *lo, *up );
}

template< typename Itr >
void sort_selection( Itr begin, Itr end ) {
    for( ; begin != end; ++begin ) {
        Itr min = begin;

        for( Itr at = begin; ++at != end; )
            if( *at < *min )
                min = at;

        std::swap( *begin, *min );
    }
}

template< typename Itr >
void sort_insertion_1( Itr begin, Itr end ) {
    for( Itr at = begin ; ++at != end; )
        for( Itr up = at; up != begin; --up )
            if( Itr lo = up; *--lo > *up )
                std::swap( *lo, *up );
            else
                break;
}

template< typename Itr >
void sort_insertion_2( Itr begin, Itr end ) {
    for( Itr at = end; --at != begin; )
        if( Itr b4 = at; *--b4 > *at )
            std::swap( *b4, *at );

    ++begin; if( begin == end ) return;

    for( ; ++begin != end; ) {
        Itr up = begin;
        Itr lo = begin;

        while( *--lo > *up )
            std::swap( *lo, *up-- );
    }
}

template< typename Itr >
void sort_bubble( Itr begin, Itr end ) {
    for( Itr at = begin; at != end; ++at )
        for( Itr up = end; --up != at; )
            if( Itr lo = up; *--lo > *up )
                std::swap( *lo, *up );
}

template< typename Itr >
void sort_shaker( Itr begin, Itr end ) {
    Itr shaker = begin;

    for( Itr at = end; --at != begin; )
        if( Itr b4 = at; *--b4 > *at )
            std::swap( *b4, *at );
    
    --end;

    for( Itr at = begin; ++at != end; )
        if( Itr b4 = at; *--b4 > *at )
            std::swap( *b4, *at );

    ++begin;

    static auto fwd = [] ( Itr& shaker ) -> void { if( Itr prev = shaker++; *prev > *shaker ) std::swap( *prev, *shaker ); };
    static auto rev = [] ( Itr& shaker ) -> void { if( Itr next = shaker--; *next < *shaker ) std::swap( *next, *shaker ); };

    while( begin != end ) {
        while( shaker != end ) fwd( shaker ); --end;

        if( begin == end ) break;

        while( shaker != begin ) rev( shaker ); ++begin;
    }
}

template< typename Itr >
void sort_shell( Itr begin, Itr end ) {
    static ptrdiff_t sequence[] = { 505, 109, 19, 1 }; 

    struct IncItr : public Itr {
        using Itr :: Itr;

        IncItr() = default;

        IncItr( Itr other )
        : Itr{ other }
        {}

        ptrdiff_t inc   = 1;
        Itr       begin = {};
        Itr       end   = {};

        IncItr operator ++ () {
            for( size_t n = 1; n <= inc; ++n )
                if( *this != end )
                    this->Itr::operator++();
                else 
                    break;
            
            return *this;
        }

        IncItr operator ++ ( int ) {
            IncItr itr = *this;

            ++*this;

            return itr;
        }

        IncItr operator -- () {
            for( size_t n = 1; n <= inc; ++n )
                if( *this != begin )
                    this->Itr::operator--();
                else
                    break;

            return *this;
        }

        IncItr operator -- ( int ) {
            IncItr itr = *this;

            --*this;

            return itr;
        }

    };

    for( size_t idx = 0; idx < std::size( sequence ); ++idx ) {
        IncItr itr{ begin }; std::advance( itr, idx );

        itr.inc = sequence[ idx ];
        itr.begin = begin; itr.end = end;

        sort_insertion_2( itr, IncItr{ end } );
    }
}

template< typename Itr >
void sort_quick( Itr begin, Itr end ) {
    static auto partition = [] ( Itr begin, Itr end ) -> Itr {
        Itr part = end;
        Itr cend = end;

        if( end <= begin ) return begin;

        end -= 1;


        while( true ) {
            while( *begin < *part ) ++begin;
            while( begin < end && *part < *end ) --end;

            if( begin >= end ) break;

            std::swap( *begin, *end );

            ++begin; --end;
        }

        std::swap( *begin, *cend );

        return begin;
    };

    if( end <= begin ) return;

    Itr itr = partition( begin, end );

    sort_quick( begin, itr - 1 );
    sort_quick( itr + 1, end );
}



template< typename Stored >
requires requires {
    Stored{} < Stored{};
}
class Shrub2 {
public:
    class Node {
    public:
        friend class Shrub2;

    _ENGINE_PROTECTED:
        Stored   _stored   = {};

        Node*    _left     = nullptr;
        Node*    _right    = nullptr;
        Node*    _above    = nullptr;

        int      _height   = 0;

    public:
        auto operator <=> ( const Node& other ) const {
            return _stored <=> other._stored;
        }

        auto operator <=> ( const Stored& other ) const {
            return _stored <=> other;
        }

    public:
        int balance() const {
           return ( _right ? _right->_height : -1 ) - ( _left ? _left->_height : -1 );
        }

    };

public:
    Shrub2() = default;

_ENGINE_PROTECTED:
    Node*   _root   = nullptr;

public:
    void push( const Stored& stored ) {
        if( this->_push_root( stored ) ) return;

        Node* ptr = _root;

        L_JMP_DOWN:
            if( stored < *ptr ) {
                if( ptr->_left ) { ptr = ptr->_left; goto L_JMP_DOWN; }

                ptr->_left = new Node{ stored };
                ptr->_left->_above = ptr;
            }
            else {
                if( ptr->_right ) { ptr = ptr->_right; goto L_JMP_DOWN; }

                ptr->_right = new Node{ stored };
                ptr->_right->_above = ptr;
            }

        int height = 1;

        L_JMP_UP:
            if( ptr && ptr->_height < height ) { 
                ++ptr->_height; ++height;

                _balance_node( ptr );

                ptr = ptr->_above; 
                
                goto L_JMP_UP; 
            }
    }

_ENGINE_PROTECTED:
    bool _push_root( const Stored& stored ) {
        if( _root ) return false;

        _root = new Node{ stored };

        return true;
    }

    void _balance_node( Node* ptr ) {
        int balance = ptr->balance();

        if( abs( balance ) <= 1 ) return;

        if( balance > 1 ) {
            if( ptr->_right && ptr->_right->balance() <= -1 )
                ptr = _rotate_right( ptr );

            _rotate_left( ptr );
        } else {
            if( ptr->_left && ptr->_left->balance() >= 1 )
                ptr = _rotate_left( ptr );

            _rotate_right( ptr );
        }
    }

    Node* _rotate_right( Node* ptr ) {
        if( !ptr || !ptr->_left ) return;

        Node* aux = ptr->_left->_right;

        if( ptr->_above ) {
            if( ptr->_above->_left == ptr ) 
                ptr->_above->_left = ptr->_left;
            else 
                ptr->_above->_right = ptr->_left;
        }

        ptr->_left->_right = ptr;
        
        std::swap( ptr->_left, aux );
        
        do {
            ptr->_height = std::max( 
                ptr->_left  ? ptr->_left->_height  : -1, 
                ptr->_right ? ptr->_right->_height : -1 
            );

            if( ptr->_above && ( ptr->_above->_height == ptr->_height + 1 ) ) break;

            ptr = ptr->_above;
        } while( ptr );

        return aux;
    } 

    Node* _rotate_left( Node* ptr ) {
        if( !ptr || !ptr->_right ) return;

        Node* aux = ptr->_right->_left;

        if( ptr->_above ) {
            if( ptr->_above->_left == ptr ) 
                ptr->_above->_left = ptr->_right;
            else 
                ptr->_above->_right = ptr->_right;
        }

        ptr->_right->_left = ptr;
        
        std::swap( ptr->_right, aux );

        do {
            ptr->_height = std::max( 
                ptr->_left  ? ptr->_left->_height  : -1, 
                ptr->_right ? ptr->_right->_height : -1 
            );

            if( ptr->_above && ( ptr->_above->_height == ptr->_height + 1 ) ) break;

            ptr = ptr->_above;
        } while( ptr );

        return aux;
    }

};



#pragma endregion Algorithms



#pragma region UTH



typedef   const void*   GUID;

class UTH : public EchoInvoker {
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
    static GFTYPE pull( GFTYPE theta ) {
        return theta * ( 180.0 / PI );
    }

    static void push( GFTYPE& theta ) {
        theta *= ( 180.0 / PI );
    }
};

class Rad {
public:
    static GFTYPE pull( GFTYPE theta ) {
        return theta * ( PI / 180.0 );
    }

    static void push( GFTYPE& theta ) {
        theta *= ( PI / 180.0 );
    }
};



#pragma region D2



class Vec2 {
public:
    Vec2() = default;

    Vec2( GFTYPE x, GFTYPE y )
    : x{ x }, y{ y }
    {}

    Vec2( GFTYPE x )
    : Vec2{ x, x }
    {}

public:
    GFTYPE   x   = 0.0;
    GFTYPE   y   = 0.0;

public:
    GFTYPE dot( const Vec2& other ) const {
        return x * other.x + y * other.y;
    }

public:
    GFTYPE mag_sq() const {
        return x * x + y * y;
    }

    GFTYPE mag() const {
        return sqrt( this->mag_sq() );
    }

    GFTYPE angel() const {
        return Deg::pull( atan2( y, x ) );
    }

public:
    GFTYPE dist_sq_to( const Vec2& other ) const {
        return ( other.x - x ) * ( other.x - x ) + ( other.y - y ) * ( other.y - y );
    }

    GFTYPE dist_to( const Vec2& other ) const {
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
    Vec2& normalize() {
        return *this /= this->mag();
    }

    Vec2 normalized() const {
        return Vec2{ *this }.normalize();
    }

    Vec2& abs() {
        return x = std::abs( x ), y = std::abs( y ), *this;
    }

    Vec2 absd() const {
        return Vec2{ *this }.abs();
    }

public:
    Vec2& polar( GFTYPE angel, GFTYPE dist ) {
        Rad::push( angel );

        x += cos( angel ) * dist;
        y += sin( angel ) * dist;

        return *this;
    }

    Vec2 polared( GFTYPE angel, GFTYPE dist ) const {
        return Vec2{ *this }.polar( angel, dist );
    }


    Vec2& approach( const Vec2 other, GFTYPE dist ) {
        return this->polar( other.respect_to( *this ).angel(), dist );
    }

    Vec2 approached( const Vec2 other, GFTYPE dist ) const {
        return Vec2{ *this }.approach( other, dist );
    }


    Vec2& spin( GFTYPE theta ) {
        Rad::push( theta );

        GFTYPE nx = x * cos( theta ) - y * sin( theta );
        y = x * sin( theta ) + y * cos( theta );
        x = nx;

        return *this;
    }

    Vec2& spin( GFTYPE theta, const Vec2& other ) {
        *this = this->respect_to( other ).spin( theta ) + other;

        return *this;
    }

    Vec2 spinned( GFTYPE theta ) const {
        return Vec2{ *this }.spin( theta );
    }

    Vec2 spinned( GFTYPE theta, const Vec2& other ) const {
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
    Vec2& operator = ( const Vec2& other ) {
        x = other.x; y = other.y; return *this;
    }

    Vec2& operator = ( GFTYPE val ) {
        x = y = val; return *this;
    }

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

    Vec2 operator + ( GFTYPE delta ) const {
        return { x + delta, y + delta };
    }

    Vec2 operator - ( GFTYPE delta ) const {
        return { x - delta, y - delta };
    }

    Vec2 operator * ( GFTYPE delta ) const {
        return { x * delta, y * delta };
    }

    Vec2 operator / ( GFTYPE delta ) const {
        return { x / delta, y / delta };
    }

    Vec2 operator >> ( GFTYPE delta ) const {
        return { x + delta, y };
    }

    Vec2 operator ^ ( GFTYPE delta ) const {
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

    Vec2& operator *= ( GFTYPE delta ) {
        x *= delta;
        y *= delta;

        return *this;
    }

    Vec2& operator /= ( GFTYPE delta ) {
        x /= delta;
        y /= delta;

        return *this;
    }

    Vec2& operator >>= ( GFTYPE delta ) {
        x += delta;

        return *this;
    }

    Vec2& operator ^= ( GFTYPE delta ) {
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



class Crd2 : public Vec2 {
public:
    using Base = Vec2;

public:
    using Base::Base;

    using Base::operator =;

public:
    Crd2( const Vec2& vec )
    : Vec2{ vec }
    {}

public:
    operator const D2D1_POINT_2F& () const {
        return *reinterpret_cast< const D2D1_POINT_2F* >( this );
    }

    operator D2D1_POINT_2F& () {
        return *reinterpret_cast< D2D1_POINT_2F* >( this );
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
    GFTYPE slope() const {
        return ( this->drop().y - origin.y ) / ( this->drop().x - origin.x );
    }

    GFTYPE y_int() const {
        return origin.y - this->slope() * origin.x;
    }

    std::tuple< GFTYPE, GFTYPE, GFTYPE > coeffs() const {
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

_ENGINE_PROTECTED:
    std::optional< Vec2 > _intersect_vec( const Ray2& other ) const {
        /* Hai noroc nea' Peter +respect. */

        static auto has_any_normal_component = [] ( Vec2 normalized ) -> bool {
            return ( normalized.x >= 0.0 && normalized.x <= 1.0 )
                    ||
                    ( normalized.y >= 0.0 && normalized.y <= 1.0 );
        };

        auto [ alpha, bravo, charlie ] = this->coeffs();
        auto [ delta, echo, foxtrot ] = other.coeffs();

        GFTYPE golf = alpha * echo - bravo * delta;

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
    : Clust2( std::begin( container ), std::end( container ) )
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
    : Clust2( org, std::begin( container ), std::end( container ), offs )
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

_ENGINE_PROTECTED:
    typedef   std::variant< Vec2, std::pair< Clust2*, Vec2 > >   Origin;
    typedef   std::pair< Vec2, Vec2 >                            Vrtx;

    enum ORIGIN_VARIANT_ACCESS_INDEX {
        OVAI_VEC  = 0,
        OVAI_HOOK = 1
    };

_ENGINE_PROTECTED:
    Origin                _origin   = Vec2{ 0.0, 0.0 };
    std::vector< Vrtx >   _vrtx     = {};

    GFTYPE                _scaleX   = 1.0;
    GFTYPE                _scaleY   = 1.0;
    GFTYPE                _angel    = 0.0;

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
    GFTYPE angel() const {
        return _angel;
    }

    GFTYPE scaleX() const {
        return _scaleX;
    }

    GFTYPE scaleY() const {
        return _scaleY;
    }

    GFTYPE scale() const {
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
    Clust2& spin_with( GFTYPE theta ) {
        _angel += theta;

        this->_refresh();

        return *this;
    }

    Clust2& spin_at( GFTYPE theta ) {
        _angel = theta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleX_with( GFTYPE delta ) {
        _scaleX *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleY_with( GFTYPE delta ) {
        _scaleY *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scale_with( GFTYPE delta ) {
        _scaleX *= delta;
        _scaleY *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleX_at( GFTYPE delta ) {
        _scaleX = delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleY_at( GFTYPE delta ) {
        _scaleY = delta;

        this->_refresh();

        return *this;
    }

    Clust2& scale_at( GFTYPE delta ) {
        _scaleX = _scaleY = delta;

        this->_refresh();

        return *this;
    }

public:
    static Clust2 triangle( GFTYPE edge_length ) {
        Vec2 vrtx = { 0.0, edge_length * sqrt( 3.0 ) / 3.0 };

        return std::vector< Vec2 >( {
            vrtx,
            vrtx.spinned( 120.0 ),
            vrtx.spinned( -120.0 )
        } );
    }

    static Clust2 square( GFTYPE edge_length ) {
        edge_length /= 2.0;

        return std::vector< Vec2 >( {
            { edge_length, edge_length },
            { edge_length, -edge_length },
            { -edge_length, -edge_length },
            { -edge_length, edge_length }
        } );
    }

    static Clust2 circle( GFTYPE radius, size_t precision ) {
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
        GFTYPE min_dist, GFTYPE max_dist,
        size_t min_ec, size_t max_ec,
        const T& generator
    ) {
        static auto scalar = [] ( const auto& generator, GFTYPE min ) -> GFTYPE {
            return ( static_cast< GFTYPE >( std::invoke( generator ) % 10001 ) / 10000 )
                    * ( 1.0 - min ) + min;
        };

        size_t edge_count = std::invoke( generator ) % ( max_ec - min_ec + 1 ) + min_ec;

        Vec2 vrtx[ edge_count ];

        vrtx[ 0 ] = { 0.0, max_dist };

        GFTYPE diff = 360.0 / edge_count;


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

_ENGINE_PROTECTED:
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
    bool DEPRECATED_contains( const Vec2& vec ) const {
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

    bool contains( Vec2 vec ) const {
        size_t intersections = 0;
        bool   shift_dir     = false;

        for( size_t idx = 0; idx < this->vrtx_count(); ++idx ) {
            Ray2 edge = this->_mkray( idx, ( idx + 1 ) % this->vrtx_count() );

            if( vec.y == edge.drop().y ) {
                constexpr uint64_t mask = 0xFFFF'0000'0000'0000;

                uint64_t& r = *reinterpret_cast< uint64_t* >( &vec.y );

                uint64_t sgnxexp = r & mask;
                --r;
                r = sgnxexp | ( r & ~mask );
            }

            if( ( vec.y - edge.origin.y ) * ( vec.y - edge.drop().y ) > 0.0 )
                continue;

            if( edge.vec.x == 0.0 ) {
                if( edge.origin.x >= vec.x )
                    goto L_INTERSECTED;
                
                continue;
            }

            if( ( vec.y - edge.y_int() ) / edge.slope() <= vec.x )
                continue;

            L_INTERSECTED:

            ++intersections; 
        }

        return intersections % 2;
    }

_ENGINE_PROTECTED:
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

public:
    void render( Renderer2& renderer, const Brush2& brush );

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



#pragma region D3



#define _ENGINE_MAT3_MUL_AT( x, y ) ( \
    this->at( x, 0 ) * other.at( 0, y ) \
    + \
    this->at( x, 1 ) * other.at( 1, y ) \
    + \
    this->at( x, 2 ) * other.at( 2, y ) ) 

class Mat3 {
public:
    using Array = std::array< GFTYPE, 9 >;

public:
    Mat3() = default;

    Mat3( Array arr ) 
    : _arr{ arr }
    {} 

_ENGINE_PROTECTED:
    Array   _arr   = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

public:
    GFTYPE* operator [] ( size_t idx ) {
        return &_arr[ idx ];
    }

    GFTYPE& operator () ( size_t x, size_t y ) {
        return _arr[ x * 3 + y ];
    }

    GFTYPE& at( size_t x, size_t y ) {
        return _arr[ x * 3 + y ];
    }

public:
    GFTYPE det() const {
        return
        this->at( 0, 0 ) * this->at( 1, 1 ) * this->at( 2, 2 )
        +
        this->at( 0, 1 ) * this->at( 1, 2 ) * this->at( 2, 0 )
        +
        this->at( 0, 2 ) * this->at( 2, 1 ) * this->at( 1, 0 )
        -
        this->at( 0, 2 ) * this->at( 1, 1 ) * this->at( 2, 0 )
        -
        this->at( 0, 1 ) * this->at( 1, 0 ) * this->at( 2, 2 )
        -
        this->at( 0, 0 ) * this->at( 1, 2 ) * this->at( 2, 1 );
    }

public:
    Mat3& operator *= ( const Mat3& other ) {
        return *this = *this * other;
    }

    Mat3 operator * ( const Mat3& other ) const {
        return Array{
            _ENGINE_MAT3_MUL_AT( 0, 0 ),
            _ENGINE_MAT3_MUL_AT( 0, 1 ),
            _ENGINE_MAT3_MUL_AT( 0, 2 ),
            _ENGINE_MAT3_MUL_AT( 1, 0 ),
            _ENGINE_MAT3_MUL_AT( 1, 1 ),
            _ENGINE_MAT3_MUL_AT( 1, 2 ),
            _ENGINE_MAT3_MUL_AT( 2, 0 ),
            _ENGINE_MAT3_MUL_AT( 2, 1 ),
            _ENGINE_MAT3_MUL_AT( 2, 2 )
        };
    }

public:
    static Mat3 translate( GFTYPE tx, GFTYPE ty ) {
        return Array{ 1, 0, tx, 0, 1, ty, 0, 0, 1 };
    }

    static Mat3 translate( const Vec2& t ) {
        return translate( t.x, t.y );
    }

    static Mat3 scale( GFTYPE sx, GFTYPE sy ) {
        return Array{ sx, 0, 0, 0, sy, 0, 0, 0, 1 };
    }

    static Mat3 scale( Vec2 s ) {
        return scale( s.x, s.y );
    }

    static Mat3 rotate( GFTYPE theta ) {
        Rad::push( theta );

        auto cosine = cos( theta );
        auto sine   = sin( theta );

        return Array{ cosine, -sine, 0, sine, cosine, 0, 0, 0, 1 };
    }

};



#pragma endregion D3



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

    SURFACE_EVENT__DESTROY = 69100,
    SURFACE_EVENT__CURSOR_HIDE, 
    SURFACE_EVENT__CURSOR_SHOW,
    SURFACE_EVENT__FORCE
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
        GUID                guid     = {};
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

typedef   std::function< void( Vec2, Vec2, SurfaceTrace& ) >                   OnMouse;
typedef   std::function< void( Key, KEY_STATE, SurfaceTrace& ) >               OnKey;
typedef   std::function< void( Vec2, SURFSCROLL_DIRECTION, SurfaceTrace& ) >       OnScroll;
typedef   std::function< void( std::vector< std::string >, SurfaceTrace& ) >   OnFiledrop;
typedef   std::function< void( Crd2, Crd2, SurfaceTrace& ) >                   OnMove;
typedef   std::function< void( Vec2, Vec2, SurfaceTrace& ) >                   OnResize;
typedef   std::function< void() >                                              OnDestroy;


class SurfaceEventSentry {
_ENGINE_PROTECTED:
    OnMouse                       _on_mouse             = {};
    OnKey                         _on_key               = {};
    OnScroll                      _on_scroll            = {};
    OnFiledrop                    _on_filedrop          = {};
    OnMove                        _on_move              = {};
    OnResize                      _on_resize            = {};
    OnDestroy                     _on_destroy           = {};

    std::map< GUID, OnMouse >     _sckt_mouse[ 2 ]      = {};
    std::map< GUID, OnKey >       _sckt_key[ 2 ]        = {};
    std::map< GUID, OnScroll >    _sckt_scroll[ 2 ]     = {};
    std::map< GUID, OnFiledrop >  _sckt_filedrop[ 2 ]   = {};
    std::map< GUID, OnMove >      _sckt_move[ 2 ]       = {};
    std::map< GUID, OnResize >    _sckt_resize[ 2 ]     = {};
    std::map< GUID, OnDestroy >   _sckt_destroy[ 2 ]    = {};

public:
    template< typename Master, typename ...Args >
    void invoke_sequence( SurfaceTrace& trace, Args&&... args ) {
        auto [ on, sckt ] = this->_seq_from_type< Master >();

        // Made sure _seq_from_type returns references and does not create a new socket map.
        // std::cout << ( std::is_same_v< const decltype( _sckt_mouse )&, decltype( sckt ) > ) << '\n';

        this->_invoke_sequence( on, sckt, trace, std::forward< Args >( args )... );
    }

public:
    template< typename Master, typename Socket, typename ...Args >
    void _invoke_sequence( const Master& master, const Socket& socket, SurfaceTrace& trace, Args&&... args ) {
        for( auto& [ guid, sckt ] : socket[ 0 ] )
            std::invoke( sckt, std::forward< Args >( args )..., trace );

        if( master )
            std::invoke( master, std::forward< Args >( args )..., trace );

        for( auto& [ guid, sckt ] : socket[ 1 ] )
            std::invoke( sckt, std::forward< Args >( args )..., trace );
    }

public:
    template< SURFACE_EVENT event, typename Function >
    SurfaceEventSentry& on( Function function ) {
        this->_seq_from_event< event >().first = function; 
        return *this;
    }

    template< SURFACE_EVENT event, typename Function >
    SurfaceEventSentry& plug( const GUID& guid, SURFACE_SOCKET_PLUG at, Function function ) {
        this->_seq_from_event< event >().second[ at ].insert( { guid, function } );
        return *this;
    }

    SurfaceEventSentry& unplug( const GUID& guid, std::optional< SURFACE_SOCKET_PLUG > at = {} ) {
        this->_unplug( guid, at, _sckt_mouse );
        this->_unplug( guid, at, _sckt_key );
        this->_unplug( guid, at, _sckt_scroll );
        this->_unplug( guid, at, _sckt_filedrop );
        this->_unplug( guid, at, _sckt_move );
        this->_unplug( guid, at, _sckt_resize );
        return *this;
    }

    template< SURFACE_EVENT event >
    SurfaceEventSentry& unplug( const GUID& guid, std::optional< SURFACE_SOCKET_PLUG > at = {} ) {
        this->_unplug( guid, at, this->_seq_from_event< event >().second );
        return *this;
    }

_ENGINE_PROTECTED:
    template< typename Socket >
    void _unplug( const GUID& guid, std::optional< SURFACE_SOCKET_PLUG > at, Socket& socket ) {
        if( at.has_value() )
            socket[ at.value() ].erase( guid );
        else {
            socket[ SURFACE_SOCKET_PLUG_AT_ENTRY ].erase( guid );
            socket[ SURFACE_SOCKET_PLUG_AT_EXIT ] .erase( guid );
        }
    }

_ENGINE_PROTECTED:
    template< typename Master >
    inline auto _seq_from_type() {
        if constexpr( std::is_same_v< Master, OnMouse > ) 
            return std::make_pair( std::ref( _on_mouse ), std::ref( _sckt_mouse ) );

        if constexpr( std::is_same_v< Master, OnKey > ) 
            return std::make_pair( std::ref( _on_key ), std::ref( _sckt_key ) );

        if constexpr( std::is_same_v< Master, OnScroll > ) 
            return std::make_pair( std::ref( _on_scroll ), std::ref( _sckt_scroll ) );

        if constexpr( std::is_same_v< Master, OnFiledrop > ) 
            return std::make_pair( std::ref( _on_filedrop ), std::ref( _sckt_filedrop ) );

        if constexpr( std::is_same_v< Master, OnMove > ) 
            return std::make_pair( std::ref( _on_move ), std::ref( _sckt_move ) );

        if constexpr( std::is_same_v< Master, OnResize > ) 
            return std::make_pair( std::ref( _on_resize ), std::ref( _sckt_resize ) );

        if constexpr( std::is_same_v< Master, OnDestroy > ) 
            return std::make_pair( std::ref( _on_destroy ), std::ref( _sckt_destroy ) );
    }

    template< SURFACE_EVENT event >
    inline auto _seq_from_event() {
        if constexpr( event == SURFACE_EVENT_MOUSE ) 
            return std::make_pair( std::ref( _on_mouse ), std::ref( _sckt_mouse ) );

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

class Surface : public UTH,
                public SurfaceEventSentry 
{
public:
    _ENGINE_ECHO_IDENTIFY_METHOD( "Surface" );

_ENGINE_PROTECTED:
    friend class Key;
    friend class Mouse;

public:
    Surface() = default;

    Surface(
        std::string_view title,
        Crd2             crd      = { 0, 0 },
        Vec2             size     = { 512, 512 },
        SURFACE_THREAD   launch   = SURFACE_THREAD_THROUGH,
        Echo             echo     = {}
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


        switch( launch ) {
            case SURFACE_THREAD_THROUGH: goto L_THREAD_THROUGH;

            case SURFACE_THREAD_ACROSS: goto L_THREAD_ACROSS;

            default: echo( this, ECHO_LOG_FAULT, "Invalid thread launch option." ); return;
        }


        L_THREAD_THROUGH: {
            std::invoke( _main, this, nullptr, echo );
        } return;


        L_THREAD_ACROSS: {
            std::binary_semaphore sync{ 0 };

            _thread = std::thread( _main, this, &sync, echo );

            if( _thread.joinable() ) {
                echo( this, ECHO_LOG_PENDING, "Awaiting across window creation..." );

                sync.acquire();
            } else
                echo( this, ECHO_LOG_FAULT, "Window thread launch failed." );
        } return;
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
    typedef   std::array< KEY_STATE, Key::COUNT >   Keys;

_ENGINE_PROTECTED:
    static constexpr auto   LIQUID_STYLE   = WS_OVERLAPPED | WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
    static constexpr auto   SOLID_STYLE    = WS_POPUP | WS_VISIBLE;

_ENGINE_PROTECTED:
#if defined( _ENGINE_UNIQUE_SURFACE )
    inline static Surface*   _ptr                  = nullptr;
#endif
    HWND                     _hwnd                 = NULL;
    WNDCLASSEX               _wnd_class            = {};
    std::thread              _thread               = {};
    Crd2                     _coord                = {};
    Vec2                     _size                 = {};

    SurfaceTrace             _trace                = {};

    Vec2                     _mouse                = {};
    Vec2                     _mouse_l              = {};
    Keys                     _keys                 = {};

_ENGINE_PROTECTED:
    void _main( std::binary_semaphore* sync, Echo echo = {} ) {
        if( !RegisterClassEx( &_wnd_class ) ) {
            echo( this, ECHO_LOG_FAULT, "Window class registration failed." );

            if( sync ) sync->release(); return;
        }

        _hwnd = CreateWindowEx(
            WS_EX_ACCEPTFILES,

            _wnd_class.lpszClassName, _wnd_class.lpszClassName,

            SOLID_STYLE,

            _coord.x, _coord.y, _size.x, _size.y,

            NULL, NULL,

            GetModuleHandle( NULL ),

            this
        );

        if( !_hwnd ) {
            echo( this, ECHO_LOG_FAULT, "Window creation failed." );

            if( sync ) sync->release(); return;
        }

        SetWindowText( _hwnd, _wnd_class.lpszClassName );


        echo( this, ECHO_LOG_OK, sync ? "Created across." : "Created through." );

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
        static auto key_proc = [ this ] ( KEY_STATE state, WPARAM w_param ) -> void {
            Key key = static_cast< Key >( w_param );

            _keys[ key ] = state;

            this->invoke_sequence< OnKey >( _trace, key, state );
        };

        _trace.reset();

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
                this->invoke_sequence< OnMouse >( _trace, _mouse, _mouse );
            break; }


            case WM_MOUSEMOVE: {
                Vec2 new_mouse = this->pull_vec( { LOWORD( l_param ), HIWORD( l_param ) } );

                this->invoke_sequence< OnMouse >( _trace, new_mouse, _mouse_l = std::exchange( _mouse, new_mouse ) );

            break; }

            case WM_MOUSEWHEEL: {
                this->invoke_sequence< OnScroll >(
                    _trace,
                    _mouse,
                    GET_WHEEL_DELTA_WPARAM( w_param ) < 0
                    ?
                    SURFSCROLL_DIRECTION_DOWN : SURFSCROLL_DIRECTION_UP
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

                this->invoke_sequence< OnFiledrop >( _trace, std::move( files ) );

            break; }


            case WM_MOVE: {
                Crd2 new_coord = { LOWORD( l_param ), HIWORD( l_param ) };

                this->invoke_sequence< OnMove >( _trace, new_coord, std::exchange( _coord, new_coord ) );

            break; }

            case WM_SIZE: {
                Vec2 new_size = { LOWORD( l_param ), HIWORD( l_param ) };

                this->invoke_sequence< OnResize >( _trace, new_size, std::exchange( _size, new_size ) );

            break; }

        }

        return DefWindowProc( hwnd, event, w_param, l_param );
    }

public:
    Vec2 pull_vec( const Crd2& crd ) const {
        return { crd.x - _size.x / 2.0, _size.y / 2.0 - crd.y };
    }

    Crd2 pull_crd( const Vec2& vec ) const {
        return { vec.x + _size.x / 2.0f, _size.y / 2.0f - vec.y };
    }

    void push_vec( Crd2& crd ) const {
        crd.x -= _size.x / 2.0;
        crd.y = _size.y / 2.0 - crd.y;
    }

    void push_crd( Vec2& vec ) const {
        vec.x += _size.x / 2.0;
        vec.y = _size.y / 2.0 - vec.y;
    }

public:
    template< typename T >
    requires( std::is_base_of_v< Vec2, T > )
    T& scale( T& vec ) {
        vec.x *= _size.x;
        vec.y *= _size.y;

        return vec;
    }

    template< typename T >
    requires( std::is_base_of_v< Vec2, T > )
    T scaled( const T& vec ) {
        T res{ vec };

        return scale( res );
    }

public:
    Crd2 coord() const {
        return _coord;
    }

    Crd2 os_crd() const {
        RECT rect = {};

        GetWindowRect( _hwnd, &rect );

        return { rect.left, rect.top };
    }

    GFTYPE x() const {
        return _coord.x;
    }

    GFTYPE y() const {
        return _coord.y;
    }

    Vec2 size() const {
        return _size;
    }

    Vec2 os_size() const {
        RECT rect = {};

        GetWindowRect( _hwnd, &rect );

        return { rect.right - rect.left, rect.bottom - rect.top };
    }

    GFTYPE width() const {
        return _size.x;
    }

    GFTYPE height() const {
        return _size.y;
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

    Surface& move_to( Vec2 crd ) {
        _coord = crd;

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

    Crd2 crd() const {
        return this->pull_crd( vec() );
    }

    Crd2 l_crd() const {
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

    static Crd2 crd() {
        return Surface::get()->pull_crd( vec() );
    }
#endif

public:
    static Vec2 g_vec() {
        auto [ x, y ] = g_crd();

        return { 
            x - Env::width_2(), 
            -y + Env::height_2()
        };
    }

    static Crd2 g_crd() {
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



class RenderWrap2 : public UTH {
public:
    RenderWrap2() = default;

    RenderWrap2( const RenderWrap2& other ) 
    : _render_wrap{ &other },
      _renderer{ other._renderer }
    {}

_ENGINE_PROTECTED:
    RenderWrap2( const Renderer2& renderer )
    : _renderer{ &renderer }
    {}

_ENGINE_PROTECTED:
    RenderWrap2*   _render_wrap    = nullptr;
    Renderer2*     _renderer       = nullptr;

public:
    virtual RenderWrap2& fill( const Chroma& ) = 0;

    virtual RenderWrap2& fill( const Brush2& ) = 0;

    virtual RenderWrap2& line( Crd2, Crd2, const Brush2& ) = 0;

    virtual RenderWrap2& line( Vec2, Vec2, const Brush2& ) = 0;

public:
    virtual Crd2 coord() const = 0;

    virtual Vec2 origin() const = 0;

    virtual Vec2 size() const = 0;

public:
    Vec2 pull_vec( const Crd2& crd ) {
        return { 2.0 * crd.x - 1.0, 2.0 * ( 1.0 - crd.y ) - 1.0 };
    }

    Crd2 pull_crd( const Vec2& vec ) {
        return { ( vec.x + 1.0 ) / 2.0, 1.0 - ( vec.y + 1.0 ) / 2.0 };
    }

    void push_vec( Crd2& crd ) {
        crd = this->pull_vec( crd );
    }

    void push_crd( Vec2& vec ) {
        vec = this->pull_crd( vec );
    }

public:
    RenderWrap2& render_wrap() {
        return *_render_wrap;
    }

    Renderer2& renderer() {
        return *_renderer;
    }

    Surface& surface();

};



class Renderer2 : public RenderWrap2 {
public:
    _ENGINE_ECHO_IDENTIFY_METHOD( "Renderer2" );

public:
    Renderer2() = default;

    Renderer2( Surface& surface, Echo echo = {} )
    : RenderWrap2{ *this }, _surface{ &surface }
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

            "<constructor>: CoCreateInstance()"
        );

        
        ECHO_ASSERT_AND_THROW(
            D2D1CreateFactory(
                D2D1_FACTORY_TYPE_MULTI_THREADED,
                &_factory
            ) == S_OK,

            "<constructor>: D2D1CreateFactory()"
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

            "<constructor>: _factory->CreateHwndRenderTarget()"
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

_ENGINE_PROTECTED:
    Surface*                    _surface       = nullptr;

    ID2D1Factory*               _factory       = nullptr;
    IWICImagingFactory*         _wic_factory   = nullptr;

    ID2D1HwndRenderTarget*      _target        = nullptr;

public:
    Surface& surface() {
        return *_surface;
    }

public:
    auto factory()     { return _factory; }
    auto target()      { return _target; }
    auto wic_factory() { return _wic_factory; }

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
    Crd2 coord()  const override { return { 0, 0 }; }
    Vec2 origin() const override { return { 0, 0 }; }
    Vec2 size()   const override { return _surface->size(); }

public:
    RenderWrap2& fill( const Chroma& chroma ) override;

    RenderWrap2& fill( const Brush2& brush ) override;

public:
    RenderWrap2& line(
        Crd2 c1, Crd2 c2,
        const Brush2& brush
    ) override;

    RenderWrap2& line(
        Vec2 v1, Vec2 v2,
        const Brush2& brush
    ) override;

public:
    template< typename Type, typename ...Args >
    Renderer2& operator () ( const Type& thing, Args&&... args ) {
        thing.render( *this, std::forward< Args >( args )... );

        return *this;
    }

public:
    static std::optional< std::pair< Vec2, Vec2 > > clip_CohenSutherland( const Vec2& tl, const Vec2& br, Vec2 p1, Vec2 p2 ) {
        auto& u = tl.y; auto& l = tl.x;
        auto& d = br.y; auto& r = br.x;

        /* UDRL */
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
    
};



class Viewport2 : public RenderWrap2,
                  public SurfaceEventSentry
{
public:
    _ENGINE_ECHO_IDENTIFY_METHOD( "Viewport2" );

public:
    Viewport2() = default;

    Viewport2(
        RenderWrap2&   render_wrap,
        Vec2           org,
        Vec2           sz,
        Echo           echo          = {}
    )
    : RenderWrap2{ render_wrap },
      _origin{ org }, _size{ sz }, _size2{ sz.x / 2.0f, sz.y / 2.0f }
    {
        echo( this, ECHO_LOG_OK, "Created." );
    }

    Viewport2(
        RenderWrap2&   render_wrap,
        Crd2           crd,
        Vec2           sz,
        Echo           echo          = {}
    )
    : Viewport2{ render_wrap, render_wrap.pull_vec( crd ) + Vec2{ sz.x / 2.0, -sz.y / 2.0 }, sz, echo }
    {}


    Viewport2( const Viewport2& ) = delete;
    Viewport2( Viewport2&& ) = delete;

_ENGINE_PROTECTED:
    Vec2   _origin       = {};
    Vec2   _size         = {};
    Vec2   _size2        = {};

    bool   _restricted   = false;

public:
    Crd2 coord()  const override { return _render_wrap->pull_crd( _origin ); }
    Vec2 origin() const override { return _origin; }
    Vec2 size()   const override { return _size; }

public:
    Viewport2& origin_to( Vec2 vec ) {
        _origin = vec;

        return *this;
    }

    Viewport2& coord_to( Crd2 crd ) {
        return this->origin_to( _render_wrap->pull_vec( crd ) );
    }

public:
    Vec2 top_left_g() const {
        return _origin + Vec2{ -_size2.x, _size2.y };
    }

    Vec2 bot_right_g() const {
        return _origin + Vec2{ _size2.x, -_size2.y };
    }

public:
    GFTYPE east_g() const {
        return _origin.x + _size2.x;
    }

    GFTYPE west_g() const {
        return _origin.x - _size2.x;
    }

    GFTYPE north_g() const {
        return _origin.y + _size2.y;
    }

    GFTYPE south_g() const {
        return _origin.y - _size2.y;
    }

    GFTYPE east() const {
        return _size2.x;
    }

    GFTYPE west() const {
        return -_size2.x;
    }

    GFTYPE north() const {
        return _size2.y;
    }

    GFTYPE south() const {
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

        auto tl = _render_wrap->pull_crd( this->top_left_g() );
        auto br = _render_wrap->pull_crd( this->bot_right_g() );

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

        .plug< SURFACE_EVENT_MOUSE >( 
            this->guid(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, Vec2 lvec, auto& trace ) -> void {
                if( !this->contains_g( vec ) ) return;

                this->invoke_sequence< OnMouse >( trace, vec - _origin, lvec - _origin );
            }
        )

        .plug< SURFACE_EVENT_SCROLL >( 
            this->guid(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, SURFSCROLL_DIRECTION dir, auto& trace ) -> void {
                if( !this->contains_g( vec ) ) return;

                this->invoke_sequence< OnScroll >( trace, vec - _origin, dir );
            }
        )

        .plug< SURFACE_EVENT_KEY >( 
            this->guid(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Key key, KEY_STATE state, auto& trace ) -> void {
                this->invoke_sequence< OnKey >( trace, key, state );
            }
        );

        return *this;
    }

    Viewport2& downlink() {
        Surface& srf = _renderer->surface();

        srf.unplug( this->guid() );

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



#pragma endregion Renderer



#pragma region Chroma



class Chroma {
public:
    Chroma() = default;

    Chroma( float r, float g, float b, float a = 1.0 )
    : r{ r }, g{ g }, b{ b }, a{ a }
    {}

    template< typename T >
    requires( std::is_integral_v< T > )
    Chroma( T r, T g, T b, T a )
    : r{ r / 255.0f }, g{ g / 255.0f }, b{ b / 255.0f }, a{ a / 255.0f }
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
    _ENGINE_ECHO_IDENTIFY_METHOD( "Brush2" );

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
    _ENGINE_ECHO_IDENTIFY_METHOD( "SolidBrush2" );

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



class LinearBrush2 : public Brush2 {
public:
    _ENGINE_ECHO_IDENTIFY_METHOD( "LinearBrush2" );

public:
    LinearBrush2() = default;

    template< typename Itr >
    LinearBrush2(
        Renderer2&    renderer,
        Vec2          launch,
        Vec2          land,
        Itr           begin,
        Itr           end,
        float         w        = 1.0,
        Echo          echo     = {}
    )
    : Brush2{ w }
    {
        _renderer = &renderer;


        D2D1_GRADIENT_STOP entries[ std::abs( std::distance( begin, end ) ) ];

        std::size_t idx = 0;

        for( Itr itr = begin; itr != end; ++itr, ++idx ) {
            entries[ idx ].color    = itr->first;
            entries[ idx ].position = itr->second;
        };

        ECHO_ASSERT_AND_THROW(
            _renderer->target()->CreateGradientStopCollection(
                entries,
                idx,
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &_grads
            ) == S_OK,

            "<constructor>: _renderer->target()->CreateGradientStopCollection"
        );

        ECHO_ASSERT_AND_THROW(
            _renderer->target()->CreateLinearGradientBrush(
                D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES{
                    _renderer->pull_crd( launch ),
                    _renderer->pull_crd( land )
                },
                D2D1_BRUSH_PROPERTIES {
                    1.0,
                    D2D1::Matrix3x2F::Identity()
                },
                _grads,
                &_brush
            ) == S_OK,

            "<constructor>: _renderer->target()->CreateLinearGradientBrush"
        );

        echo( this, ECHO_LOG_OK, "Created." );
    }

    template< typename Cntr >
    LinearBrush2(
        Renderer2&    renderer,
        Vec2          launch,
        Vec2          land,
        const Cntr&   container,
        float         w           = 1.0,
        Echo          echo        = {}
    )
    : LinearBrush2{ renderer, launch, land, container.begin(), container.end(), w }
    {}

public:
    virtual ~LinearBrush2() {
        if( _brush ) _brush->Release();

        if( _grads ) _grads->Release();
    }

private:
    Renderer2*                     _renderer   = nullptr;

    ID2D1LinearGradientBrush*      _brush      = nullptr;
    ID2D1GradientStopCollection*   _grads      = nullptr;

public:
    virtual ID2D1Brush* brush() const override {
        return _brush;
    }

public:
    Vec2 launch() const {
        auto [ x, y ] = _brush->GetStartPoint();
        return _renderer->pull_vec( Crd2{ x, y } );
    }

    Vec2 land() const {
        auto [ x, y ] = _brush->GetEndPoint();
        return _renderer->pull_vec( Crd2{ x, y } );
    }

public:
    LinearBrush2& launch_to( Vec2 vec ) {
        _brush->SetStartPoint( _renderer->pull_crd( vec ) );

        return *this;
    }

    LinearBrush2& land_to( Vec2 vec ) {
        _brush->SetEndPoint( _renderer->pull_crd( vec ) );

        return *this;
    }

};



class RadialBrush2 : public Brush2 {
public:
    _ENGINE_ECHO_IDENTIFY_METHOD( "RadialBrush2" );

public:
    RadialBrush2() = default;

    template< typename Itr >
    RadialBrush2(
        Renderer2&    renderer,
        Vec2          cen,
        Vec2          offs,
        Vec2          rad,
        Itr           begin,
        Itr           end,
        float         w        = 1.0,
        Echo          echo     = {}
    )
    : Brush2{ w }
    {
        _renderer = &renderer;


        D2D1_GRADIENT_STOP entries[ std::abs( std::distance( begin, end ) ) ];

        std::size_t idx = 0;

        for( Itr itr = begin; itr != end; ++itr, ++idx ) {
            entries[ idx ].color    = itr->first;
            entries[ idx ].position = itr->second;
        };

        ECHO_ASSERT_AND_THROW(
            _renderer->target()->CreateGradientStopCollection(
                entries,
                idx,
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &_grads
            ) == S_OK,

            "<constructor>: _renderer->target()->CreateGradientStopCollection"
        );

        ECHO_ASSERT_AND_THROW(
            _renderer->target()->CreateRadialGradientBrush(
                D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES{
                    _renderer->pull_crd( cen ),
                    Crd2{ offs.x, -offs.y },
                    rad.x, rad.y
                },
                _grads,
                &_brush
            ) == S_OK,

            "<constructor>: _renderer->target()->CreateRadialGradientBrush"
        )

        echo( this, ECHO_LOG_OK, "Created." );
    }

    template< typename Cntr >
    RadialBrush2(
        Renderer2&   renderer,
        Vec2         cen,
        Vec2         offs,
        Vec2         rad,
        const Cntr&  container,
        float        w           = 1.0,
        Echo         echo        = {}
    )
    : RadialBrush2{
          renderer,
          cen, offs, rad,
          container.begin(), container.end(),
          w,
          echo
      }
    {}

public:
    ~RadialBrush2() {
        if( _brush ) _brush->Release();

        if( _grads ) _grads->Release();
    }

private:
    Renderer2*                     _renderer   = nullptr;

    ID2D1RadialGradientBrush*      _brush      = nullptr;
    ID2D1GradientStopCollection*   _grads      = nullptr;


public:
    Vec2 center() const {
        auto [ x, y ] = _brush->GetCenter();
        return _renderer->pull_vec( Crd2{ x, y } );
    }

    Vec2 offset() const {
        auto [ x, y ] = _brush->GetGradientOriginOffset();
        return { x, -y };
    }

    float radX() const {
        return _brush->GetRadiusX();
    }

    float radY() const {
        return _brush->GetRadiusY();
    }

    Vec2 rad() const {
        return { this->radX(), this->radY() };
    }

public:
    RadialBrush2& center_to( Vec2 c ) {
        _brush->SetCenter( _renderer->pull_crd( c ) );

        return *this;
    }

    RadialBrush2& offset_to( Vec2 offs ) {
        _brush->SetGradientOriginOffset( Crd2{ 
            static_cast< float >( offs.x ), static_cast< float >( -offs.y ) 
        } );

        return *this;
    }

    RadialBrush2& radX_to( float rx ) {
        _brush->SetRadiusX( rx );

        return *this;
    }

    RadialBrush2& radY_to( float ry ) {
        _brush->SetRadiusY( ry );

        return *this;
    }

    RadialBrush2& rad_to( Vec2 vec ) {
        this->radX_to( vec.x );
        this->radY_to( vec.y );

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

_ENGINE_PROTECTED:
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


_ENGINE_PROTECTED:
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
_ENGINE_PROTECTED:
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

    DerivedWave& tweak_volume( const auto& op ) {
        _volume = std::clamp( std::invoke( op, _volume ), -1.0, 1.0 );

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

    DerivedWave& tweak_velocity( const auto& op ) {
        _velocity = std::invoke( op, _velocity );

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
    _ENGINE_ECHO_IDENTIFY_METHOD( "Audio" );

_ENGINE_PROTECTED:
    friend class Wave;

public:
    Audio() = default;

    Audio(
        std::string_view   device,
        size_t             sample_rate          = 48'000,
        size_t             channel_count        = 1,
        size_t             block_count          = 16,
        size_t             block_sample_count   = 256,
        _ENGINE_ECHO_DFD_ARG
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

_ENGINE_PROTECTED:
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

_ENGINE_PROTECTED:


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
            if( 
                size_t fbc_compare = 0;
                _free_block_count.compare_exchange_strong( fbc_compare, 0, std::memory_order_relaxed, std::memory_order_relaxed ) 
            ) {
                std::unique_lock< std::mutex > lock{ _mtx };
                _cnd_var.wait( lock );
            }

            _free_block_count.fetch_sub( 1, std::memory_order_relaxed );

           
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

_ENGINE_PROTECTED:
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
    _ENGINE_ECHO_IDENTIFY_METHOD( "Sound" );

public:
    Sound() = default;

    Sound( 
        Audio&             audio, 
        std::string_view   path, 
        _ENGINE_ECHO_DFD_ARG 
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
        _ENGINE_ECHO_DFD_ARG
    ) {
        using namespace std::string_literals;


        if( path.ends_with( ".wav" ) ) {
            auto wav = Codec::Wav::from_file( path, echo );

            _stream         = std::move( wav.stream );
            _sample_rate    = wav.sample_rate;
            _sample_count   = wav.sample_count;
            _channel_count  = wav.channel_count;

            return;
        }

        echo( this, ECHO_LOG_FAULT, "Unsupported extension: "s + path.data() );
    }

    Sound( const Sound& ) = default;

    Sound( Sound&& ) = delete;


    ~Sound() {
        stop();
    }

_ENGINE_PROTECTED:
    Shared< double[] >    _stream          = nullptr;

    std::list< double >   _needles         = {};

    size_t                _sample_rate     = 0;
    size_t                _sample_count    = 0;
    size_t                _channel_count   = 0;

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

_ENGINE_PROTECTED:
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
    _ENGINE_ECHO_IDENTIFY_METHOD( "Synth" );

public:
    typedef   std::function< double( double, size_t ) >   Function;

public:
    Synth() = default;

    Synth( 
        Audio&     audio,
        Function   function,
        _ENGINE_ECHO_DFD_ARG
    )
    : Synth{ function, echo }
    {
        _audio = &audio;

        _decay_step = 1.0 / _audio->sample_rate();
    }

    Synth(
        Function   function,
        _ENGINE_ECHO_DFD_ARG
    )
    : _function( function )
    {
        echo( this, ECHO_LOG_OK, "Created from source function." );
    }


    Synth( const Synth& ) = default;

    Synth( Synth&& ) = delete;

_ENGINE_PROTECTED:
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


_ENGINE_PROTECTED:
    virtual double _sample( size_t channel, bool advance ) override {
        if( advance )
            _elapsed += _audio->time_step() * _velocity;

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

    if( !this->is_playing() )
        _audio->_waves.push_back( this );
}



#pragma endregion Audio



#pragma region After



#pragma region Echo



const Echo& Echo::_echo(
    EchoInvoker*       invoker,
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
        << invoker->echo_name()
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



Surface& RenderWrap2::surface() {
    return _renderer->surface();
}



RenderWrap2& Renderer2::fill( const Chroma& chroma = {} ) {
    _target->Clear( chroma );

    return *this;
}

RenderWrap2& Renderer2::fill( const Brush2& brush ) {
    _target->FillRectangle(
        D2D1_RECT_F{ 0, 0, _surface->width(), _surface->height() },
        brush
    );

    return *this;
}

RenderWrap2& Renderer2::line(
    Crd2 c1, Crd2 c2,
    const Brush2& brush
) {
    _target->DrawLine(
        _surface->scaled( c1 ), _surface->scaled( c2 ),
        brush.brush(),
        brush.width()
    );

    return *this;
}

RenderWrap2& Renderer2::line(
    Vec2 v1, Vec2 v2,
    const Brush2& brush
) {
    return this->line(
        this->pull_crd( v1 ),
        this->pull_crd( v2 ),
        brush
    );
}



Viewport2& Viewport2::fill(
    const Chroma& chroma
) { 
    this->restrict();
    _renderer->fill( chroma ); /* Poor man's fill */
    this->lift_restriction();

    return *this;
}

Viewport2& Viewport2::fill(
    const Brush2& brush
) { 
    auto tl = _renderer->pull_crd( this->top_left_g() );
    auto br = _renderer->pull_crd( this->bot_right_g() );

    _renderer->target()->FillRectangle(
        D2D1_RECT_F{ tl.x, tl.y, br.x, br.y },
        brush
    );

    return *this;
}

Viewport2& Viewport2::line(
    Crd2 c1, Crd2 c2,
    const Brush2& brush
) {
    return this->line(
        _render_wrap->pull_vec( c1 ),
        _render_wrap->pull_vec( c2 ),
        brush
    );
}

Viewport2& Viewport2::line(
    Vec2 v1, Vec2 v2,
    const Brush2& brush
) {
    _render_wrap->line(
        _render_wrap->pull_crd( v1 + _origin ),
        _render_wrap->pull_crd( v2 + _origin ),
        brush
    );

    return *this;
}



#pragma endregion Renderer



#pragma region Space



void Clust2::render( Renderer2& renderer, const Brush2& brush ) {
    for( size_t idx = 0; idx < this->vrtx_count() - 1; ++idx )
        renderer.line( this->operator()( idx ), this->operator()( idx + 1 ), brush );

    renderer.line( this->operator()( this->vrtx_count() - 1 ), this->operator()( 0 ), brush );
}



#pragma endregion Space



#pragma endregion After



#pragma region Builds



class Sensor : public UTH,
               public Clust2 
{
public:
    _ENGINE_ECHO_IDENTIFY_METHOD( "Sensor" );

public:
    Sensor() = default;

    Sensor(
        Clust2   clst
    )
    : Clust2{ std::move( clst ) }
    {

    }

_ENGINE_PROTECTED:


};



enum SYSTEM2_LINK {
    SYSTEM2_LINK_FUNCTION = 0,
    SYSTEM2_LINK_COLLECTION = 1
};

class System2Node {
_ENGINE_PROTECTED:
    inline static bool   _uplink_on_construction   = true;

public:
    static void uplink_on_construction_to( bool flag ) {
        _uplink_on_construction = flag;
    }

public:
    System2Node() = default;

    template< typename Source >
    System2Node( Source&& source )
    : _source{ std::forward< Source >( source ) },
      _uplinked{ _uplink_on_construction }
    {}

public:
    typedef   std::function< double( double ) >   Function;
    typedef   std::vector< Vec2 >                 Collection;

_ENGINE_PROTECTED:
    std::variant< Function, Collection >   _source     = {};

    bool                                   _uplinked   = false;

    Shared< Brush2 >                       _brush      = nullptr;

public:
    auto type() const {
        return _source.index();
    }

    bool is_function() const {
        return this->type() == SYSTEM2_LINK_FUNCTION;
    }

    bool is_collection() const {
        return this->type() == SYSTEM2_LINK_COLLECTION;
    }

public:
    System2Node& uplink() {
        _uplinked = true;

        return *this;
    }

    System2Node& downlink() {
        _uplinked = false;

        return *this;
    }

public:
    bool is_uplinked() const {
        return _uplinked;
    }

public:
    Function& function() {
        return std::get< 0 >( _source );
    } 

    Collection& collection() {
        return std::get< 1 >( _source );
    }

public:
    System2Node& give_brush( decltype( _brush ) ptr ) {
        _brush = std::move( ptr );

        return *this;
    }

    System2Node& remove_brush() {
        _brush.reset();

        return *this;
    }

public:
    Brush2& brush() const {
        return *_brush;
    }

public:
    bool has_brush() const {
        return _brush.operator bool();
    }

};

class System2Packet {
public:
    class Div {
    public:
        Vec2     px       = { 80.0, 80.0 };
        Vec2     px_max   = { 300.0, 300.0 };
        Vec2     px_min   = { 40.0, 40.0 };
        double   hstk     = 10.0;
        Vec2     mean     = { 1.0, 1.0 };

    public:
        Vec2 coeffs() const {
            return px / mean;
        }

    public:
        Div& px_to( double val ) {
            return this->px_to( Vec2{ val } );
        }

        Div& px_to( Vec2 val ) {
            this->_calibrate( val );

            return *this;
        }

        Div& pxX_to( double val ) {
            this->_calibrate_x( val ); return *this;
        }

        Div& pxY_to( double val ) {
            this->_calibrate_y( val ); return *this;
        }

        Div& tweak_px( const auto& opX, double valX, const auto& opY, double valY ) {
            return this->tweak_pxX( opX, valX ), this->tweak_pxY( opY, valY );
        }

        Div& tweak_px( const auto& op, double val ) {
            return tweak_pxX( op, val ), tweak_pxY( op, val );
        }

        Div& tweak_pxX( const auto& op, double val ) {
            double next = px.x;

            next = std::clamp( 
                std::invoke( op, next, val ), 
                1.0, std::numeric_limits< decltype( next ) >::max() 
            );

            this->_calibrate_x( next );

            return *this;
        }

        Div& tweak_pxY( const auto& op, double val ) {
            double next = px.y;

            next = std::clamp( 
                std::invoke( op, next, val ), 
                1.0, std::numeric_limits< decltype( next ) >::max() 
            );

            this->_calibrate_y( next );

            return *this;
        }

        Div& mean_to( double val ) {
            mean = val; return *this;
        }

        Div& mean_to( Vec2 val ) {
            mean = val; return *this;
        }

        Div& meanX_to( double val ) {
            mean.x = val; return *this;
        }

        Div& meanY_to( double val ) {
            mean.y = val; return *this;
        }

        Div& tweak_mean( const auto& opX, double valX, const auto& opY, double valY ) {
            return tweak_meanX( opX, valX ), tweak_meanY( opY, valY );
        }

        Div& tweak_mean( const auto& op, double val ) {
            return tweak_meanX( op, val ), tweak_meanY( op, val );
        }

        Div& tweak_meanX( const auto& op, double val ) {
            mean.x = std::clamp( 
                std::invoke( op, mean.x, val ), 
                std::numeric_limits< decltype( mean.x ) >::min(), 
                std::numeric_limits< decltype( mean.x ) >::max() 
            );
            return *this;
        }

        Div& tweak_meanY( const auto& op, double val ) {
            mean.y = std::clamp( 
                std::invoke( op, mean.y, val ), 
                std::numeric_limits< decltype( mean.y ) >::min(), 
                std::numeric_limits< decltype( mean.y ) >::max() 
            );
            return *this;
        }
    
    _ENGINE_PROTECTED:
        bool _calibrate_x( double next ) {
            double reset = ( ( px_max - px_min ) / 2.0 ).x;

            if( next < px_min.x ) {
                mean.x *= reset / px.x;
                px.x = reset;

                goto L_RESET;
            }

            if( next > px_max.x ) {
                mean.x /= px.x / reset;
                px.x = reset;

                goto L_RESET;
            }

            px.x = next;

            return false;

            L_RESET:

            return true;
        }

        bool _calibrate_y( double next ) {
            double reset = ( ( px_max - px_min ) / 2.0 ).y;

            if( next < px_min.y ) {
                mean.y *= reset / px.y;
                px.y = reset;

                goto L_RESET;
            }

            if( next > px_max.y ) {
                mean.y /= px.y / reset;
                px.y = reset;

                goto L_RESET;
            }

            px.y = next;

            return false;

            L_RESET:

            return true;
        }

        void _calibrate( Vec2 val ) {
            _calibrate_x( val.x ); _calibrate_y( val.y );
        }

    } div = {};

    class Brushes {
    public:
        std::vector< Shared< Brush2 > >   nodes   = {};
        mutable size_t                    at      = 0;

        Shared< Brush2 >                  bgnd    = {};
        Shared< Brush2 >                  axis    = {};

    public:
        Brush2& next_node() {
            Brush2& brush = nodes[ at++ ];
            at %= nodes.size();

            return brush;
        }
        
        void prepare_render() {
            at = 0;
        }
    
    } brushes = {};

};

class System2 : public UTH,
                public std::map< std::string, System2Node >,
                public System2Packet
{
public:
    _ENGINE_ECHO_IDENTIFY_METHOD( "System2" );

public:
    System2() = default;

    System2(
        Viewport2&      vwprt,
        System2Packet   pckt,
        Echo            echo          = {}
    )
    : System2Packet{ std::move( pckt ) }
    {
        _viewport = &vwprt;

        if( brushes.nodes.empty() )
            brushes.nodes.emplace_back( brushes.axis );

        echo( this, ECHO_LOG_OK, "Created." );
    }

private:
    Viewport2*   _viewport     = nullptr;
    Vec2         _origin       = {};

public:
    Viewport2& viewport() {
        return *_viewport;
    }

    Surface& surface() {
        return _viewport->surface();
    }

public:
    System2& offset_to( Vec2 vec ) {
        _origin = vec;

        return *this;
    }

    Vec2 origin() const {
        return _origin;
    }

public:
    System2& uplink_auto_roam() {
        _viewport->plug< SURFACE_EVENT_MOUSE >( 
            this->guid(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, Vec2 lvec, auto& trace ) -> void {
                if( !_viewport->surface().down( Key::LMB ) ) return; 

                _origin += ( vec - lvec );
            }
        );

        _viewport->plug< SURFACE_EVENT_SCROLL >( 
            this->guid(), SURFACE_SOCKET_PLUG_AT_ENTRY, 
            [ this ] ( Vec2 vec, SURFSCROLL_DIRECTION dir, auto& trace ) -> void {
                auto& s = this->surface();

                double sgn   = dir == SURFSCROLL_DIRECTION_UP ? 1.0 : -1.0;
                double tweak = 4.6 * sgn;

                switch( ( s.down( Key::CTRL ) << 1 ) | s.down( Key::SHIFT ) ) {
                    case 0: 
                    case 3:
                        this->div.tweak_px( std::plus<>{}, tweak ); 
                        _origin -= vec( _origin ) / div.px * tweak;
                    break;

                    case 1:
                        this->div.tweak_pxY( std::plus<>{}, tweak ); 
                    break;

                    case 2:
                        this->div.tweak_pxX( std::plus<>{}, tweak ); 
                    break;
                } 
            }
        );

        return *this;
    }

    System2& downlink_auto_roam() {
        _viewport->unplug( this->guid() );

        return *this;
    }

public:
    void render( Renderer2& renderer ) const {
        _viewport->restrict().fill( brushes.bgnd );


        double    cst        = 0.0;
        double    cst_stk    = 0.0;
        bool      axis_out[] = { false, false, false, false };


        if( ( axis_out[ HEADING_EAST ] = ( _origin.x > _viewport->east() ) )
            || 
            ( axis_out[ HEADING_WEST ] = ( _origin.x < _viewport->west() ) )
        )
            goto L_SKIP_Y_AXIS;


        cst = _viewport->origin().x + _origin.x;

        renderer.line(
            Vec2{ cst, _viewport->north_g() }, 
            Vec2{ cst, _viewport->south_g() },
            brushes.axis
        );


        L_SKIP_Y_AXIS:


        if( ( axis_out[ HEADING_NORTH ] = ( _origin.y > _viewport->north() ) )
            || 
            ( axis_out[ HEADING_SOUTH ] = ( _origin.y < _viewport->south() ) )
        )
            goto L_SKIP_X_AXIS;

        cst = _viewport->origin().y + _origin.y;

        renderer.line(
            Vec2{ _viewport->west_g(), cst }, 
            Vec2{ _viewport->east_g(), cst },
            brushes.axis
        );

        
        L_SKIP_X_AXIS:


        cst = _viewport->origin().x + _origin.x;

        if( cst < _viewport->west_g() ) {
            cst = _viewport->west_g() - cst;
            
            double tweak = cst / div.px.x;

            cst = _viewport->west_g() + div.px.x * ( 1.0 - ( tweak - static_cast< int64_t >( tweak ) ) );

        } else if( cst > _viewport->east_g() ) {
            cst = cst - _viewport->east_g();
            
            double tweak = cst / div.px.x;

            cst = _viewport->east_g() - div.px.x * ( 1.0 - ( tweak - static_cast< int64_t >( tweak ) ) );
        }

        cst_stk = _viewport->origin().y + _origin.y;

        auto strike_x_at = [ this, &renderer, &cst_stk, &axis_out ] ( double x ) -> void {
            renderer.line(
                Vec2{ 
                    x, 
                    axis_out[ HEADING_SOUTH ] ?
                        std::min( _viewport->south_g() + div.hstk, _viewport->north_g() )
                        :
                        std::min( _viewport->north_g(), cst_stk + div.hstk )
                },
                Vec2{ 
                    x, 
                    axis_out[ HEADING_NORTH ] ?
                        std::max( _viewport->north_g() - div.hstk, _viewport->south_g() )
                        :
                        std::max( _viewport->south_g(), cst_stk - div.hstk ) 
                },
                *brushes.axis
            );
        };

        for( double offs = 0.0; ( cst + offs ) <= _viewport->east_g(); offs += div.px.x ) 
            strike_x_at( cst + offs );
        
        for( double offs = 0.0; ( cst - offs ) >= _viewport->west_g(); offs += div.px.x ) 
            strike_x_at( cst - offs );


        cst = _viewport->origin().y + _origin.y;

        if( cst < _viewport->south_g() ) {
            cst = _viewport->south_g() - cst;
            
            double tweak = cst / div.px.y;

            cst = _viewport->south_g() + div.px.y * ( 1.0 - ( tweak - static_cast< int64_t >( tweak ) ) );

        } else if( cst > _viewport->north_g() ) {
            cst = cst - _viewport->north_g();
            
            double tweak = cst / div.px.y;

            cst = _viewport->north_g() - div.px.y * ( 1.0 - ( tweak - static_cast< int64_t >( tweak ) ) );
        }

        cst_stk = _viewport->origin().x + _origin.x;

        auto strike_y_at = [ this, &renderer, &cst_stk, &axis_out ] ( double y ) -> void {
            renderer.line(
                Vec2{ 
                    axis_out[ HEADING_WEST ] ?
                        std::min( _viewport->west_g() + div.hstk, _viewport->east_g() )
                        :
                        std::min( _viewport->east_g(), cst_stk + div.hstk ),
                    y
                },
                Vec2{
                    axis_out[ HEADING_EAST ] ?
                        std::max( _viewport->east_g() - div.hstk, _viewport->west_g() )
                        :
                        std::max( _viewport->west_g(), cst_stk - div.hstk ),
                    y
                },
                *brushes.axis
            );
        };

        for( double offs = 0.0; ( cst + offs ) <= _viewport->north_g(); offs += div.px.y ) 
            strike_y_at( cst + offs );

        for( double offs = 0.0; ( cst - offs ) >= _viewport->south_g(); offs += div.px.y ) 
            strike_y_at( cst - offs );


        brushes.prepare_render();

        for( auto& [ idx, node ] : *this ) {
            if( !node.is_uplinked() ) continue;

            Brush2& brush = node.has_brush() ? node.brush() : brushes.next_node();

            switch( node.type() ) {
                case SYSTEM2_LINK_FUNCTION: 
                    this->_render_function( node, brush ); 
                    continue;

                case SYSTEM2_LINK_COLLECTION: 
                    this->_render_collection( node, brush ); 
                    continue;

                default: continue;
            }
        }


        _viewport->lift_restriction();
    }

_ENGINE_PROTECTED:
    void _render_function( const System2Node& node, Brush2& brush ) const {
        System2Node::Function& func = node.function();

        Vec2   c   = div.coeffs();
        double x   = ( _viewport->west() - _origin.x ) * ( 1.0 / c.x );
        double end = ( _viewport->east() - _origin.x ) * ( 1.0 / c.x ); 
        double h   = abs( ( end - x ) / _viewport->size().x );

        Vec2 v1{ x, std::invoke( func, x ) };
        x += h;

        for( ; x <= end; x += h ) {
            Vec2 v2{ x, std::invoke( func, x ) };

            _viewport->line( v1 * c + _origin, v2 * c + _origin, brush );
            
            v1 = v2;
        }
    }

    void _render_collection( const System2Node& node, Brush2& brush ) const {
        System2Node::Collection& col = node.collection();

        if( col.empty() ) return;

        Vec2   c  = div.coeffs();
        auto   v1 = col.begin();

        for( auto v2 = v1 + 1; v2 != col.end(); ++v2 ) {
            _viewport->line( *v1 * c + _origin, *v2 * c + _origin, brush );

            v1 = v2;
        }
    }

};



#pragma endregion Builds



};
