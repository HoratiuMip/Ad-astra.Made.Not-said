#pragma once
/*
*/



#include "descriptor.hpp"



enum CONSOLE_TEXT_COLOR : char {
    #if defined( _ENGINE_OS_WINDOWS )
        CONSOLE_TEXT_COLOR_GRAY   = 8,
        CONSOLE_TEXT_COLOR_BLUE   = 9,
        CONSOLE_TEXT_COLOR_GREEN  = 10,
        CONSOLE_TEXT_COLOR_RED    = 12,
        CONSOLE_TEXT_COLOR_PINK   = 13,
        CONSOLE_TEXT_COLOR_YELLOW = 14,
        CONSOLE_TEXT_COLOR_WHITE  = 15
    #endif
};

void console_text_color_to( CONSOLE_TEXT_COLOR color ) {
    #if defined( _ENGINE_OS_WINDOWS )
        static auto handle = GetStdHandle( STD_OUTPUT_HANDLE );

        SetConsoleTextAttribute( handle, color );
    #endif
}
