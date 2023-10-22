#pragma region DETAILS
/*
    [ GENERAL ]
        Engine:      _9T

        Version:     n/a

        Hackerman:   Mipsan

        C++:         2023's GNU standard

        OSs:         Windows

    [ PRE-DEFINES ]
        _9T_ECHO --- logs stuff.
        _9T_UNIQUE_SURFACE --- enables quicker surface event routing.

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


#define _ENGINE_NAMESPACE _9T

#define _ENGINE_STRUCT_TYPE( type ) "_9T :: " type

#define _ENGINE_STRUCT_TYPE_MTD( type ) virtual std :: string_view struct_type() const override { return _ENGINE_STRUCT_TYPE( type ); }


#if defined( _9T_ECHO )
        #define _ENGINE_ECHO
#endif

#if defined( _9T_UNIQUE_SURFACE )
    #define _ENGINE_UNIQUE_SURFACE
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
    enum Console_color_code {
        
    };



    void console_color_to( const auto& code ) {
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), code );
    }
#endif


};
#pragma endregion OS



namespace _ENGINE_NAMESPACE {



class Base;

class Audio;
class Sound;



#pragma region Echo



#if defined( _ENGINE_ECHO )
    #define _ECHO_LITERAL( op, code ) \
        const char* operator "" op ( const char* str, [[ maybe_unused ]] size_t unused ) { \
            OS :: console_color_to( code ); \
            return str; \
        }

    _ECHO_LITERAL( _echo_normal, 15 )
    _ECHO_LITERAL( _echo_highlight, 8 )
    _ECHO_LITERAL( _echo_special, 9 )
#endif


class Echo {
public:
    enum Type {
        FAULT = 12, WARNING = 14, OK = 10, PENDING = 9, HEADSUP = 13
    };

public:
    Echo() = default;

    Echo( const Echo& other )
    : _depth{ other._depth + 1 }
    {}

    ~Echo() {
        if( _depth == 0 )
            std :: cout << '\n';
    }

private:
    size_t   _depth   = 0;

public:
    const Echo& operator () (
        Base*                invoker,
        Type                 type,
        std :: string_view   message
    ) const {
        return std :: invoke( _route, this, invoker, type, message );
    }

private:
    const Echo& _echo(
        Base*                invoker,
        Type                 type,
        std :: string_view   message
    ) const;

    const Echo& _nop(
        Base*                invoker,
        Type                 type,
        std :: string_view   message
    ) const {
        return *this;
    }

private:
    inline static auto   _route   = &_echo;

public:
    static void high() {
        _route = &_echo;
    }

    static void low() {
        _route = &_nop;
    }

private:
    static std :: string_view _type_name( Type type ) {
        switch( type ) {
            case FAULT:   return "FAULT";
            case WARNING: return "WARNING";
            case OK:      return "OK";
            case PENDING: return "PENDING";
            case HEADSUP: return "HEADSUP";
        }

        return "UNKNOWN";
    }

    static std :: string_view _type_fill( Type type ) {
        switch( type ) {
            case FAULT:   return "  ";
            case WARNING: return "";
            case OK:      return "     ";
            case PENDING: return "";
            case HEADSUP: return "";
        }

        return "";
    }

};



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
        using _Base :: _Base;

    public:
        inline static constexpr bool   is_array   = std :: is_array_v< Type >;

    public:
        typedef   std :: conditional_t< is_array, std :: decay_t< Type >, Type* >   Type_ptr;
        typedef   std :: remove_pointer_t< Type_ptr >&                              Type_ref;

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
        operator std :: enable_if_t< isnt_array, Type_ref > () {
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
    class Unique : public _Smart_ptr_extended< std :: unique_ptr, Type, Unique< Type > > {
    public:
        typedef   _Smart_ptr_extended< std :: unique_ptr, Type, Unique< Type > >   _Base;

    public:
        using _Base :: _Base;

        using _Base :: operator =;

    };


    template< typename Type >
    class Shared : public _Smart_ptr_extended< std :: shared_ptr, Type, Shared< Type > > {
    public:
        typedef   _Smart_ptr_extended< std :: shared_ptr, Type, Shared< Type > >   _Base;

    public:
        using _Base :: _Base;

        using _Base :: operator =;

    };



#pragma endregion Tricks



#pragma region Syncs



template< typename T >
requires ( std :: is_arithmetic_v< T > )
void wait_for( T duration ) {
    if constexpr( std :: is_floating_point_v< T > )
        std :: this_thread :: sleep_for( std :: chrono :: milliseconds( static_cast< int64_t >( duration * 1000.0 ) ) );
    else
        std :: this_thread :: sleep_for( std :: chrono :: milliseconds( static_cast< int64_t >( duration ) ) );
}



class Clock {
public:
    Clock()
    : _create{ std :: chrono :: high_resolution_clock :: now() },
    _last_lap{ std :: chrono :: high_resolution_clock :: now() }
    {}

public:
    inline static constexpr double M[ 4 ] = { 1000.0, 1.0, 1.0 / 60.0, 1.0 / 3600.0 };

    enum Unit {
        MILLI = 0, SEC, MIN, HOUR
    };

private:
    std :: chrono :: high_resolution_clock :: time_point   _create     = {};
    std :: chrono :: high_resolution_clock :: time_point   _last_lap   = {};

public:
    double up_time( Unit unit = SEC ) const {
        using namespace std :: chrono;

        return duration< double >( high_resolution_clock :: now() - _create ).count() * M[ unit ];
    }

    double peek_lap( Unit unit = SEC ) const {
        using namespace std :: chrono;

        return duration< double >( high_resolution_clock :: now() - _last_lap ).count() * M[ unit ];
    }

    double lap( Unit unit = SEC ){
        using namespace std :: chrono;

        auto now = high_resolution_clock :: now();

        return duration< double >( now - std :: exchange( _last_lap, now ) ).count() * M[ unit ];
    }

public:
    static auto UNIX() {
        return time( nullptr );
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
        : _value( std :: move( val ) )
    {}


    Controller( const Controller< T >& other ) = delete;

    Controller( Controller< T >&& other ) = delete;


    ~Controller() {
        release();
    }

private:
    typedef std :: tuple<
                Unique< std :: mutex >,
                Unique< std :: condition_variable >,
                std :: function< bool( const T& ) >
            > Entry;

    enum _TUPLE_ACCESS_INDEX {
        _MTX = 0, _CND = 1, _OP = 2
    };

private:
    mutable T              _value      = {};
    std :: mutex           _sync_mtx   = {};
    std :: list< Entry >   _entries    = {};

public:
    operator typename std :: enable_if_t< std :: is_copy_constructible_v< T >, T > () const {
        return _value;
    }

public:
    Controller& operate( std :: function< bool( T& ) > op ) {
        std :: unique_lock< std :: mutex > sync_lock( _sync_mtx );

        op( _value );

        for( Entry& entry : _entries )
            if( std :: get< _OP >( entry )( _value ) )
                std :: get< _CND >( entry )->notify_all();

        return *this;
    }

    Controller& operator () ( std :: function< bool( T& ) > op ) {
        return operate( op );
    }

public:
    Controller& wait_until( std :: function< bool( const T& ) > cnd ) {
        std :: unique_lock< std :: mutex > sync_lock( _sync_mtx );

        if( cnd( _value ) ) return *this;


        _entries.emplace_back(
            std :: make_unique< std :: mutex >(),
            new std :: condition_variable,
            cnd
        );

        auto entry = _entries.rbegin();

        sync_lock.unlock();


        std :: unique_lock< std :: mutex > lock( *std :: get< _MTX >( *entry ) );

        std :: get< _CND >( *entry )->wait( lock );

        lock.unlock();
        lock.release();

        _entries.erase( entry );

        return *this;
    }

public:
    Controller& release() {
        for( Entry& entry : _entries )
            std :: get< _CND >( entry )->notify_all();

        _entries.clear();
    }

};



#pragma endregion Syncs



#pragma region Utility



template< typename T >
class Coord {
    Coord() = default;

    Coord( T x, T y )
        : x( x ), y( y )
    {}

    template< typename T_other >
    Coord( const Coord< T_other >& other )
        : x( static_cast< T >( other.x ) ), y( static_cast< T >( other.y ) )
    {}


    T   x   = {};
    T   y   = {};


    template< bool is_float = std :: is_same_v< float, T > >
    operator std :: enable_if_t< is_float, const D2D1_POINT_2F& > () const {
        return *reinterpret_cast< const D2D1_POINT_2F* >( this );
    }

    template< bool is_float = std :: is_same_v< float, T > >
    operator std :: enable_if_t< is_float, D2D1_POINT_2F& > () {
        return *reinterpret_cast< D2D1_POINT_2F* >( this );
    }

};

template< typename T >
class Size {
    Size() = default;

    Size( T width, T height )
        : width( width ), height( height )
    {}

    template< typename T_other >
    Size( const Size< T_other >& other )
        : width( static_cast< T >( other.width ) ), height( static_cast< T >( other.height ) )
    {}

    T   width    = {};
    T   height   = {};

};



class File {
    public:
        static std :: string dir_of( std :: string_view path ) {
            return path.substr( 0, path.find_last_of( "/\\" ) ).data();
        }

        static std :: string name_of( std :: string_view path ) {
            return path.substr( path.find_last_of( "/\\" ) + 1, path.size() - 1 ).data();
        }

    public:
        static size_t size( std :: string_view path ) {
            std :: ifstream file( path.data(), std :: ios_base :: binary );

            return size( file );
        }

        static size_t size( std :: ifstream& file ) {
            file.seekg( 0, std :: ios_base :: end );

            size_t sz = file.tellg();

            file.seekg( 0, std :: ios_base :: beg );

            return sz;
        }

    public:
        static std :: string browse( std :: string_view title ) {
            char path[ MAX_PATH ];

            OPENFILENAME hf;

            std :: fill_n( path, sizeof( path ), 0 );
            std :: fill_n( reinterpret_cast< char* >( &hf ), sizeof( hf ), 0 );

            hf.lStructSize  = sizeof( hf );
            hf.hwndOwner    = GetFocus();
            hf.lpstrFile    = path;
            hf.nMaxFile     = MAX_PATH;
            hf.lpstrTitle   = title.data();
            hf.Flags        = OFN_EXPLORER | OFN_NOCHANGEDIR;

            GetOpenFileName( &hf );

            return path;
        }

        static std :: string save( std :: string_view title ) {
            char path[ MAX_PATH ];

            OPENFILENAME hf;

            std :: fill_n( path, sizeof( path ), 0 );
            std :: fill_n( reinterpret_cast< char* >( &hf ), sizeof( hf ), 0 );

            hf.lStructSize  = sizeof( hf );
            hf.hwndOwner    = GetFocus();
            hf.lpstrFile    = path;
            hf.nMaxFile     = MAX_PATH;
            hf.lpstrTitle   = title.data();
            hf.Flags        = OFN_EXPLORER | OFN_NOCHANGEDIR;

            GetSaveFileName( &hf );

            return path;
        }

    public:
        template< typename Itr >
        static std :: optional< ptrdiff_t > next_idx(
            std :: ifstream& file, std :: string& str,
            Itr begin, Itr end
        ) {
            if( !( file >> str ) ) return {};

            return std :: distance(
                begin,
                std :: find_if( begin, end, [ &str ] ( const decltype( *begin )& entry ) -> bool {
                    return str == entry;
                } )
            );
        }

        template< typename Itr >
        static void auto_nav(
            std :: ifstream& file,
            Itr begin, Itr end,
            std :: function< void( ptrdiff_t, std :: string& ) > func
        ) {
            std :: string str = {};

            for(
                auto idx = next_idx( file, str, begin, end );
                idx.has_value();
                idx = next_idx( file, str, begin, end )
            ) {
                func( idx.value(), str );
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
            std :: is_signed_v< T >;

        for( size_t n = byte_count; n < sizeof( T ); ++n )
            bytes[ n ] = is_negative ? -1 : 0;

        for( size_t n = 0; n < byte_count && n < sizeof( T ); ++n )
            bytes[ n ] = array[ end == LITTLE ? n : byte_count - n - 1 ];

        return *reinterpret_cast< T* >( &bytes );
    }
};



class Env {
public:
    static int W() {
        static int value = ( [] () -> int {
            RECT rect;
            GetWindowRect( GetDesktopWindow(), &rect );

            return rect.right;
        } )();

        return value;
    }

    static int H_W() {
        return W() / 2;
    }

    static int T_W() {
        return W() / 3;
    }

    static int Q_W() {
        return W() / 4;
    }

    static int H() {
        static int value = ( [] () -> int {
            RECT rect;
            GetWindowRect( GetDesktopWindow(), &rect );

            return rect.bottom;
        } )();

        return value;
    }

    static int H_H() {
        return H() / 2;
    }

    static int T_H() {
        return H() / 3;
    }

    static int Q_H() {
        return H() / 4;
    }

    static float D() {
        static float value = std :: sqrt( W() * W() + H() * H() );

        return value;
    }

    static float H_D() {
        return D() / 2.0;
    }

    static float A() {
        return static_cast< float >( W() ) / H();
    }

    static std :: string_view dir() {
        static std :: string value = ( [] () -> std :: string {
            char path[ MAX_PATH ];

            GetModuleFileNameA( GetModuleHandle( NULL ), path, MAX_PATH );

            return File :: dir_of( path );
        } )();

        return value;
    }

    static std :: string_view process() {
        static std :: string value = ( [] () -> std :: string {
            char path[ MAX_PATH ];

            GetModuleFileNameA( GetModuleHandle( NULL ), path, MAX_PATH );

            return File :: name_of( path );
        } )();

        return value;
    }
};



#pragma endregion Utility



#pragma region Base



class Base {
public:
    using Index_t = size_t;

public:
    enum Action : unsigned long long {
        TRIGGER, REPRESS, count
    };

private:
    inline static std :: unordered_map< std :: type_index, Index_t >   _register                   = {};

    inline static std :: vector< void (*)( Base& ) >                   _grids[ Action :: count ]   = {};

public:
    template< typename ...Structs >
    requires ( std :: is_base_of_v< Base, Structs > && ... )
    static void init() {
        _init<
            Audio, Sound,

            Structs...
        >();
    }

private:
    template< typename ...Structs >
    static void _init() {
        Index_t idx = 0;

        ( _register.insert( { typeid( Structs ), ++idx } ), ... );

        for( auto& grid : _grids )
            grid.assign( idx * idx, nullptr );
    }

public:
    template< typename Type >
    static Index_t idx_of() {
        return _register.at( typeid( Type ) );
    }

/*----------------------------------------- STATIC <-> OBJECT -----------------------------------------*/

protected:
    Base( Index_t idx )
    : _idx( idx )
    {}

protected:
    Index_t   _idx   = {};

public:
    auto idx() const { 
        return _idx; 
    }

public:
    virtual void trigger( Base& ) {}

    virtual void repress( Base& ) {}

public:
    virtual void render() {}

public:
    virtual std :: string_view struct_type() const {
        return _ENGINE_STRUCT_TYPE( "Base" );
    }

};



template< typename Inh >
class Is_base : public Base {
protected:
    Is_base() 
    : Base{ idx_of< Inh >() }
    {}

};



#pragma endregion Base



#pragma region Audio



class Wave {
public:
    friend class Audio;

public:
    typedef   std :: function< double( double, size_t ) >   Filter;

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

    void release() {
        _audio = nullptr;
    }

public:
    Audio& lock() const {
        return *_audio;
    }


protected:
    virtual double _sample( size_t channel ) = 0;

};


class Volumable_wave {
public:
    friend class Audio;

protected:
    double   _volume   = 1.0;

public:
    bool volume() const {
        return _volume;
    }

public:
    void volume_to( double vlm ) {
        _volume = std :: clamp( vlm, -1.0, 1.0 );
    }

    void tweak_volume( double twk ) {
        _volume = std :: clamp( _volume + twk, -1.0, 1.0 );
    }

};

class Pausable_wave {
public:
    friend class Audio;

protected:
    bool   _paused   = false;

public:
    bool is_paused() const {
        return _paused;
    }

public:
    void pause() {
        _paused = true;
    }

    void resume() {
        _paused = false;
    }

};

class Mutable_wave {
public:
    friend class Audio;

protected:
    bool   _muted   = false;

public:
    bool is_muted() const {
        return _muted;
    }

public:
    void mute() {
        _muted = true;
    }

    void unmute() {
        _muted = false;
    }

};

class Loopable_wave {
public:
    friend class Audio;

protected:
    bool   _looping   = false;

public:
    bool is_looping() const {
        return _looping;
    }

public:
    void loop() {
        _looping = true;
    }

    void unloop() {
        _looping = false;
    }

};

class Velocitable_wave {
public:
    friend class Audio;

protected:
    double   _velocity   = 1.0;

public:
    double velocity() const {
        return _velocity;
    }

public:
    void velocity_to( double vlc ) {
        _velocity = vlc;
    }

    void tweak_velocity( double twk ) {
        _velocity += twk;
    }

};

class Filtrable_wave {
public:
    friend class Audio;

protected:
    Wave :: Filter   _filter   = nullptr;

public:
    Wave :: Filter filter() const {
        return _filter;
    }

public:
    void filter_to( const Wave :: Filter& flt ) {
        _filter = flt;
    }

    void remove_filter() {
        _filter = nullptr;
    }

};



class Audio : public Is_base< Audio >,
              public Volumable_wave, 
              public Pausable_wave,
              public Mutable_wave,
              public Velocitable_wave, 
              public Filtrable_wave
{
public:
    _ENGINE_STRUCT_TYPE_MTD( "Audio" );

private:
    friend class Wave;

public:
    Audio() = default;

    Audio(
        std :: string_view device,
        size_t             sample_rate        = 48000,
        size_t             channel_count      = 1,
        size_t             block_count        = 16,
        size_t             block_sample_count = 256,
        Echo               echo               = {}
    )
    : _sample_rate       { sample_rate },
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
            echo( this, Echo :: FAULT, "Device does not exist." ); return;
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
            echo( this, Echo :: FAULT, "Wave open link failed." ); return;
        }


        _block_memory = new int[ _block_count * _block_sample_count ];

        if( !_block_memory ) {
            echo( this, Echo :: FAULT, "Block alloc failed." ); return;
        }

        std :: fill_n( _block_memory.get(), _block_count * _block_sample_count, 0 );


        _wave_headers = new WAVEHDR[ _block_count ];

        if( !_wave_headers ) {
            echo( this, Echo :: FAULT, "Wave headers alloc failed." ); return;
        }

        std :: fill_n( ( char* ) _wave_headers.get(), sizeof( WAVEHDR ) * _block_count, 0 );


        for( size_t n = 0; n < _block_count; ++n ) {
            _wave_headers[ n ].dwBufferLength = sizeof( int ) * _block_sample_count;
            _wave_headers[ n ].lpData = ( char* ) ( _block_memory + ( n * _block_sample_count ) );
        }


        _powered = true;

        _thread = std :: thread( _main, this );

        if( !_thread.joinable() ) {
            echo( this, Echo :: FAULT, "Thread launch failed." ); return;
        }

        std :: unique_lock< std :: mutex > lock{ _mtx };
        _cnd_var.notify_one();

        echo( this, Echo :: OK, "Created." );
    }


    Audio( const Audio& other ) = delete;

    Audio( Audio&& other ) = delete;


    ~Audio() {
        _powered = false;

        _cnd_var.notify_one();

        if( _thread.joinable() )
            _thread.join();

        waveOutReset( _wave_out );
        waveOutClose( _wave_out );
    }

private:
    volatile bool                _powered              = false;

    size_t                       _sample_rate          = 0;
    size_t                       _channel_count        = 0;
    size_t                       _block_count          = 0;
    size_t                       _block_sample_count   = 0;
    size_t                       _block_current        = 0;
    Unique< int[] >              _block_memory         = nullptr;

    Unique< WAVEHDR[] >          _wave_headers         = nullptr;
    HWAVEOUT                     _wave_out             = nullptr;
    std :: string                _device               = {};

    std :: thread                _thread               = {};

    std :: atomic< size_t >      _free_block_count     = 0;
    std :: condition_variable    _cnd_var              = {};
    std :: mutex                 _mtx                  = {};

    std :: list< Wave* >         _waves                = {};

private:


    void _main() {
        constexpr double max_sample = static_cast< double >(
            std :: numeric_limits< int > :: max()
        );


        auto sample = [ this ] ( size_t channel ) -> double {
            double amp = 0.0;

            if( _paused ) return amp;

            for( Wave* wave : _waves )
                amp += wave -> _sample( channel );

            return _filter ? _filter( amp, channel ) : amp;
        };


        while( _powered ) {
            if( _free_block_count == 0 ) {
                std :: unique_lock< std :: mutex > lock{ _mtx };

                _cnd_var.wait( lock );
            }

            --_free_block_count;


            if( _wave_headers[ _block_current ].dwFlags & WHDR_PREPARED )
                waveOutUnprepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );


            _waves.remove_if( [] ( Wave* wave ) {
                return wave->done();
            } );


            size_t current_block = _block_current * _block_sample_count;

            for( size_t n = 0; n < _block_sample_count; n += _channel_count )
                for( size_t ch = 0; ch < _channel_count; ++ch )
                    _block_memory[ current_block + n + ch ] = static_cast< int >( 
                        std :: clamp( sample( ch ), -1.0, 1.0 ) * max_sample 
                    );


            waveOutPrepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );
            waveOutWrite( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );


            ++_block_current;
            _block_current %= _block_count;
        }

    }

private:
    static void event_proc_router( HWAVEOUT hwo, UINT event, DWORD_PTR instance, DWORD w_param, DWORD l_param ) {
        reinterpret_cast< Audio* >( instance )->event_proc( hwo, event, w_param, l_param);
    }

    void event_proc( HWAVEOUT hwo, UINT event, DWORD w_param, DWORD l_param ) {
        switch( event ) {
            case WOM_OPEN: {

            break; }

            case WOM_DONE: {
                ++_free_block_count;

                std :: unique_lock< std :: mutex > lock{ _mtx };
                _cnd_var.notify_one();
            break; }

            case WOM_CLOSE: {
                /* Here were the uniques deleted[] */
            break; }
        }
    }

public:
    static std :: vector< std :: string > devices() {
        WAVEOUTCAPS woc;

        std :: vector< std :: string > devs;

        for( decltype( waveOutGetNumDevs() ) n = 0; n < waveOutGetNumDevs(); ++n ) {
            if( waveOutGetDevCaps( n, &woc, sizeof( WAVEOUTCAPS ) ) != S_OK ) continue;

            devs.emplace_back( woc.szPname );
        }

        return devs;
    }

    std :: string_view device() const {
        return _device;
    }

    void device_to( std :: string_view dev ) {
        
    }

public:
    size_t sample_rate() const {
        return _sample_rate;
    }

    size_t channel_count() const {
        return _channel_count;
    }

};



class Sound : public Is_base< Sound >,
              public Wave, 
              public Volumable_wave, 
              public Pausable_wave, 
              public Mutable_wave, 
              public Loopable_wave, 
              public Velocitable_wave, 
              public Filtrable_wave
{
public:
    _ENGINE_STRUCT_TYPE_MTD( "Sound" );

public:
    friend class Audio;

public:
    Sound() = default;

    Sound( 
        Audio&             audio, 
        std :: string_view path, 
        Echo               echo  = {} 
    )
    : Sound{ path }
    {
        _audio = &audio;

        if( _sample_rate != _audio->sample_rate() )
            echo( this, Echo :: WARNING, "Sample rate does not match with locked on audio's." );


        if( _channel_count != _audio->channel_count() )
            echo( this, Echo :: WARNING, "Channel count does not match with locked on audio's." );
    }

    Sound( 
        std :: string_view path,
        Echo               echo   = {}
    ) {
        using namespace std :: string_literals;


        std :: ifstream file{ path.data(), std :: ios_base :: binary };

        if( !file ) {
            echo( this, Echo :: FAULT, "Open file: "s + path.data() ); return;
        }


        size_t file_size = File :: size( file );

        Unique< char[] > file_stream{ new char[ file_size ] };

        if( !file_stream ) {
            echo( this, Echo :: FAULT, "File stream allocation." ); return;
        }


        file.read( file_stream, file_size );


        _sample_rate = Bytes :: as< unsigned int >( file_stream + 24, 4, Bytes :: LITTLE );


        _bits_per_sample = Bytes :: as< unsigned short >( file_stream + 34, 2, Bytes :: LITTLE );

        size_t bytes_per_sample = _bits_per_sample / 8;

        _sample_count = Bytes :: as< size_t >( file_stream + 40, 4, Bytes :: LITTLE )
                        /
                        bytes_per_sample;


        _stream = new double[ _sample_count ];

        if( !_stream ) {
            echo( this, Echo :: FAULT, "Sound stream allocation." ); return;
        }


        double max_sample = static_cast< double >( 1 << ( _bits_per_sample - 1 ) );

        for( size_t n = 0; n < _sample_count; ++n )
            _stream[ n ] = static_cast< double >(
                                Bytes :: as< int >( file_stream + 44 + n * bytes_per_sample, bytes_per_sample, Bytes :: LITTLE )
                            ) / max_sample;


        _channel_count = Bytes :: as< unsigned short >( file_stream + 22, 2, Bytes :: LITTLE );


        if( _sample_count % _channel_count != 0 )
            echo( this, Echo :: WARNING, "Samples do not condense." );

        
        _sample_count /= _channel_count;


        echo( this, Echo :: OK, "Created from: "s + path.data() );
    }

    Sound( const Sound& other ) = default;

    Sound( Sound&& other ) = delete;


    ~Sound() {
        stop();
    }

protected:
    Shared< double[] >       _stream             = nullptr;

    std :: list< double >    _needles            = {};

    size_t                   _sample_rate        = 0;
    size_t                   _bits_per_sample    = 0;
    size_t                   _sample_count       = 0;
    size_t                   _channel_count      = 0;

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
    virtual double _sample( size_t channel ) override {
        double amp = 0.0;

        if( _paused ) return amp;

        _needles.remove_if( [ this, &amp, &channel ] ( double& at ) {
            double raw = _stream[ static_cast< size_t >( at ) * _channel_count + channel ];

            amp +=  _filter ? _filter( raw, channel ) : raw
                    *
                    _volume * !_muted;


            if( channel == _channel_count - 1 )
                at += _velocity * _audio->velocity();


            if( static_cast< size_t >( at ) >= _sample_count ) {
                at = _velocity >= 0.0 ? 0.0 : _sample_count - 1.0;

                return !_looping;
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
        return is_locked() && has_stream();
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



bool Wave :: is_playing() const {
    return std :: find( _audio->_waves.begin(), _audio->_waves.end(), this )
           !=
           _audio->_waves.end();
}

void Wave :: play() {
    this->prepare_play();

    if( !is_playing() )
        _audio->_waves.push_back( this );
}



#pragma endregion Audio




#pragma region After



#pragma region Echo



const Echo& Echo :: _echo(
    Base*                invoker,
    Type                 type,
    std :: string_view   message
) const {
    #if defined( _ENGINE_ECHO )
        std :: cout << " [ "_echo_normal;

        OS :: console_color_to( type );

        std :: cout << _type_name( type ) << " ] "_echo_normal << _type_fill( type ) << "   "_echo_special;

        for( size_t l = 0; l < _depth; ++l )
            std :: cout << "-";

        std :: cout
        << "From "_echo_normal
        << "[ "
        << ""_echo_highlight
        << invoker->struct_type()
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



#pragma endregion After



};
