#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/endec.hpp>
#include <IXT/comms.hpp>
#include <IXT/concepts.hpp>

namespace _ENGINE_NAMESPACE {



class Wave;
class WaveBehaviour;
class Audio;



class Wave : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Wave" );

public:
    friend class Audio;

public:
    typedef   uint16_t   tunnel_t;
    typedef   double     volume_t;
    typedef   double     velocity_t;

public:
    typedef   std::function< double( double, tunnel_t ) >   Filter;

public:
    Wave() = default;

    Wave( std::shared_ptr< Audio > audio )
    : _audio{ std::move( audio ) }
    {}

_ENGINE_PROTECTED:
    std::shared_ptr< Audio >   _audio   = nullptr;

public:
    bool is_playing() const;

    void play();

public:
    virtual void set() = 0;

    virtual void stop() = 0;

    virtual bool done() const = 0;

public:
    bool is_docked() const {
        return _audio != nullptr;
    }

    Wave& dock_in( std::shared_ptr< Audio > audio ) {
        _audio = std::move( audio );
        return *this;
    }

    Wave& dock_out() {
        _audio = nullptr;
        return *this;
    }

public:
    Audio& audio() const {
        return *_audio;
    }


_ENGINE_PROTECTED:
    virtual double _sample( double elapsed, tunnel_t tunnel, bool advance ) = 0;

};



struct WaveMetas {
_ENGINE_PROTECTED:
    double         _volume     = 1.0;
    bool           _paused     = false;
    bool           _muted      = false;
    bool           _looping    = false;
    double         _velocity   = 1.0;
    Wave::Filter   _filter     = {};

public:
    Wave::volume_t volume() const {
        return _volume;
    } 

public:
    WaveMetas& volume_at( Wave::volume_t vlm ) {
        _volume = std::clamp( vlm, -1.0, 1.0 );
        return *this;
    }

    WaveMetas& volume_tweak( const auto& op, const auto& rhs ) {
        _volume = std::clamp( std::invoke( op, _volume, rhs ), -1.0, 1.0 );
        return *this;
    }

public:
    bool is_paused() const {
        return _paused;
    }

    WaveMetas& pause() {
        _paused = true;
        return *this;
    }

    WaveMetas& resume() {
        _paused = false;
        return *this;
    }

    WaveMetas& pause_tweak() {
        _paused ^= true;
        return *this;
    }

public:
    bool is_muted() const {
        return _muted;
    }

    WaveMetas& mute() {
        _muted = true;
        return *this;
    }

    WaveMetas& unmute() {
        _muted = false;
        return *this;
    }

    WaveMetas& mute_tweak() {
        _muted ^= true;
        return *this;
    }

public:
    bool is_looping() const {
        return _looping;
    }

    WaveMetas& loop() {
        _looping = true;
        return *this;
    }

    WaveMetas& unloop() {
        _looping = false;
        return *this;
    }

    WaveMetas& loop_tweak() {
        _looping ^= true;
        return *this;
    }

public:
    Wave::velocity_t velocity() const {
        return _velocity;
    }

    WaveMetas& velocity_at( Wave::velocity_t vlc ) {
        _velocity = vlc;
        return *this;
    }

    WaveMetas& velocity_tweak( const auto& op, const auto& rhs ) {
        _velocity = std::invoke( op, _velocity, rhs );
        return *this;
    }

public:
    Wave::Filter filter() const {
        return _filter;
    }

    WaveMetas& filter_with( const Wave::Filter& flt ) {
        _filter = flt;
        return *this;
    }

    WaveMetas& remove_filter() {
        _filter = nullptr;
        return *this;
    }

};



class Audio : public Descriptor, public WaveMetas {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Audio" );

public:
    Audio() = default;

    Audio(
        std::string_view   device,
        size_t             sample_rate          = 48'000,
        size_t             tunnel_count         = 1,
        size_t             block_count          = 16,
        size_t             block_sample_count   = 256,
        _ENGINE_COMMS_ECHO_ARG
    )
    : _sample_rate       { sample_rate },
      _time_step         { 1.0 / _sample_rate },
      _tunnel_count      { tunnel_count },
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
            echo( this, ECHO_STATUS_ERROR ) << "Device \"" << _device << "\" does not exist.";
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
            echo( this, ECHO_STATUS_ERROR ) << "Could NOT open wave to device.";
            return;
        }


        _block_memory.reset( new int[ _block_count * _block_sample_count ] );

        if( !_block_memory ) {
            echo( this, ECHO_STATUS_ERROR ) << "Blocks bad alloc."; 
            return;
        }

        std::fill_n( _block_memory.get(), _block_count * _block_sample_count, 0 );


        _wave_headers.reset( new WAVEHDR[ _block_count ] );

        if( !_wave_headers ) {
            echo( this, ECHO_STATUS_ERROR ) << "Wave headers bad alloc.";
            return;
        }

        std::fill_n( ( char* ) _wave_headers.get(), sizeof( WAVEHDR ) * _block_count, 0 );


        for( size_t n = 0; n < _block_count; ++n ) {
            _wave_headers[ n ].dwBufferLength = sizeof( int ) * _block_sample_count;
            _wave_headers[ n ].lpData = ( char* ) ( _block_memory.get() + ( n * _block_sample_count ) );
        }


        _powered = true;

        _thread = std::thread( _main, this );

        if( !_thread.joinable() ) {
            echo( this, ECHO_STATUS_ERROR ) << "Could NOT launch main thread."; 
            return;
        }

        std::unique_lock< std::mutex > lock{ _mtx };
        _cnd_var.notify_one();

        echo( this, ECHO_STATUS_OK ) << "Created. Streaming to \"" << _device << "\".";
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
    volatile bool                          _powered              = false;

    size_t                                 _sample_rate          = 0;
    double                                 _time_step            = 0.0;
    double                                 _elapsed              = 0.0;
    size_t                                 _tunnel_count         = 0;
    size_t                                 _block_count          = 0;
    size_t                                 _block_sample_count   = 0;
    size_t                                 _block_current        = 0;
    std::unique_ptr< int[] >               _block_memory         = nullptr;

    std::unique_ptr< WAVEHDR[] >           _wave_headers         = nullptr;
    HWAVEOUT                               _wave_out             = nullptr;
    std::string                            _device               = {};

    std::thread                            _thread               = {};

    std::atomic< size_t >                  _free_block_count     = 0;
    std::condition_variable                _cnd_var              = {};
    std::mutex                             _mtx                  = {};

    std::list< std::shared_ptr< Wave > >   _waves                = {};

_ENGINE_PROTECTED:
    void _main() {
        constexpr double max_sample = static_cast< double >(
            std::numeric_limits< int >::max()
        );

        
        auto sample = [ this ] ( Wave::tunnel_t tunnel ) -> double {
            double amp = 0.0;

            if( _paused ) return amp;

            for( auto& wave : _waves )
                amp += wave->_sample( _elapsed, tunnel, tunnel == _tunnel_count - 1 );

            return _filter ? _filter( amp, tunnel ) : amp
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


            _waves.remove_if( [] ( auto& wave ) {
                return wave->done();
            } );
            
            
            size_t current_block = _block_current * _block_sample_count;

            for( size_t n = 0; n < _block_sample_count; n += _tunnel_count ) {
                for( size_t tnl = 0; tnl < _tunnel_count; ++tnl )
                    _block_memory[ current_block + n + tnl ] = static_cast< int >( 
                        std::clamp( sample( tnl ), -1.0, 1.0 ) * max_sample 
                    );
                
                _elapsed += _time_step;
            }
            
            
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

    size_t tunnel_count() const {
        return _tunnel_count;
    }

public:
    bool is_playing( const Wave& wave ) {
        return std::find_if( _waves.begin(), _waves.end(), [ &wave ] ( auto& entry ) -> bool {
            return entry->uid() == wave.uid();
        } ) != _waves.end();
    }

    Audio& play( std::shared_ptr< Wave > wave ) {
        wave->set();

        if( !this->is_playing( *wave ) )
            _waves.emplace_back( std::move( wave ) );

        return *this;
    }

};



class Sound : public Wave, public WaveMetas {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Sound" );

public:
    Sound() = default;

    Sound( 
        std::shared_ptr< Audio >   audio, 
        std::string_view           path, 
        _ENGINE_COMMS_ECHO_ARG 
    )
    : Wave{ std::move( audio ) }
    {
        using namespace std::string_literals;


        if( path.ends_with( ".wav" ) ) {
            Endec::Wav< double > wav{ path, echo };

            _stream         = std::move( wav.stream );
            _sample_rate    = wav.sample_rate;
            _sample_count   = wav.sample_count;
            _tunnel_count  = wav.tunnel_count;

            echo( this, ECHO_STATUS_OK ) << "Created from: \"" << path.data() << "\".";
            return;
        }

        echo( this, ECHO_STATUS_ERROR ) << "Unsupported format: \"" << path.substr( path.find_last_of( '.' ) ) << "\".";
    

        if( !audio ) return;

        if( _sample_rate != _audio->sample_rate() )
            echo( this, ECHO_STATUS_WARNING ) << "Sample rate does not match with docked in audio's.";


        if( _tunnel_count != _audio->tunnel_count() )
            echo( this, ECHO_STATUS_WARNING ) << "Tunnel count does not match with docked in audio's.";
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
    std::shared_ptr< double[] >    _stream         = nullptr;

    std::list< double >            _needles        = {};

    uint64_t                       _sample_rate    = 0;
    uint64_t                       _sample_count   = 0;
    uint16_t                       _tunnel_count   = 0;

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
    virtual double _sample( double elapsed, Wave::tunnel_t tunnel, bool advance ) override {
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

};


/*
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
*/



};
