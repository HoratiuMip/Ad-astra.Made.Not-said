#ifndef _ENGINE_OS_HPP
#define _ENGINE_OS_HPP
/*
*/

#include "descriptor.hpp"



namespace _ENGINE_NAMESPACE { namespace OS {



#if defined( _ENGINE_OS_WINDOWS )

enum CONSOLE_CLR : char {
    CONSOLE_CLR_GRAY   = 8,
    CONSOLE_CLR_BLUE   = 9,
    CONSOLE_CLR_GREEN  = 10,
    CONSOLE_CLR_TURQ   = 11,
    CONSOLE_CLR_RED    = 12,
    CONSOLE_CLR_PINK   = 13,
    CONSOLE_CLR_YELLOW = 14,
    CONSOLE_CLR_WHITE  = 15
};

class Console {
public:
    void clr_to( CONSOLE_CLR clr ) {
        static auto handle = GetStdHandle( STD_OUTPUT_HANDLE );

        SetConsoleTextAttribute( handle, clr );
    } 

public:
    std::recursive_mutex   mtx   = {};

public:
    inline operator decltype( mtx )& () {
        return mtx;
    }

}; inline Console console;



class IPPipe {
public:
    IPPipe() = default;

};



enum SIG : int {
    SIG_ABORT     = SIGABRT, 
    SIG_FLOAT     = SIGFPE, 
    SIG_ILLEGAL   = SIGILL, 
    SIG_INTERRUPT = SIGINT, 
    SIG_MEMORY    = SIGSEGV, 
    SIG_TERMINATE = SIGTERM
};

class SigInterceptor : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "OS::SigInterceptor" );

_ENGINE_PROTECTED:
    inline static constexpr const SIG   _codes[]   = {
        SIG_ABORT, SIG_FLOAT, SIG_ILLEGAL, SIG_INTERRUPT, SIG_MEMORY, SIG_TERMINATE
    };

    inline static std::map< SIG, const char* >   _codes_strs   = {
        { SIG_ABORT, "SIG_ABORT" },
        { SIG_FLOAT, "SIG_FLOAT" },
        { SIG_ILLEGAL, "SIG_ILLEGAL" },
        { SIG_INTERRUPT, "SIG_INTERRUPT" },
        { SIG_MEMORY, "SIG_MEMORY" },
        { SIG_TERMINATE, "SIG_TERMINATE" }
    };

public:
    SigInterceptor() {
        for( const auto& code : _codes )
            signal( code, _callback_proc );
    }

_ENGINE_PROTECTED:
    using cbmap_key_t   = UId;
    using cbmap_value_t = std::function< void( SIG ) >;

_ENGINE_PROTECTED:
    std::map< cbmap_key_t, cbmap_value_t >   _callbacks   = {};

public:
    void push_on_external_exception( const cbmap_key_t& uid, const cbmap_value_t& callback ) {
        _callbacks.insert( std::make_pair( uid, callback ) );
    }

    void pop_on_external_exception( const cbmap_key_t& uid ) {
        _callbacks.erase( uid );
    }

_ENGINE_PROTECTED:
    static void _callback_proc( SIG code );

}; inline SigInterceptor sig_interceptor;

#endif



}; };



#endif
