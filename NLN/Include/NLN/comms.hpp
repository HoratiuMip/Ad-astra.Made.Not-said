#pragma once
/*===== NLN Logging - Vatca "Mipsan" Tudor-Horatiu
|
> THIS SECTION REQUIRES HEAVY MODIFICATION.
> ATTEMPT TO KEEP THE SYNTAX comms( *, ECHO_LEVEL )/echo( *, ECHO_LEVEL ).
> IF NOT... GOD BLESS.
|
======*/

#include <NLN/descriptor.hpp>
#include <NLN/concepts.hpp>
#include <NLN/os.hpp>

namespace _ENGINE_NAMESPACE {



#define  _ENGINE_COMMS_ECHO_ARG         Echo echo = {}
#define  _ENGINE_COMMS_ECHO_NO_DFT_ARG  Echo echo
#define  _ENGINE_COMMS_ECHO_RT_ARG      Echo echo = { nullptr }
#define  NLN_COMMS_ECHO_ARG             _ENGINE_NAMESPACE::_ENGINE_COMMS_ECHO_ARG
#define  NLN_COMMS_ECHO_NO_DFT_ARG      _ENGINE_NAMESPACE::_ENGINE_COMMS_ECHO_NO_DFT_ARG
#define  NLN_COMMS_ECHO_RT_ARG          _ENGINE_NAMESPACE::_ENGINE_COMMS_ECHO_RT_ARG


class Echo; class Comms;

enum ECHO_MODE {
    ECHO_MODE_RT, ECHO_MODE_ACC
};

enum ECHO_LEVEL {
    ECHO_LEVEL_OK      = 0,
    ECHO_LEVEL_WARNING = 1,
    ECHO_LEVEL_ERROR   = 2,
    ECHO_LEVEL_INTEL   = 3,
    ECHO_LEVEL_PENDING = 4,
    ECHO_LEVEL_INPUT   = 5,
    ECHO_LEVEL_DEBUG   = 6
};

struct _RtEchoOneLiner {
    _RtEchoOneLiner( Comms& c, Echo& e, const Descriptor* that, bool lock, ECHO_LEVEL level );

    ~_RtEchoOneLiner();

    Comms&   comms;
    Echo&    echo;
    bool     locked;

    template< typename T >
    requires is_std_ostringstream_pushable< std::decay_t< T > >
    Echo& operator << ( T&& frag );
};

// template< ECHO_MODE MODE >
// requires ( MODE == ECHO_MODE_RT || MODE == ECHO_MODE_ACC )
class Echo {
public:
    friend class Comms;

public:
    using out_stream_t = std::ostream;

    using descriptor_t = char;

    using Dump = std::tuple< std::ostringstream, std::vector< descriptor_t > >;

public:
    static constexpr descriptor_t   desc_color_mask   = 0b1111;
    static constexpr char           desc_switch       = '$';

_ENGINE_PROTECTED:
    inline static OS::CONSOLE_CLR _status_colors[] = {
        OS::CONSOLE_CLR_GREEN, OS::CONSOLE_CLR_YELLOW, OS::CONSOLE_CLR_RED, OS::CONSOLE_CLR_TURQ, OS::CONSOLE_CLR_BLUE, OS::CONSOLE_CLR_PINK, OS::CONSOLE_CLR_GRAY
    };

    inline static const char* _status_strs[] = {
        "OK", "WARNING", "ERROR", "INTEL", "PENDING", "INPUT", "DEBUG"
    };

public:
    Echo();

    Echo( const Echo& other )
    : _dump{ other._dump }, _depth{ other._depth + 1 }
    {}

    Echo( [[maybe_unused]]decltype( nullptr ) )
    : Echo{ nullptr, 0 }
    {}

_ENGINE_PROTECTED:
    Echo( Dump* dump, int64_t depth )
    : _dump{ dump }, _depth{ depth }
    {}

public:
    virtual ~Echo();

_ENGINE_PROTECTED:
    enum _DUMP_ACCESS_IDX {
        _STR, _DESCS
    };

    Dump*     _dump    = nullptr;
    int64_t   _depth   = 0;

_ENGINE_PROTECTED:
    auto& _acc_str() {
        return std::get< _STR >( *_dump );
    }

    const auto& _acc_str() const {
        return std::get< _STR >( *_dump );
    }

    out_stream_t& _any_str();

    auto& _descs() {
        return std::get< _DESCS >( *_dump );
    }

    const auto& _descs() const {
        return std::get< _DESCS >( *_dump );
    }

public:
    template< typename T >
    requires is_std_ostringstream_pushable< std::decay_t< T > >
    Echo& operator << ( T&& frag ) {
        this->_any_str() << std::forward< T >( frag );

        return *this;
    }

public:
    Echo& push_desc( descriptor_t desc );

    Echo& push_color( OS::CONSOLE_CLR color ) {
        return this->push_desc( color & desc_color_mask );    
    }

    Echo& gray()   { return this->push_color( OS::CONSOLE_CLR_GRAY ); }
    Echo& blue()   { return this->push_color( OS::CONSOLE_CLR_BLUE ); }
    Echo& green()  { return this->push_color( OS::CONSOLE_CLR_GREEN ); }
    Echo& red()    { return this->push_color( OS::CONSOLE_CLR_RED ); }
    Echo& pink()   { return this->push_color( OS::CONSOLE_CLR_TURQ ); }
    Echo& yellow() { return this->push_color( OS::CONSOLE_CLR_YELLOW ); }
    Echo& white()  { return this->push_color( OS::CONSOLE_CLR_WHITE ); }

_ENGINE_PROTECTED:
    inline static struct _UnknownInvoker : public Descriptor {
        _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Echo::Unknwn" );
    } _unknown_invoker_placeholder;

public:
    _RtEchoOneLiner operator () ( const Descriptor& invoker, ECHO_LEVEL status );

    _RtEchoOneLiner operator() ( const Descriptor* invoker, ECHO_LEVEL status ) {
        return this->operator()( *invoker, status );
    }

    template< typename T >
    requires( !is_descriptor_tolerant< T > )
    _RtEchoOneLiner operator() ( const T& invoker, ECHO_LEVEL status ) {
        return this->operator()( _unknown_invoker_placeholder, status );
    }

    template< typename T >
    requires( !is_descriptor_tolerant< T > )
    _RtEchoOneLiner operator() ( const T* invoker, ECHO_LEVEL status ) {
        return this->operator()( *invoker, status );
    }

    Echo& operator [] ( const Descriptor& invoker ) {
        this->operator<<( '\n' );
        
        const char* struct_name = invoker.struct_name();

        return this->white()
        .operator<<( "[ " )
        .red()
        .operator<<( struct_name ? struct_name : "NULL" )
        .white()
        .operator<<( " ][ " )
        .red()
        .operator<<( invoker.xtdx() )
        .white()
        .operator<<( " ]" )
        .red()
        .operator<<( " -> " )
        .white();
    }

    Echo& operator[] ( const Descriptor* invoker ) {
        return this->operator[]( *invoker );
    }

};



class Comms : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Comms" );

public:
    friend class Echo;

public:
    using out_stream_t = Echo::out_stream_t;

public:
    using desc_proc_key_t   = std::reference_wrapper< const std::type_info >;
    using desc_proc_value_t = std::function< void( Echo::descriptor_t ) >;

public:
    Comms()
    : Comms{ std::cout }
    {}

    template< typename T >
    requires std::is_base_of_v< out_stream_t, T >
    Comms( T& stream ) {
        _desc_procs.emplace( typeid( nullptr ), [] ( [[ maybe_unused ]] Echo::descriptor_t ) -> void {} );

        _desc_procs.emplace( typeid( std::cout ), [] ( Echo::descriptor_t desc ) -> void {
            OS::console.clr_with( static_cast< OS::CONSOLE_CLR >( desc & Echo::desc_color_mask ) );
        } );

        this->stream_to< T >( stream );


        OS::sig_interceptor.push_on_external_exception( this->xtdx(), _flush );
    }

_ENGINE_PROTECTED:
    struct _DescProcHasher {
        size_t operator () ( desc_proc_key_t key ) const {
            return key.get().hash_code();
        }
    };

    struct _DescProcEqualer {
        bool operator () ( desc_proc_key_t lhs, desc_proc_key_t rhs ) const {
            return lhs.get() == rhs.get();
        }
    };

    std::unordered_map< desc_proc_key_t, desc_proc_value_t, _DescProcHasher, _DescProcEqualer >   _desc_procs   = {};

_ENGINE_PROTECTED:
    out_stream_t*             _stream       = nullptr;
    desc_proc_value_t         _desc_proc    = {};

    std::mutex                _out_mtx      = {};

    std::set< Echo::Dump* >   _supervisor   = {};

    Echo                      _rt_echo      = { nullptr, 0 };

_ENGINE_PROTECTED:
    friend class Echo;
    friend struct _RtEchoOneLiner;

public:
    template< typename T >
    requires std::is_base_of_v< out_stream_t, T >
    void stream_to( T& stream ) {
        _stream = static_cast< out_stream_t* >( &stream );

        this->set_desc_proc< T >();
    }

    out_stream_t& stream() {
        return *_stream;
    }

    template< typename T >
    void set_desc_proc() {
        auto itr = _desc_procs.find( typeid( T ) );

        if( itr == _desc_procs.end() ) {
            _desc_proc = _desc_procs.at( typeid( nullptr ) );

            return;
        }

        _desc_proc = itr->second;
    }

public:
    void out( const Echo& echo ) {
        auto        view    = echo._acc_str().view();
        const char* p       = view.data();
        size_t      at_desc = 0;
        size_t      pos     = 0;


        std::unique_lock lock{ _out_mtx };

        while( true ) {
            pos = view.find_first_of( Echo::desc_switch, pos );

            if( pos == decltype( view )::npos ) {
                ( *_stream ) << p;
                break;
            }

            const char* q = view.data() + pos++;

            *const_cast< char* >( q ) = '\0';
            ( *_stream ) << p;
            *const_cast< char* >( q ) = Echo::desc_switch;

            std::invoke( _desc_proc, echo._descs().at( at_desc++ ) );

            p = q + 1;
        }

        ( *_stream ) << "\n";
    }

    void raw( const Echo& echo ) {
        std::unique_lock lock{ _out_mtx }; 

        ( *_stream ) << echo._acc_str().view() << std::endl;
    }

public:
    inline _RtEchoOneLiner operator () ( ECHO_LEVEL level = ECHO_LEVEL_INTEL ) {
        return { *this, _rt_echo, this, true, level };
    }

    inline _RtEchoOneLiner operator () ( auto* that, ECHO_LEVEL level = ECHO_LEVEL_INTEL ) {
        return { *this, _rt_echo, that, true, level };
    }

public:
    [[ nodiscard ]] Echo::Dump* new_echo_dump() {
        Echo::Dump* dump = new Echo::Dump{};

        if( dump == nullptr ) {
            throw std::runtime_error{ 
                std::string{ _ENGINE_STR } + "::" + __func__ + ": Echo::Dump bad alloc."
            };

            return nullptr;
        }

        return *_supervisor.emplace( dump ).first;
    }

    void delete_echo_dump( Echo::Dump* dump ) {
        delete dump;
        _supervisor.erase( dump );
    }

_ENGINE_PROTECTED:
    static void _flush( OS::sig_t code );

public:
    decltype( _out_mtx )& mtx() {
        return _out_mtx;
    }

    operator decltype( _out_mtx )& () {
        return this->mtx();
    }

}; inline Comms comms;



template< typename T >
requires is_std_ostringstream_pushable< std::decay_t< T > >
Echo& _RtEchoOneLiner::operator << ( T&& frag ) { return echo.operator<<( std::forward< T >( frag ) ); }



};
