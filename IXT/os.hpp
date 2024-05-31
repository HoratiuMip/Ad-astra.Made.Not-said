#pragma once
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
    static void clr_to( CONSOLE_CLR clr ) {
        static auto handle = GetStdHandle( STD_OUTPUT_HANDLE );

        SetConsoleTextAttribute( handle, clr );
    } 

};



class IPPipe {
public:
    IPPipe() = default;

};

#endif



}; };
