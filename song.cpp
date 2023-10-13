#pragma region DETAILS
/*
    [ GENERAL ]
        Engine:      Song

        Version:     n/a

        Hackerman:   Mipsan

        C++:         2023's GNU standard

        OSs:         Windows

    [ PRE-DEFINES ]
        SONG_ECHO --- logs stuff.
        SONG_UNIQUE_SURFACE --- enables quicker surface event routing.

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


#define _ENGINE_NAMESPACE Song

#define _ENGINE_STRUCT_TYPE( type ) "Song :: " type

#define _ENGINE_STRUCT_TYPE_MTD( type ) virtual std :: string_view struct_type() const override { return _ENGINE_STRUCT_TYPE( type ); }


#if defined( SONG_ECHO )
        #define _ENGINE_ECHO
#endif

#if defined( SONG_UNIQUE_SURFACE )
    #define _ENGINE_UNIQUE_SURFACE
#endif


#if defined( SONG_OS_WINDOWS )
    #define _ENGINE_OS_WINDOWS
#endif

#if defined( SONG_GL_DIRECT )
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



struct Base;

struct Audio;
struct Sound;



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


struct Echo {
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



    template< template< typename Type > typename Smart_ptr, typename Type, typename Derived_ptr >
    struct _Smart_ptr_extended : public Smart_ptr< Type > {
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
            this -> reset( ptr );

            return static_cast< Derived_ptr& >( *this );
        }

    public:
        operator Type_ptr () {
            return this -> get();
        }

        template< bool isnt_array = !is_array >
        operator std :: enable_if_t< isnt_array, Type_ref > () {
            return *this -> get();
        }

    public:
        Type_ptr operator + ( ptrdiff_t offset ) const {
            return this -> get() + offset;
        }

        Type_ptr operator - ( ptrdiff_t offset ) const {
            return this -> get() - offset;
        }

    };


    template< typename Type >
    struct Unique : public _Smart_ptr_extended< std :: unique_ptr, Type, Unique< Type > > {
    public:
        typedef   _Smart_ptr_extended< std :: unique_ptr, Type, Unique< Type > >   _Base;

    public:
        using _Base :: _Base;

        using _Base :: operator =;

    };


    template< typename Type >
    struct Shared : public _Smart_ptr_extended< std :: shared_ptr, Type, Shared< Type > > {
    public:
        typedef   _Smart_ptr_extended< std :: shared_ptr, Type, Shared< Type > >   _Base;

    public:
        using _Base :: _Base;

        using _Base :: operator =;

    };



#pragma endregion Tricks



#pragma region Utility



template< typename T >
struct Coord {
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
struct Size {
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



struct File {
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



struct Bytes {
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



struct Env {
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



struct Base {
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



template< typename Inheritor >
struct Is_base : public Base {
protected:
    Is_base() 
    : Base{ idx_of< Inheritor >() }
    {}

};



#pragma endregion Base



#pragma region Audio



struct Sound : public Is_base< Sound >
{
public:
    friend struct Audio;

public:
    typedef   std :: function< double( double, size_t ) >   Filter;

public:
    Sound() = default;

    Sound( 
        Audio&             audio, 
        std :: string_view path, 
        Echo               echo  = {} 
    );

    Sound( 
        std :: string_view path,
        Echo               echo   = {}
    )
    {
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


        echo( this, Echo :: OK, "Created from: "s + path.data() );
    }

    Sound( const Sound& other ) = default;

    Sound( Sound&& other ) = delete;


    ~Sound() {
        stop();
    }

private:
    Audio*                   _audio              = nullptr;

    Shared< double[] >       _stream             = nullptr;

    std :: list< double >    _needles            = {};

    size_t                   _sample_rate        = 0;
    size_t                   _bits_per_sample    = 0;
    size_t                   _sample_count       = 0;
    size_t                   _channel_count      = 0;

    bool                     _loop               = false;
    bool                     _pause              = false;
    bool                     _mute               = false;

    Filter                   _filter             = nullptr;
    double                   _volume             = 1.0;
    double                   _velocity           = 1.0;

public:
    Sound& lock_on( Audio& audio ) {
        _audio = &audio;

        return *this;
    }

    Sound& release() {
        _audio = nullptr;

        return *this;
    }

public:
    Audio& lock() const {
        return *_audio;
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
    bool is_playing() const;

    Sound& play();

    Sound& stop();

public:
    Sound& loop() {
        _loop = true;

        return *this;
    }

    Sound& unloop() {
        _loop = false;

        return *this;
    }

    Sound& swap_loop() {
        _loop ^= true;

        return *this;
    }

    bool is_looping() const {
        return _loop;
    }


    Sound& pause() {
        _pause = true;

        return *this;
    }

    Sound& resume() {
        _pause = false;

        return *this;
    }

    Sound& swap_pause() {
        _pause ^= true;

        return *this;
    }

    bool is_paused() const {
        return _pause;
    }


    Sound& mute() {
        _mute = true;

        return *this;
    }

    Sound& unmute() {
        _mute = false;

        return *this;
    }

    Sound& swap_mute() {
        _mute ^= true;

        return *this;
    }

    bool is_muted() {
        return _mute;
    }


    Sound& volume_to( double vlm ) {
        _volume = vlm;

        return *this;
    }

    double volume() const {
        return _volume;
    }


    Sound& filter_to( Filter flt ) {
        _filter = flt;

        return *this;
    }

    Filter filter() const {
        return _filter;
    }


    Sound& velocity_to( double vlc ) {
        _velocity = vlc;

        return *this;
    }

    double velocity() const {
        return _velocity;
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

public:
    _ENGINE_STRUCT_TYPE_MTD( "Sound" );

};



struct Audio : public Is_base< Audio >
{
public:
    static std :: string_view name() {
        return _ENGINE_STRUCT_TYPE( "Audio" );
    }

private:
    friend struct Sound;

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

    std :: list< Sound* >        _sounds               = {};

    bool                         _pause                = false;
    bool                         _mute                 = false;

    Sound :: Filter              _filter               = nullptr;
    double                       _volume               = 1.0;
    double                       _velocity             = 1.0;

private:
    void _main() {
        constexpr double max_sample = static_cast< double >(
            std :: numeric_limits< int > :: max()
            );


        auto clip = [] ( double amp ) -> double {
            return amp >= 0.0 ? std :: min( amp, 1.0 ) : std :: max( amp, -1.0 );
        };

        auto sample = [ this ] ( size_t channel ) -> double {
            double amp = 0.0;

            if( _pause ) return amp;

            for( Sound* snd : _sounds ) {
                if( snd -> _pause ) continue;

                snd -> _needles.remove_if( [ this, &snd, &amp, &channel ] ( double& at ) {
                    if( snd -> _filter )
                        amp += snd -> _filter(
                                snd -> _stream[ static_cast< size_t >( at ) ],
                                channel
                            )
                            *
                            snd -> _volume * !snd -> _mute
                            *
                            _volume * !_mute;
                    else
                        amp += snd -> _stream[ static_cast< size_t >( at ) ]
                            *
                            snd -> _volume * !snd -> _mute
                            *
                            _volume * !_mute;


                    if( channel == snd -> _channel_count - 1 ) {
                        if( 
                            static_cast< size_t >( at ) 
                            == 
                            static_cast< size_t >( at + _velocity * snd -> _velocity ) 
                        )
                            at -= channel;
                        else
                            at += 1.0;
                    } else
                        at += 1.0;


                    if( at >= snd -> _sample_count ) {
                        at = 0;

                        return !snd -> _loop;
                    }

                    return false;
                } );
            }

            if( _filter )
                return _filter( amp, channel );
            else
                return amp;
        };


        while( _powered ) {
            if( _free_block_count == 0 ) {
                std :: unique_lock< std :: mutex > lock{ _mtx };

                _cnd_var.wait( lock );
            }

            --_free_block_count;


            if( _wave_headers[ _block_current ].dwFlags & WHDR_PREPARED )
                waveOutUnprepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );


            _sounds.remove_if( [] ( Sound* snd ) {
                return snd -> _needles.empty();
            } );


            size_t current_block = _block_current * _block_sample_count;

            for( size_t n = 0; n < _block_sample_count; n += _channel_count )
                for( size_t c = 0; c < _channel_count; ++c )
                    _block_memory[ current_block + n + c ] = static_cast< int >( clip( sample( c ) ) * max_sample );


            waveOutPrepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );
            waveOutWrite( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );


            ++_block_current;
            _block_current %= _block_count;
        }

    }

private:
    static void CALLBACK event_proc_router( HWAVEOUT hwo, UINT event, DWORD_PTR instance, DWORD w_param, DWORD l_param ) {
        reinterpret_cast< Audio* >( instance ) -> event_proc( hwo, event, w_param, l_param);
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

public:
    Audio& pause() {
        _pause = true;

        return *this;
    }

    Audio& resume() {
        _pause = false;

        return *this;
    }

    Audio& swap_pause() {
        _pause ^= true;

        return *this;
    }

    bool is_paused() const {
        return _pause;
    }


    Audio& mute() {
        _mute = true;

        return *this;
    }

    Audio& unmute() {
        _mute = false;

        return *this;
    }

    Audio& swap_mute() {
        _mute ^= true;

        return *this;
    }

    bool is_muted() {
        return _mute;
    }


    Audio& volume_to( double vlm ) {
        _volume = vlm;

        return *this;
    }

    double volume() const {
        return _volume;
    }


    Audio& filter_to( Sound :: Filter flt ) {
        _filter = flt;

        return *this;
    }

    Sound :: Filter filter() const {
        return _filter;
    }


    Audio& velocity_to( double vlc ) {
        _velocity = vlc;

        return *this;
    }

    double velocity() const {
        return _velocity;
    }


public:
    std :: string_view device() const {
        return _device;
    }

    Audio& device_to( std :: string_view dev ) {
        return *this;
    }

public:
    _ENGINE_STRUCT_TYPE_MTD( "Audio" );

};



Sound :: Sound( 
    Audio&             audio, 
    std :: string_view path, 
    Echo               echo
)
: Sound{ path }
{
    _audio = &audio;

    if( _sample_rate != _audio -> _sample_rate )
        echo( this, Echo :: WARNING, "Sample rate does not match with locked on audio's." );


    if(
        _audio -> _channel_count
        !=
        _channel_count
    )
        echo( this, Echo :: WARNING, "Channel count does not match with locked on audio's." );
}

bool Sound :: is_playing() const {
    return std :: find( _audio -> _sounds.begin(), _audio -> _sounds.end(), this )
           !=
           _audio -> _sounds.end();
}

Sound& Sound :: play() {
    _needles.push_back( 0 );

    if( !is_playing() )
        _audio -> _sounds.push_back( this );

    return *this;
}

Sound& Sound :: stop() {
    _needles.clear();

    return *this;
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
        << invoker -> struct_type()
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
