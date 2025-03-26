#pragma once
/*====== IXT-NLN Engine - OS - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> The component of the engine which attempts to make an abstraction of the underlying operating system.
|
======*/

#include <IXN/descriptor.hpp>



namespace _ENGINE_NAMESPACE { namespace OS {



#if defined( _ENGINE_OS_WINDOWS )

enum ConsoleColor_ : char {
    ConsoleColor_Gray   = 8,
    ConsoleColor_Blue   = 9,
    ConsoleColor_Green  = 10,
    ConsoleColor_Turq   = 11,
    ConsoleColor_Red    = 12,
    ConsoleColor_Pink   = 13,
    ConsoleColor_Yellow = 14,
    ConsoleColor_White  = 15
};

struct ConsoleCursor {
    short row;
    short col;
};

class Console {
public:
    Console()
    : _h_std_out{ GetStdHandle( STD_OUTPUT_HANDLE ) }
    {}

public:
    Console& color_with( ConsoleColor_ color ) {
        SetConsoleTextAttribute( _h_std_out, color );
        return *this;
    } 

    Console& cursor_at( ConsoleCursor crs ) {
        SetConsoleCursorPosition( _h_std_out, COORD{ crs.col, crs.row } );
        return *this;
    }

    ConsoleCursor cursor() const {
        CONSOLE_SCREEN_BUFFER_INFO cinf{};
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



enum Signal_ : int32_t {
    Signal_Abort       = SIGABRT, 
    Signal_Float       = SIGFPE, 
    Signal_Illegal     = SIGILL, 
    Signal_Interrupt   = SIGINT, 
    Signal_Memory      = SIGSEGV, 
    Signal_Terminate   = SIGTERM,
    Signal_Break       = SIGBREAK,
    Signal_AbortCompat = SIGABRT_COMPAT
};

class SignalIntercept : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "OS::SignalIntercept" );

_ENGINE_PROTECTED:
    inline static constexpr const Signal_   _codes[]   = {
        Signal_Abort, Signal_Float, Signal_Illegal, Signal_Interrupt, Signal_Memory, Signal_Terminate, Signal_Break, Signal_AbortCompat 
    };

    inline static std::map< Signal_, const char* >   _codes_strings   = {
        { Signal_Abort, "Signal_Abort" },
        { Signal_Float, "Signal_Float" },
        { Signal_Illegal, "Signal_Illegal" },
        { Signal_Interrupt, "Signal_Interrupt" },
        { Signal_Memory, "Signal_Memory" },
        { Signal_Terminate, "Signal_Terminate" },
        { Signal_Break, "Signal_Break" },
        { Signal_AbortCompat, "Signal_AbortCompat" }
    };

public:
    SignalIntercept() {
        for( const auto& code : _codes ) signal( code, ( __p_sig_fn_t )_callback_proc );
    }

_ENGINE_PROTECTED:
    using cbmap_key_t   = XtDx;
    using cbmap_value_t = std::function< void( Signal_ ) >;

_ENGINE_PROTECTED:
    std::map< cbmap_key_t, cbmap_value_t >   _callbacks   = {};

public:
    void push_on_signal( const cbmap_key_t& xtdx, const cbmap_value_t& callback ) {
        _callbacks.insert( std::make_pair( xtdx, callback ) );
    }

    void pop_on_signal( const cbmap_key_t& xtdx ) {
        _callbacks.erase( xtdx );
    }

_ENGINE_PROTECTED:
    static void _callback_proc( Signal_ code );

}; inline SignalIntercept signal_intercept;



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

