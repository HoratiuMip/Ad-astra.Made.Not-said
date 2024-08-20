#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/file-manip.hpp>

namespace _ENGINE_NAMESPACE {



class Env {
public:
    static int width() {
        static int value = ( [] () -> int {
            RECT rect;
            GetWindowRect( GetDesktopWindow(), &rect );

            return rect.right;
        } )();

        return value;
    }

    template< auto mul > requires std::integral< decltype( mul ) >
    static int w() {
        return width() * mul;
    }

    template< auto div > requires std::floating_point< decltype( div ) >
    static ggfloat_t w() {
        return width() / div;
    }

    static int height() {
        static int value = ( [] () -> int {
            RECT rect;
            GetWindowRect( GetDesktopWindow(), &rect );

            return rect.bottom;
        } )();

        return value;
    }

    template< auto mul > requires std::integral< decltype( mul ) >
    static int h() {
        return height() * mul;
    }

    template< auto div > requires std::floating_point< decltype( div ) >
    static ggfloat_t h() {
        return height() / div;
    }

public:
    static ggfloat_t diagon() {
        static ggfloat_t value = sqrt( width() * width() + height() * height() );

        return value;
    }

    template< auto mul > requires std::integral< decltype( mul ) >
    static ggfloat_t d() {
        return diagon() * mul;
    }

    template< auto div > requires std::floating_point< decltype( div ) >
    static ggfloat_t d() {
        return diagon() / div;
    }

public:
    static ggfloat_t aspect() {
        return static_cast< ggfloat_t >( width() ) / height();
    }

public:
    static std::string_view dir() {
        static std::string value = ( [] () -> std::string {
            char path[ MAX_PATH ];

            GetModuleFileNameA( GetModuleHandle( NULL ), path, MAX_PATH );

            return File::dir_of( path );
        } )();

        return value;
    }

    static std::string_view process() {
        static std::string value = ( [] () -> std::string {
            char path[ MAX_PATH ];

            GetModuleFileNameA( GetModuleHandle( NULL ), path, MAX_PATH );

            return File::name_of( path );
        } )();

        return value;
    }

};



};
