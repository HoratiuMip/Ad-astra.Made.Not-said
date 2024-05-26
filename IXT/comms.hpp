#pragma once

#include "descriptor.hpp"
#include "concepts.hpp"
#include "console.hpp"



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
        CONSOLE_TEXT_COLOR   value   = CONSOLE_TEXT_COLOR_WHITE;
    };

    static Color gray() { return { CONSOLE_TEXT_COLOR_GRAY }; }
    static Color blue() { return { CONSOLE_TEXT_COLOR_BLUE }; }
    static Color green() { return { CONSOLE_TEXT_COLOR_GREEN }; }
    static Color red() { return { CONSOLE_TEXT_COLOR_RED }; }
    static Color pink() { return { CONSOLE_TEXT_COLOR_PINK }; }
    static Color yellow() { return { CONSOLE_TEXT_COLOR_YELLOW }; }
    static Color white() { return { CONSOLE_TEXT_COLOR_WHITE }; }

public:
    struct LineType {
        Color   color   = { CONSOLE_TEXT_COLOR_PINK };
    };

    static LineType ok() { return { CONSOLE_TEXT_COLOR_GREEN }; }
    static LineType warning() { return { CONSOLE_TEXT_COLOR_YELLOW }; }
    static LineType error() { return { CONSOLE_TEXT_COLOR_RED }; }
    static LineType pending() { return { CONSOLE_TEXT_COLOR_BLUE }; }
    static LineType intel() { return { CONSOLE_TEXT_COLOR_PINK }; }

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
                case CONSOLE_TEXT_COLOR_GREEN:  return "OK";
                case CONSOLE_TEXT_COLOR_YELLOW: return "WARNING";
                case CONSOLE_TEXT_COLOR_RED:    return "ERROR";
                case CONSOLE_TEXT_COLOR_BLUE:   return "PENDING";
                case CONSOLE_TEXT_COLOR_PINK:   return "INTEL";
            }
            
            return "UNKNOWN";
        };

        auto type_fill = [ &line_type ] () -> const char* {
            switch( line_type.color.value ) {
                case CONSOLE_TEXT_COLOR_GREEN:  return "     ";
                case CONSOLE_TEXT_COLOR_YELLOW: return "";
                case CONSOLE_TEXT_COLOR_RED:    return "  ";
                case CONSOLE_TEXT_COLOR_BLUE:   return "";
                case CONSOLE_TEXT_COLOR_PINK:   return "  ";
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
                console_text_color_to( desc & Log::desc_color_mask );
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
            console_text_color_to( CONSOLE_TEXT_COLOR_WHITE );
        }
    }

    void raw( const Log& log ) {
        std::unique_lock< decltype( _out_mtx ) > lock{ _out_mtx }; 

        ( *_stream ) << log._str().view() << std::endl;
    }

}; std::unique_ptr< Comms > comms{ new Comms{} };



};
