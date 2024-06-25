#pragma once
/*
*/

#include "descriptor.hpp"
#include "concepts.hpp"
#include "os.hpp"



namespace _ENGINE_NAMESPACE {



#define IXT_COMMS_ECHO Echo echo = {}



class Echo {
public:
    friend class Comms;

public:
    using descriptor_t = char;

    using Dump = std::tuple< std::ostringstream, std::vector< descriptor_t > >;

public:
    static constexpr descriptor_t   desc_color_mask   = 0b1111;
    static constexpr char           desc_switch       = '$';

public:
    struct Color {
        OS::CONSOLE_CLR   value   = OS::CONSOLE_CLR_WHITE;
    };

    static Color gray() { return { OS::CONSOLE_CLR_GRAY }; }
    static Color blue() { return { OS::CONSOLE_CLR_BLUE }; }
    static Color green() { return { OS::CONSOLE_CLR_GREEN }; }
    static Color red() { return { OS::CONSOLE_CLR_RED }; }
    static Color pink() { return { OS::CONSOLE_CLR_TURQ }; }
    static Color yellow() { return { OS::CONSOLE_CLR_YELLOW }; }
    static Color white() { return { OS::CONSOLE_CLR_WHITE }; }

public:
    struct LineType {
        Color   color   = { OS::CONSOLE_CLR_TURQ };
    };

    static LineType ok() { return { OS::CONSOLE_CLR_GREEN }; }
    static LineType warning() { return { OS::CONSOLE_CLR_YELLOW }; }
    static LineType error() { return { OS::CONSOLE_CLR_RED }; }
    static LineType pending() { return { OS::CONSOLE_CLR_BLUE }; }
    static LineType intel() { return { OS::CONSOLE_CLR_TURQ }; }

public:
    Echo();

    Echo( const Echo& other )
    : _dump{ other._dump }, _depth{ other._depth + 1 }
    {}

_ENGINE_PROTECTED:
    Echo( Dump* dump )
    : _dump{ dump }
    {}

public:
    ~Echo();

_ENGINE_PROTECTED:
    enum _CONTENT_INDEX {
        _STR, _DESCS
    };

    Dump*    _dump    = nullptr;
    size_t   _depth   = 0;

_ENGINE_PROTECTED:
    inline auto& _str() {
        return std::get< _STR >( *_dump );
    }

    inline auto& _descs() {
        return std::get< _DESCS >( *_dump );
    }

public:
    template< typename T >
    requires is_std_ostringstream_pushable< T >
    Echo& operator << ( const T& frag ) {
        this->_str() << frag;

        return *this;
    }

public:
    Echo& operator << ( const Color& color ) {
        this->_descs().emplace_back( color.value );
        this->_str() << desc_switch;

        return *this;
    }

    Echo& operator << ( const LineType& line_type ) {
        auto type_str = [ &line_type ] () -> const char* {
            switch( line_type.color.value ) {
                case OS::CONSOLE_CLR_GREEN:  return "OK";
                case OS::CONSOLE_CLR_YELLOW: return "WARNING";
                case OS::CONSOLE_CLR_RED:    return "ERROR";
                case OS::CONSOLE_CLR_BLUE:   return "PENDING";
                case OS::CONSOLE_CLR_TURQ:   return "INTEL";
                default: break;
            }
            
            return "UNKNOWN";
        };

        auto type_fill = [ &line_type ] () -> const char* {
            switch( line_type.color.value ) {
                case OS::CONSOLE_CLR_GREEN:  return "     ";
                case OS::CONSOLE_CLR_YELLOW: return "";
                case OS::CONSOLE_CLR_RED:    return "  ";
                case OS::CONSOLE_CLR_BLUE:   return "";
                case OS::CONSOLE_CLR_TURQ:   return "  ";
                default: break;
            }
            
            return "";
        };
        
        this->_str() << '\n';

        this->operator<<( white() )
        << "[ " << line_type.color << type_str() << white() << " ]" << type_fill() << ' ';

        if( _depth != 0 )
            this->operator<<( pink() );
        for( size_t l = 1; l <= _depth; ++l )
            this->_str() << '-';

        this->operator<<( white() );

        return *this;
    }

};



class Comms : public UIdDescriptor {
public:
    using out_stream_t = std::ostream;

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
            OS::console.clr_to( static_cast< OS::CONSOLE_CLR >( desc & Echo::desc_color_mask ) );
        } );

        this->stream_to< T >( stream );


        OS::sig_interceptor.push_on_external_exception( this->uid(), _flush );
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

public:
    template< typename T >
    requires std::is_base_of_v< out_stream_t, T >
    void stream_to( T& stream ) {
        _stream = static_cast< out_stream_t* >( &stream );

        this->set_desc_proc< T >();
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
        if( echo._depth != 0 ) {
            std::unique_lock lock{ OS::console };
            _splash() << "Echo out invoked from depth " << echo._depth << ". Proceeding...\n";
        }

        auto        view    = echo._str().view();
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

        ( *_stream ) << std::endl;
    }

    void raw( const Echo& echo ) {
        std::unique_lock lock{ _out_mtx }; 

        ( *_stream ) << echo._str().view() << std::endl;
    }

public:
    Echo::Dump* new_echo_dump() {
        Echo::Dump* dump = new Echo::Dump{};

        if( dump == nullptr ) {
            _splash() << "Echo dump bad alloc.\n";
            return nullptr;
        }

        return *_supervisor.emplace( dump ).first;
    }

    void delete_echo_dump( Echo::Dump* dump ) {
        _supervisor.erase( dump );
    }

_ENGINE_PROTECTED:
    static std::ostream& _splash() {
        splash() << "[ ";
        OS::console.clr_to( OS::CONSOLE_CLR_RED );
        std::cout << "COMMS";
        OS::console.clr_to( OS::CONSOLE_CLR_WHITE );
        std::cout << " ] ";

        return std::cout;
    }

_ENGINE_PROTECTED:
    static void _flush( OS::SIG code );

} comms;



Echo::Echo()
: _dump{ comms.new_echo_dump() }
{}

Echo::~Echo() {
    if( _depth == 0 )
        if( _dump != nullptr )
            comms.delete_echo_dump( std::exchange( _dump, nullptr ) );
}



void Comms::_flush( OS::SIG code ) {
    for( auto dump : comms._supervisor )
        comms.out( Echo{ dump } );
}



};
