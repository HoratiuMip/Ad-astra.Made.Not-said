#pragma once
/*
*/

#include "descriptor.hpp"
#include "concepts.hpp"
#include "os.hpp"



namespace _ENGINE_NAMESPACE {



#define IXT_COMMS_LOG Log log = {}



class Log {
public:
    friend class Comms;

public:
    using descriptor_t = char;
    using Content      = std::tuple< std::ostringstream, std::vector< descriptor_t > >;

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
    Log()
    : _content{ new Content{} }
    {}

    Log( const Log& other )
    : _content{ other._content }, _depth{ other._depth + 1 }
    {}

public:
    ~Log() {
        if( _depth == 0 )
            if( _content != nullptr )
                delete _content;
    }

_ENGINE_PROTECTED:
    enum _CONTENT_INDEX {
        _STR, _DESCS
    };

    Content*   _content   = nullptr;
    size_t     _depth     = 0;

_ENGINE_PROTECTED:
    inline auto& _str() {
        return std::get< _STR >( *_content );
    }

    inline auto& _descs() {
        return std::get< _DESCS >( *_content );
    }

public:
    template< typename T >
    requires is_std_ostringstream_pushable< T >
    Log& operator << ( const T& frag ) {
        this->_str() << frag;

        return *this;
    }

public:
    Log& operator << ( const Color& color ) {
        this->_descs().emplace_back( color.value );
        this->_str() << desc_switch;

        return *this;
    }

    Log& operator << ( const LineType& line_type ) {
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
        << "[ " << line_type.color << type_str() << white() << " ]" << type_fill() << '\t';

        if( _depth != 0 )
            this->operator<<( pink() );
        for( size_t l = 1; l <= _depth; ++l )
            this->_str() << '-';

        this->operator<<( white() );

        return *this;
    }

};



class Comms {
public:
    using out_stream_t = std::ostream;

public:
    Comms() = default;

    Comms( out_stream_t& stream )
    : _stream{ &stream }
    {}

_ENGINE_PROTECTED:
    out_stream_t*   _stream    = nullptr;
    bool            _sup_clr   = false;

    std::mutex      _out_mtx   = {};

public:
    void stream_to( out_stream_t& stream ) {
        _stream = &stream;

        _sup_clr = dynamic_cast< decltype( std::cout )* >( _stream );
    }

public:
    void out( const Log& log ) {
        if( log._depth != 0 ) return;

        auto        view    = log._str().view();
        const char* p       = view.data();
        size_t      at_desc = 0;
        size_t      pos     = 0;

        auto switch_description = [ this, &log, &at_desc ] () -> void { 
            auto desc = log._descs().at( at_desc++ ); 

            if( _sup_clr ) {
                OS::Console::clr_to( desc & Log::desc_color_mask );
            }
        };

        std::unique_lock< decltype( _out_mtx ) > lock{ _out_mtx };

        while( true ) {
            pos = view.find_first_of( Log::desc_switch, pos );

            if( pos == decltype( view )::npos ) {
                ( *_stream ) << p;
                break;
            }

            const char* q = view.data() + pos++;

            *const_cast< char* >( q ) = '\0';
            ( *_stream ) << p;
            *const_cast< char* >( q ) = Log::desc_switch;

            switch_description();
            p = q + 1;
        }

        ( *_stream ) << std::endl;

        if( _sup_clr ) {
            OS::Console::clr_to( OS::CONSOLE_CLR_WHITE );
        }
    }

    void raw( const Log& log ) {
        std::unique_lock< decltype( _out_mtx ) > lock{ _out_mtx }; 

        ( *_stream ) << log._str().view() << std::endl;
    }

}; std::unique_ptr< Comms > comms{ new Comms{} };



};
