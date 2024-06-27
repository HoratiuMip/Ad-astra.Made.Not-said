#ifndef _ENGINE_DESCRIPTOR_HPP
#define _ENGINE_DESCRIPTOR_HPP
/*
-----------------------------------------
-   Descriptor
-       Home of all defines, typedefs and includes used by the engine. 
-
-----------------------------------------
-   [[ Pre-include defines ]] 
-       Definitions for altering the behaviour of the engine.
-
-       "[ DEFINITION CLASS ]*" - the '*' means that one of the following shall be defined.
-       "DEFINITION*" - the '*' means that this definition is mandatory.
-             

-       [ Constants ]
-           PI
-           e
-       
-       [ General behaviour ]
-           IXT_ALL_PUBLIC - all class data/functions are public.
-
-       [ Target OS ]*
-           IXT_OS_WINDOWS
-
-       [ Target GL ]*
-           IXT_GL_DIRECT_2D1
-
-       [ Geometry and Graphics ] - floating point precision, 
-                                   defaults to "float", single precision,
-                                   typedef'd as "ggfloat_t".
-           IXT_GG_FLOAT_LONG_DOUBLE
-           IXT_GG_FLOAT_DOUBLE
-
-----------------------------------------
*/



#pragma region Defines



#if !defined( PI )
    #define PI 3.141592653
#endif

#if !defined( EUL )
    #define EUL 2.7182818284
#endif



#define _ENGINE_NAMESPACE IXT
#define _ENGINE_STR       "IXT"

#define _ENGINE_STRUCT_NAME( name ) _ENGINE_STR "::" name
#define _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( name ) virtual std::string_view struct_name() const override { return _ENGINE_STRUCT_NAME( name ); }



#if defined( IXT_ALL_PUBLIC )
    #define _ENGINE_PROTECTED public
#else
    #define _ENGINE_PROTECTED protected
#endif

#if defined( IXT_OS_WINDOWS )
    #define _ENGINE_OS_WINDOWS
#else
    #error Define target operating system.
#endif

#if defined( IXT_GL_DIRECT_2D1 )
    #define _ENGINE_GL_DIRECT_2D1
#elif defined( IXT_GL_NONE )
    #define _ENGINE_GL_NONE
#else
    #error Define target graphics library.
#endif

#if defined( IXT_GG_FLOAT_LONG_DOUBLE )
    #define _ENGINE_GG_FLOAT_TYPE long double
#elif defined( IXT_GG_FLOAT_DOUBLE )
    #define _ENGINE_GG_FLOAT_TYPE double
#else
    #define _ENGINE_GG_FLOAT_TYPE float
#endif




#define _ENGINE_ECHO_ADD_PREFIX( type ) "IXT::" type

#define _ENGINE_ECHO_IDENTIFY_METHOD( type ) virtual std::string_view echo_name() const override { return _ENGINE_ECHO_ADD_PREFIX( type ); }

#define _ENGINE_ECHO_DFD_ARG Echo echo = {}



#if defined( IXT_ECHO )
        #define _ENGINE_ECHO
#endif

#if defined( IXT_UNIQUE_SURFACE )
    #define _ENGINE_UNIQUE_SURFACE
#endif

#if defined( IXT_THROW_ON_FAULT )
    #define ENGINE_THROW_ON_FAULT
#endif



#pragma endregion Defines



#pragma region Typedefs



namespace _ENGINE_NAMESPACE {
    typedef   _ENGINE_GG_FLOAT_TYPE    ggfloat_t;
};



#pragma endregion Typedefs



#pragma region Includes



#include <iostream>
#include <fstream>
#include <sstream>

#include <filesystem>

#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <map>
#include <unordered_map>
#include <set>

#include <algorithm>
#include <utility>
#include <cmath>

#include <functional>
#include <concepts>
#include <optional>
#include <tuple>
#include <variant>
#include <bitset>
#include <string>
#include <string_view>

#include <memory>
#include <thread>
#include <chrono>
#include <future>
#include <semaphore>
#include <atomic>
#include <condition_variable>

#include <type_traits>
#include <typeindex>
#include <typeinfo>

#if defined( _ENGINE_OS_WINDOWS )
    #include <windows.h>
    #include <wincodec.h>
#endif

#if defined( _ENGINE_GL_DIRECT_2D1 )
    #include <d2d1.h>
#endif



#pragma endregion Includes



#pragma region UId



namespace _ENGINE_NAMESPACE {
    typedef   void*   UId;

    class Descriptor {
    public:
        UId uid() const {
            return ( void* )( this );
        }

    public:
        virtual std::string_view struct_name() const {
            return _ENGINE_STRUCT_NAME( "Descriptor" );
        }

    };
};



#pragma endregion UId



#pragma region UTH



namespace _ENGINE_NAMESPACE {
    class UTH : public Descriptor {
    public:
        

    };
};



#pragma endregion UTH



#endif
