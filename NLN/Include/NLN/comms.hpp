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



enum EchoLevel_  : char {
    EchoLevel_Input = 0,
    EchoLevel_Trace,
    EchoLevel_Debug,
    EchoLevel_Error,
    EchoLevel_Warning,
    EchoLevel_Pending,
    EchoLevel_Ok,
    EchoLevel_Info
};

class Echo;

struct _BackwardCompatibility_EchoOneLiner {
    _BackwardCompatibility_EchoOneLiner( Echo* echo, const Descriptor& invoker, EchoLevel_ level, bool is_critical = false );
    ~_BackwardCompatibility_EchoOneLiner();

    Echo*   _echo;

    template< typename T > _BackwardCompatibility_EchoOneLiner& operator << ( T&& frag );
};

class Echo {
public:
    friend class Comms;

public:
    using out_stream_t = std::ostream;

_ENGINE_PROTECTED:
    inline static OS::CONSOLE_CLR _level_colors[] = {
        OS::CONSOLE_CLR_PINK,   /* EchoLevel_Input */ 
        OS::CONSOLE_CLR_PINK,   /* EchoLevel_Trace */
        OS::CONSOLE_CLR_GRAY,   /* EchoLevel_Debug */
        OS::CONSOLE_CLR_RED,    /* EchoLevel_Error */
        OS::CONSOLE_CLR_YELLOW, /* EchoLevel_Warning */
        OS::CONSOLE_CLR_BLUE,   /* EchoLevel_Pending */
        OS::CONSOLE_CLR_GREEN,  /* EchoLevel_Ok */
        OS::CONSOLE_CLR_TURQ,   /* EchoLevel_Info */
    };

    inline static const char* _level_strings[] = {
        "Input", "Trace", "Debug", "Error", "Warning", "Pending", "Ok", "Info"
    };

public:
    Echo() = default;

    Echo( const Echo& other )
    : _depth{ other._depth + 1 }
    {}

_ENGINE_PROTECTED:
    int   _depth   = 0;

_ENGINE_PROTECTED:
    inline static struct _UnknownInvoker : public Descriptor {
        _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "?" );
    } _unknown_invoker_placeholder;

/* VVV === For backward compatibility. === VVV */
public:
    Echo( decltype( nullptr ) ) {};

public:
    _BackwardCompatibility_EchoOneLiner operator () ( const Descriptor& desc, EchoLevel_ level ) {
        return { this, desc, level };
    }

    _BackwardCompatibility_EchoOneLiner operator() ( const Descriptor* desc, EchoLevel_ level ) {
        return ( *this )( *desc, level );
    }

    template< typename T > requires( !is_descriptor_derived< T > )
    _BackwardCompatibility_EchoOneLiner operator() ( const T& invoker, EchoLevel_ level ) {
        return ( *this )( _unknown_invoker_placeholder, level );
    }

    template< typename T > requires( !is_descriptor_derived< T > )
    _BackwardCompatibility_EchoOneLiner operator() ( const T* invoker, EchoLevel_ level ) {
        return ( *this )( *invoker, level );
    }

public:
    template< typename T > Echo& operator << ( T&& frag );

};



class Comms : public Descriptor, public std::mutex {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Comms" );

public:
    friend class Echo;
    friend struct _BackwardCompatibility_EchoOneLiner;

public:
    using out_stream_t = Echo::out_stream_t;

public:
    Comms() : Comms{ std::cout } {}

    template< typename T > requires std::is_base_of_v< out_stream_t, T > 
    Comms( T& stream ) {
        this->stream_to< T >( stream );
    }

_ENGINE_PROTECTED:
    out_stream_t*             _stream       = nullptr;
    bool                      _do_color     = false;
    Echo                      _echo         = {};

public:
    template< typename T > requires std::is_base_of_v< out_stream_t, T >
    void stream_to( T& stream ) {
        _stream = ( out_stream_t* )&stream;
        _do_color = dynamic_cast< decltype( std::cout )* >( _stream ) != nullptr;
    }

    out_stream_t& stream() {
        return *_stream;
    }

_ENGINE_PROTECTED:
    void _color( OS::CONSOLE_CLR clr ) { if( _do_color ) OS::console.clr_with( clr ); }

    void _pink()   { if( _do_color ) OS::console.clr_with( OS::CONSOLE_CLR_PINK ); }
    void _gray()   { if( _do_color ) OS::console.clr_with( OS::CONSOLE_CLR_GRAY ); }
    void _red()    { if( _do_color ) OS::console.clr_with( OS::CONSOLE_CLR_RED ); }
    void _yellow() { if( _do_color ) OS::console.clr_with( OS::CONSOLE_CLR_YELLOW ); }
    void _blue()   { if( _do_color ) OS::console.clr_with( OS::CONSOLE_CLR_BLUE ); }
    void _green()  { if( _do_color ) OS::console.clr_with( OS::CONSOLE_CLR_GREEN ); }
    void _turq()   { if( _do_color ) OS::console.clr_with( OS::CONSOLE_CLR_TURQ ); }
    void _white()  { if( _do_color ) OS::console.clr_with( OS::CONSOLE_CLR_WHITE ); }

public:
    void splash( const Descriptor& desc, EchoLevel_ level, Echo* echo ) {
        ( *_stream ) << '\n';
        this->_white();
        ( *_stream ) << "[ ";
        this->_color( Echo::_level_colors[ level ] ); ( *_stream ) << Echo::_level_strings[ level ];
        this->_white();
        ( *_stream ) <<  " ]   \t";
        this->_white();
        ( *_stream ) << "[ ";
        this->_gray();
        ( *_stream ) << time( nullptr );
        this->_white();
        ( *_stream ) << " ]";
        this->_blue();
        if( echo != nullptr ) ( *_stream ) << echo->_depth << '>';
        this->_white();
        ( *_stream ) <<  "[ ";
        this->_gray();
        const char* struct_name = desc.struct_name(); ( *_stream ) << struct_name ? struct_name : "?Null";
        this->_white();
        ( *_stream ) <<  " ][ ";
        this->_gray();
        ( *_stream ) << desc.xtdx();
        this->_white();
        ( *_stream ) <<  " ]";
        this->_blue();
        ( *_stream ) <<  " -> ";
        this->_white();
    }

    void splash_critical( const Descriptor& desc ) {
        ( *_stream ) << '\n';
        this->_white();
        ( *_stream ) << "[ ";
        this->_red();
        const char* struct_name = desc.struct_name(); ( *_stream ) << struct_name ? struct_name : "?Null";
        this->_white();
        ( *_stream ) << " ][ ";
        this->_red();
        ( *_stream ) << desc.xtdx();
        this->_white();
        ( *_stream ) << " ]";
        this->_red();
        ( *_stream ) << " -> ";
        this->_white();
    }

/* VVV === For backward compatibility. === VVV */
public:
    _BackwardCompatibility_EchoOneLiner operator () ( EchoLevel_ level = EchoLevel_Info ) {
        return { nullptr, *this, level };
    }

    _BackwardCompatibility_EchoOneLiner operator () ( const Descriptor& desc, EchoLevel_ level = EchoLevel_Info ) {
        return { nullptr, desc, level };
    }

    _BackwardCompatibility_EchoOneLiner operator () ( const Descriptor* desc, EchoLevel_ level = EchoLevel_Info ) {
        return { nullptr, *desc, level };
    }

    _BackwardCompatibility_EchoOneLiner operator [] ( const Descriptor& desc ) {
        return { nullptr, desc, EchoLevel_Trace, true };
    }

    _BackwardCompatibility_EchoOneLiner operator [] ( const Descriptor* desc ) {
        return { nullptr, *desc, EchoLevel_Trace, true };
    }

}; inline Comms comms;



template< typename T > Echo& Echo::operator << ( T&& frag ) {
    ( *comms._stream ) << std::forward< T >( frag ); 
    return *this;
}
template< typename T > _BackwardCompatibility_EchoOneLiner& _BackwardCompatibility_EchoOneLiner::operator << ( T&& frag ) {
    ( *comms._stream ) << std::forward< T >( frag ); 
    return *this;
}



};
