#pragma once
/*
*/

#include <IXT/descriptor.hpp>



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

struct ConsoleCursor {
    short   row   = 0;
    short   col   = 0;
};

class Console {
public:
    Console()
    : _h_std_out{ GetStdHandle( STD_OUTPUT_HANDLE ) }
    {}

public:
    Console& clr_with( CONSOLE_CLR clr ) {
        SetConsoleTextAttribute( _h_std_out, clr );
        return *this;
    } 

    Console& crs_at( ConsoleCursor crd ) {
        SetConsoleCursorPosition( _h_std_out, COORD{ crd.col, crd.row } );
        return *this;
    }

    ConsoleCursor crs() const {
        CONSOLE_SCREEN_BUFFER_INFO  cinf{};
        GetConsoleScreenBufferInfo( _h_std_out, &cinf );

        return { cinf.dwCursorPosition.Y, cinf.dwCursorPosition.X };
    }

_ENGINE_PROTECTED:
    HANDLE   _h_std_out   = nullptr;

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
    SIG_SEGFAULT  = SIGSEGV, 
    SIG_TERMINATE = SIGTERM
};
typedef   int   sig_t;

class SigInterceptor : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "OS::SigInterceptor" );

_ENGINE_PROTECTED:
    inline static constexpr const sig_t   _codes[]   = {
        SIG_ABORT, SIG_FLOAT, SIG_ILLEGAL, SIG_INTERRUPT, SIG_SEGFAULT, SIG_TERMINATE
    };

    inline static std::map< sig_t, const char* >   _codes_strs   = {
        { SIG_ABORT, "SIG_ABORT" },
        { SIG_FLOAT, "SIG_FLOAT" },
        { SIG_ILLEGAL, "SIG_ILLEGAL" },
        { SIG_INTERRUPT, "SIG_INTERRUPT" },
        { SIG_SEGFAULT, "SIG_SEGFAULT" },
        { SIG_TERMINATE, "SIG_TERMINATE" }
    };

public:
    SigInterceptor() {
        for( const auto& code : _codes )
            signal( code, _callback_proc );
    }

_ENGINE_PROTECTED:
    using cbmap_key_t   = XtDx;
    using cbmap_value_t = std::function< void( sig_t ) >;

_ENGINE_PROTECTED:
    std::map< cbmap_key_t, cbmap_value_t >   _callbacks   = {};

public:
    void push_on_external_exception( const cbmap_key_t& xtdx, const cbmap_value_t& callback ) {
        _callbacks.insert( std::make_pair( xtdx, callback ) );
    }

    void pop_on_external_exception( const cbmap_key_t& xtdx ) {
        _callbacks.erase( xtdx );
    }

_ENGINE_PROTECTED:
    static void _callback_proc( sig_t code );

}; inline SigInterceptor sig_interceptor;



inline std::string file_browse_save( std::string_view browser_title ) {
    static constexpr int PATH_BUF_SZ = MAX_PATH;

    char         path[ PATH_BUF_SZ ];
    OPENFILENAME hf;

    memset( &path, 0, sizeof( path ) );
    memset( ( char* )&hf, 0, sizeof( hf ) );
    
    hf.lStructSize = sizeof( hf );
    hf.hwndOwner   = GetFocus();
    hf.lpstrFile   = path;
    hf.nMaxFile    = MAX_PATH;
    hf.lpstrTitle  = browser_title.data();
    hf.Flags       = OFN_EXPLORER | OFN_NOCHANGEDIR;

    GetSaveFileName( &hf );

    return path;
}

#endif



}; };

