#pragma once
/*
*/

#include <NLN/descriptor.hpp>
#include <NLN/file-manip.hpp>

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

    static ggfloat_t w( ggfloat_t mul ) {
        return width() * mul;
    }

    static int height() {
        static int value = ( [] () -> int {
            RECT rect;
            GetWindowRect( GetDesktopWindow(), &rect );

            return rect.bottom;
        } )();

        return value;
    }

    static ggfloat_t h( ggfloat_t mul ) {
        return height() * mul;
    }

public:
    static ggfloat_t diagonal() {
        static ggfloat_t value = sqrt( width() * width() + height() * height() );

        return value;
    }

    static ggfloat_t d( ggfloat_t mul ) {
        return diagonal() * mul;
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
