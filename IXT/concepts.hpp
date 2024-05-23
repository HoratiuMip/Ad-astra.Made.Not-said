#pragma once
/*
-----------------------------------------
-   Concepts
-
-----------------------------------------
-   [ STD ]
-       is_std_ostringstream_pushable - "some_std_ostringstream << some_type" is valid code.
-
-----------------------------------------
*/



#include "descriptor.hpp"



namespace _ENGINE_NAMESPACE {



template< typename T > concept is_std_ostringstream_pushable = requires {
    std::ostringstream{} << T{};
};



};