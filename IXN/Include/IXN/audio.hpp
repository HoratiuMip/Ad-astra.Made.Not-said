#pragma once
/*
*/

#include <IXN/descriptor.hpp>
#include <IXN/comms.hpp>
#include <IXN/concepts.hpp>
#include <IXN/endec.hpp>
#include <IXN/tempo.hpp>
#include <IXN/hyper-vector.hpp>

namespace _ENGINE_NAMESPACE {



#if defined( _ENGINE_AVX )
    /* #define _ENGINE_AVX_INTRIN_CONCAT( avx_bits, suffix ) _mm##avx_bits##suffix */
    #define   _ENGINE_AUDIO_AVX_SELECT_PD( vector, offset )   ( *( ( double* )&vector + offset ) )

    #define   _ENGINE_AUDIO_AVX_ALIGN   ( _ENGINE_AVX / ( sizeof( double ) * 8 ) )

    #if _ENGINE_AVX == 256
        #define   _engine_audio__mAVXd               __m256d
        #define   _engine_audio__mAVXi               __m128i

        #define   _engine_audio_mmAVX_set1_pd        _mm256_set1_pd

        #define   _engine_audio_mmAVX_mul_pd         _mm256_mul_pd
        #define   _engine_audio_mmAVX_add_pd         _mm256_add_pd

        #define   _engine_audio_mmAVX_min_pd         _mm256_min_pd
        #define   _engine_audio_mmAVX_max_pd         _mm256_max_pd

        #define   _engine_audio_mmAVX_cvtpd_epi32    _mm256_cvtpd_epi32
    #elif _ENGINE_AVX == 512
        #define   _engine_audio__mAVXd               __m512d
        #define   _engine_audio__mAVXi               __m256i

        #define   _engine_audio_mmAVX_set1_pd        _mm512_set1_pd

        #define   _engine_audio_mmAVX_mul_pd         _mm512_mul_pd
        #define   _engine_audio_mmAVX_add_pd         _mm512_add_pd

        #define   _engine_audio_mmAVX_min_pd         _mm512_min_pd
        #define   _engine_audio_mmAVX_max_pd         _mm512_max_pd

        #define   _engine_audio_mmAVX_cvtpd_epi32    _mm512_cvtpd_epi32
    #endif
#endif



class  Wave;
struct WaveMeta;
class  Audio;



struct WaveMeta {
public:
    typedef   std::function< double( double, WORD ) >   Filter;

_ENGINE_PROTECTED:
    double   _volume     = 1.0;
    bool     _paused     = false;
    bool     _muted      = false;
    bool     _looping    = false;
    double   _velocity   = 1.0;
    Filter   _filter     = {};
#if defined( _ENGINE_AVX )
    struct {
        _engine_audio__mAVXd      volume   = { _engine_audio_mmAVX_set1_pd( 1.0 ) };
    }        _avx        = {};            
#endif

public:
    double volume() const {
        return _volume;
    } 

public:
    WaveMeta& volume_at( double vlm ) {
        _volume = std::clamp( vlm, -1.0, 1.0 );
    #if defined( _ENGINE_AVX )
        _avx.volume = _engine_audio_mmAVX_set1_pd( _volume );
    #endif
        return *this;
    }

    WaveMeta& volume_tweak( const auto& op, const auto& rhs ) {
        _volume = std::clamp( std::invoke( op, _volume, rhs ), -1.0, 1.0 );
    #if defined( _ENGINE_AVX )
        _avx.volume = _engine_audio_mmAVX_set1_pd( _volume );
    #endif
        return *this;
    }

public:
    bool is_paused() const {
        return _paused;
    }

    WaveMeta& pause() {
        _paused = true;
        return *this;
    }

    WaveMeta& resume() {
        _paused = false;
        return *this;
    }

    WaveMeta& pause_tweak() {
        _paused ^= true;
        return *this;
    }

public:
    bool is_muted() const {
        return _muted;
    }

    WaveMeta& mute() {
        _muted = true;
        return *this;
    }

    WaveMeta& unmute() {
        _muted = false;
        return *this;
    }

    WaveMeta& mute_tweak() {
        _muted ^= true;
        return *this;
    }

public:
    bool is_looping() const {
        return _looping;
    }

    WaveMeta& loop() {
        _looping = true;
        return *this;
    }

    WaveMeta& unloop() {
        _looping = false;
        return *this;
    }

    WaveMeta& loop_tweak() {
        _looping ^= true;
        return *this;
    }

public:
    double velocity() const {
        return _velocity;
    }

    WaveMeta& velocity_at( double vlc ) {
        _velocity = vlc;
        return *this;
    }

    WaveMeta& velocity_tweak( const auto& op, const auto& rhs ) {
        _velocity = std::invoke( op, _velocity, rhs );
        return *this;
    }

public:
    Filter filter() const {
        return _filter;
    }

    WaveMeta& filter_with( const Filter& flt ) {
        _filter = flt;
        return *this;
    }

    WaveMeta& remove_filter() {
        _filter = nullptr;
        return *this;
    }

};

class Wave : public Descriptor, public WaveMeta {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Wave" );

public:
    friend class Audio;

public:
    Wave() = default;

    Wave( HVEC< Audio > audio )
    : _audio{ std::move( audio ) }
    {}

public:
    virtual ~Wave() = default;

_ENGINE_PROTECTED:
    HVEC< Audio >   _audio   = nullptr;
    
public:
    bool is_playing() const;

    void play();

    void play( HVEC< Wave > self );

public:
    virtual void set() = 0;

    virtual void stop() = 0;

    virtual bool done() const = 0;

public:
    bool is_docked() const {
        return _audio != nullptr;
    }

    Wave& dock_in( HVEC< Audio > audio ) {
        _audio = std::move( audio );
        return *this;
    }

    Wave& dock_out() {
        _audio = nullptr;
        return *this;
    }

public:
    Audio& audio() {
        return *_audio;
    }

_ENGINE_PROTECTED:
    virtual double _sample( double elapsed, WORD tunnel, bool tunnel_end ) = 0;

#if defined( _ENGINE_AVX )
public:
    virtual const DWORD waveid_assert_avx() const = 0;

_ENGINE_PROTECTED:
    virtual _engine_audio__mAVXd _sample_avx( _engine_audio__mAVXd elapsed, _engine_audio__mAVXi tunnel, __mmask8 tunnel_end ) {
        return _engine_audio_mmAVX_set1_pd( 0.0 );
    }
#endif

};



class Audio : public Descriptor, public WaveMeta {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Audio" );

public:
    Audio() = default;

    Audio(
        std::string_view   device,
        DWORD              sample_rate          = 48'000,
        WORD               tunnel_count         = 1,
        DWORD              block_count          = 16,
        DWORD              block_sample_count   = 256,
        _ENGINE_COMMS_ECHO_ARG
    )
    : _sample_rate       { sample_rate },
      _time_step         { 1.0 / _sample_rate },
      _tunnel_count      { tunnel_count },
      _block_count       { block_count },
      _block_sample_count{ block_sample_count * tunnel_count },
      _block_current     { 0 },
      _blocks_memory     { nullptr },
      _wave_headers      { nullptr },
      _device            { device.data() },
      _free_block_count  { block_count }
    { 
    #if defined( _ENGINE_AVX )
        if( block_sample_count % _ENGINE_AUDIO_AVX_ALIGN != 0 ) {
            echo( this, EchoLevel_Error ) << "Block sample count is not " << _ENGINE_AUDIO_AVX_ALIGN << " sample aligned. Cannot use AVX-" << _ENGINE_AVX << ".";
            return;
        }
        echo( this, EchoLevel_Ok ) << "Block sample count aligned for AVX-" << _ENGINE_AVX << ".";
    #endif

        _blocks_memory.reset( new int[ _block_count * _block_sample_count ] );

        if( !_blocks_memory ) {
            echo( this, EchoLevel_Error ) << "Blocks bad alloc."; 
            return;
        }

        std::fill_n( _blocks_memory.get(), _block_count * _block_sample_count, 0 );


        _wave_headers.reset( new WAVEHDR[ _block_count ] );

        if( !_wave_headers ) {
            echo( this, EchoLevel_Error ) << "Wave headers bad alloc.";
            return;
        }

        std::fill_n( ( char* ) _wave_headers.get(), sizeof( WAVEHDR ) * _block_count, 0 );


        for( size_t n = 0; n < _block_count; ++n ) {
            _wave_headers[ n ].dwBufferLength = sizeof( int ) * _block_sample_count;
            _wave_headers[ n ].lpData = ( char* ) ( _blocks_memory.get() + ( n * _block_sample_count ) );
        }


        uint32_t dev_idx = 0;

        auto devs = devices();

        for( auto& dev : devs ) {
            if( dev == _device ) break;

            ++dev_idx;
        }

        if( dev_idx == devs.size() ) {
            echo( this, EchoLevel_Error ) << "Device \"" << _device << "\" does not exist.";
            return;
        }

 
        WAVEFORMATEX wave_format;

        wave_format.wFormatTag      = WAVE_FORMAT_PCM;
        wave_format.nSamplesPerSec  = _sample_rate;
        wave_format.wBitsPerSample  = sizeof( int ) * 8;
        wave_format.nChannels       = _tunnel_count;
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
            echo( this, EchoLevel_Error ) << "Could NOT open wave to device.";
            return;
        }


        _powered.store( true, std::memory_order_seq_cst );

        _thread = std::thread( _main, this );

        if( !_thread.joinable() ) {
            echo( this, EchoLevel_Error ) << "Could NOT launch main thread."; 
            return;
        }

        std::unique_lock< std::mutex > lock{ _mtx };
        _cnd_var.notify_one();

        echo( this, EchoLevel_Ok ) << "Created. Streaming to \"" << _device << "\".";
    }


    Audio( const Audio& ) = delete;

    Audio( Audio&& ) = delete;


    ~Audio() {
        _powered.store( false, std::memory_order_seq_cst );

        _cnd_var.notify_one();

        if( _thread.joinable() )
            _thread.join();

        waveOutReset( _wave_out );
        waveOutClose( _wave_out );
    }

_ENGINE_PROTECTED:
    std::atomic< bool >         _powered              = false;

    DWORD                       _sample_rate          = 0;
    double                      _time_step            = 0.0;
    WORD                        _tunnel_count         = 0;

    DWORD                       _block_count          = 0;
    DWORD                       _block_sample_count   = 0;
    DWORD                       _block_current        = 0;
    UPtr< int[] >               _blocks_memory        = nullptr;

    UPtr< WAVEHDR[] >           _wave_headers         = nullptr;
    HWAVEOUT                    _wave_out             = nullptr;
    std::string                 _device               = {};

    std::thread                 _thread               = {};

    std::atomic< DWORD >        _free_block_count     = 0;
    std::condition_variable     _cnd_var              = {};
    std::mutex                  _mtx                  = {};

    std::list< HVEC< Wave > >   _waves                = {};

#if defined( _ENGINE_AVX )
    struct {
        _engine_audio__mAVXd   elapsed   = { _engine_audio_mmAVX_set1_pd( 0.0 ) };
    }                           _avx                  = {};
#else
    double                      _elapsed              = 0.0;
#endif

_ENGINE_PROTECTED:
    void _main() {
    #if defined( _ENGINE_AVX )
        const _engine_audio__mAVXd max_sample = _engine_audio_mmAVX_set1_pd( std::numeric_limits< int >::max() );
    #else
        constexpr double max_sample = static_cast< double >(
            std::numeric_limits< int >::max()
        );
    #endif

    #if defined( _ENGINE_AVX )
        _engine_audio__mAVXd avx_time_step = _engine_audio_mmAVX_set1_pd( _time_step ); 
        WORD avx_tunnel[ _ENGINE_AUDIO_AVX_ALIGN + _tunnel_count ];
        bool avx_tunend[ _ENGINE_AUDIO_AVX_ALIGN + _tunnel_count ];
        BYTE avx_tunoff = 0;
        _engine_audio__mAVXd avx_amp_low = _engine_audio_mmAVX_set1_pd( -1.0 );
        _engine_audio__mAVXd avx_amp_high = _engine_audio_mmAVX_set1_pd( 1.0 );

        for( BYTE idx = 0; idx < _ENGINE_AUDIO_AVX_ALIGN; ++idx )
            _ENGINE_AUDIO_AVX_SELECT_PD( _avx.elapsed, idx ) = idx;
        _avx.elapsed = _engine_audio_mmAVX_mul_pd( avx_time_step, _avx.elapsed );  

        for( WORD t = 0; t < sizeof( avx_tunnel ) / sizeof( WORD ); ++t ) {
            avx_tunnel[ t ] = t % _tunnel_count;
            avx_tunend[ t ] = ( avx_tunnel[ t ] == _tunnel_count - 1 );
        }

        auto sample = [ this, &avx_tunend, &avx_tunoff ] ( _engine_audio__mAVXi tunnel ) -> _engine_audio__mAVXd {
            _engine_audio__mAVXd amp = _engine_audio_mmAVX_set1_pd( 0.0 );

            if( _paused ) return amp;

            for( auto& wave : _waves )
                if( wave->waveid_assert_avx() ) {
                    amp += wave->_sample_avx( _avx.elapsed, tunnel, {} ); /*MARK_NOT_DONE*/
                } else {
                    for( BYTE idx = 0; idx < _ENGINE_AUDIO_AVX_ALIGN; ++idx )
                        _ENGINE_AUDIO_AVX_SELECT_PD( amp, idx ) += wave->_sample(
                            _ENGINE_AUDIO_AVX_SELECT_PD( _avx.elapsed, idx ),
                            _ENGINE_AUDIO_AVX_SELECT_PD( tunnel, idx ),
                            *( avx_tunend + avx_tunoff + idx )
                        );
                }

            if( !_muted )
                return _engine_audio_mmAVX_mul_pd( amp, WaveMeta::_avx.volume );
            return _engine_audio_mmAVX_set1_pd( 0.0 );
        };
    #else
        auto sample = [ this ] ( WORD tunnel ) -> double {
            double amp = 0.0;

            if( _paused ) return amp;

            for( auto& wave : _waves )
                amp += wave->_sample( _elapsed, tunnel, tunnel == _tunnel_count - 1 );

            return _filter ? _filter( amp, tunnel ) : amp
                   * _volume * !_muted;
        };
    #endif
        
        while( _powered.load( std::memory_order_relaxed ) ) {
            if( _free_block_count.load( std::memory_order_consume ) == 0 ) {
                std::unique_lock< std::mutex > lock{ _mtx, std::defer_lock_t{} };

                if( lock.try_lock() )
                    _cnd_var.wait( lock );
            }

            _free_block_count.fetch_sub( 1, std::memory_order_release );

           
            if( _wave_headers[ _block_current ].dwFlags & WHDR_PREPARED )
                waveOutUnprepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );

            _waves.remove_if( [] ( auto& wave ) {
                return wave->done();
            } );
            
            
            int* current_block = _blocks_memory.get() + _block_current * _block_sample_count;
   
        #if defined( _ENGINE_AVX )
            for( WORD n = 0; n < _block_sample_count; n += _ENGINE_AUDIO_AVX_ALIGN ) {
                if( ( avx_tunoff += _ENGINE_AUDIO_AVX_ALIGN ) >= _tunnel_count )
                    avx_tunoff %= _tunnel_count;
           
                _engine_audio__mAVXd amp = _engine_audio_mmAVX_min_pd( 
                    _engine_audio_mmAVX_max_pd( sample( *( _engine_audio__mAVXi* )( avx_tunnel + avx_tunoff ) ), 
                    avx_amp_low ), avx_amp_high 
                );
               
                _engine_audio__mAVXi& current_vector = *( _engine_audio__mAVXi* )&current_block[ n ]; 
                current_vector = _engine_audio_mmAVX_cvtpd_epi32( _engine_audio_mmAVX_mul_pd( amp, max_sample ) ); 

                _avx.elapsed = _engine_audio_mmAVX_add_pd( _avx.elapsed, avx_time_step );
            }
        #else
            for( WORD n = 0; n < _block_sample_count; n += _tunnel_count ) {
                for( WORD tnl = 0; tnl < _tunnel_count; ++tnl )
                    current_block[ n + tnl ] = static_cast< int >( 
                        std::clamp( sample( tnl ), -1.0, 1.0 ) * max_sample 
                    );
                
                _elapsed += _time_step;
            }
        #endif
           
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
                if( _free_block_count.fetch_add( 1, std::memory_order_relaxed ) != 0 ) 
                    break;

                std::unique_lock< std::mutex > lock{ _mtx };
                _cnd_var.notify_one();
            break; }

            case WOM_CLOSE: {
                
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

    double elapsed() const {
    #if defined( _ENGINE_AVX )
        return *( double* )&_avx.elapsed;
    #else
        return _elapsed;
    #endif
    }

    uint16_t tunnel_count() const {
        return _tunnel_count;
    }

public:
    bool is_playing( const Wave& wave ) const {
        return std::find_if( _waves.begin(), _waves.end(), [ &wave ] ( auto& node ) -> bool {
            return node->xtdx() == wave.xtdx();
        } ) != _waves.end();
    }

    Audio& play( HVEC< Wave > wave ) {
        wave->set();

        if( !this->is_playing( *wave ) )
            _waves.emplace_back( std::move( wave ) );

        return *this;
    }

    Audio& stop() {
        for( auto& w : _waves )
            w->stop();

        return *this;
    }

};



class Sound : public Wave {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Sound" );

public:
    Sound() = default;

    Sound( 
        HVEC< Audio >      audio, 
        std::string_view   path, 
        _ENGINE_COMMS_ECHO_ARG 
    )
    : Wave{ std::move( audio ) }
    {
        using namespace std::string_literals;


        if( path.ends_with( ".wav" ) ) {
            Endec::Wav< double > wav{ path, echo };

            _stream       = std::move( wav.stream );
            _sample_rate  = wav.sample_rate;
            _sample_count = wav.sample_count;
            _tunnel_count = wav.tunnel_count;

            echo( this, EchoLevel_Ok ) << "Created from: \"" << path.data() << "\".";
        } else
            echo( this, EchoLevel_Error ) << "Unsupported format: \"" << path.substr( path.find_last_of( '.' ) ) << "\".";
    

        if( !_audio ) return;

        if( _sample_rate != _audio->sample_rate() )
            echo( this, EchoLevel_Warning ) << "Sample rate ( " << _sample_rate << " ) does not match with docked in audio's ( " << _audio->sample_rate() << " ).";

        if( _tunnel_count != _audio->tunnel_count() )
            echo( this, EchoLevel_Warning ) << "Tunnel count ( " << _tunnel_count << " ) does not match with docked in audio's ( " << _audio->tunnel_count() << " ).";

        echo( this, EchoLevel_Ok ) << "Audio docked.";
    }

    Sound( 
        std::string_view   path,
        _ENGINE_COMMS_ECHO_ARG
    ) : Sound{ nullptr, path, echo }
    {}


    ~Sound() {
        stop();
    }

_ENGINE_PROTECTED:
    HVEC< double[] >      _stream         = nullptr;

    std::list< double >   _needles        = {};

    DWORD                 _sample_rate    = 0;
    DWORD                 _sample_count   = 0;
    WORD                  _tunnel_count   = 0;

public:
    virtual void set() override {
        _needles.push_back( 0.0 );
    }

    virtual void stop() override {
        _needles.clear();
    }

    virtual bool done() const override {
        return _needles.empty();
    }

_ENGINE_PROTECTED:
    virtual double _sample( double elapsed, WORD tunnel, bool advance ) override {
        double amp = 0.0;

        if( _paused ) return amp;

        _needles.remove_if( [ this, &amp, &tunnel, &advance ] ( double& at ) {
            double raw = _stream[ static_cast< size_t >( at ) * _tunnel_count + tunnel ];

            amp +=  _filter ? _filter( raw, tunnel ) : raw
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
    bool has_stream() const {
        return _stream != nullptr;
    }

    operator bool () const {
        return this->is_docked() && this->has_stream();
    }

public:
    size_t sample_rate() const {
        return _sample_rate;
    }

    size_t tunnel_count() const {
        return _tunnel_count;
    }

    size_t sample_count() const {
        return _sample_count;
    }

    double duration() const {
        return static_cast< double >( _sample_count ) / _sample_rate;
    }

#if defined( _ENGINE_AVX )
public:
    virtual const DWORD waveid_assert_avx() const override {
       return 0;
    }

_ENGINE_PROTECTED:
    virtual _engine_audio__mAVXd _sample_avx( _engine_audio__mAVXd elapsed, _engine_audio__mAVXi tunnel, __mmask8 tunnel_end ) override {
        return _engine_audio_mmAVX_set1_pd( 0.0 );
    }
#endif

};



class Synth : public Wave {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Synth" );

public:
    typedef   std::function< double( double, WORD ) >   Generator;

public:
    Synth() = default;

    Synth( 
        HVEC< Audio >   audio,
        Generator       generator,
        double          decay_in_secs,
        _ENGINE_COMMS_ECHO_ARG
    )
    : Wave{ std::move( audio ) }, _generator{ generator }
    {
        echo( this, EchoLevel_Ok ) << "Created from source generator.";

        if( !_audio ) return;


        this->decay_in( decay_in_secs );


        echo( this, EchoLevel_Ok ) << "Audio docked.";
    }

    Synth(
        Generator   generator,
        _ENGINE_COMMS_ECHO_ARG
    )
    : Synth{ nullptr, generator, 0.0 }
    {}

_ENGINE_PROTECTED:
    Generator   _generator    = {};

    double      _elapsed      = 0.0;
    double      _decay        = 1.0;
    double      _decay_step   = 0.0;

public:
    virtual void set() override {
        _elapsed = 0.0;
        _decay   = 1.0;
    }

    virtual void stop() override {
        _decay = 0.0;
    }

    virtual bool done() const override {
        return _decay <= 1e-6;
    }


_ENGINE_PROTECTED:
    virtual double _sample( double elapsed, WORD tunnel, bool advance ) override {
        if( _paused ) return 0.0;

        if( advance )
            _elapsed += _audio->time_step() * _velocity * _audio->velocity();

        _decay -= _decay_step;
        
        if( this->Synth::done() ) {
            if( _looping )
                this->Synth::set();
            else
                return 0.0;
        }

        return _decay * std::invoke( _generator, _elapsed, tunnel ) * _volume * !_muted;
    }

public:
    Synth& decay_in( double secs ) {
        _decay_step = 1.0 / ( secs * _audio->sample_rate() );
        return *this;
    }

#if defined( _ENGINE_AVX )
public:
    virtual const DWORD waveid_assert_avx() const override {
        return 0;
    }

_ENGINE_PROTECTED:
    virtual _engine_audio__mAVXd _sample_avx( _engine_audio__mAVXd elapsed, _engine_audio__mAVXi tunnel, __mmask8 tunnel_end ) {
        return _engine_audio_mmAVX_set1_pd( 0 );
    }
#endif

public:
    static Generator gen_sine( double amp, double freq ) {
        return [ amp, freq ] ( double elapsed, [[maybe_unused]] WORD ) -> double {
            return sin( freq * 2.0 * PI * elapsed ) * amp; 
        };
    }

    static Generator gen_cos( double amp, double freq ) {
        return [ amp, freq ] ( double elapsed, [[maybe_unused]] WORD ) -> double {
            return cos( freq * 2.0 * PI * elapsed ) * amp; 
        };
    }

    static Generator gen_flat( double amp ) {
        return [ amp ] ( double elapsed, [[maybe_unused]] WORD ) -> double {
            return amp;
        };
    }

};



};
