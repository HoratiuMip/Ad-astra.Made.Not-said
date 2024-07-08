#pragma once
/*
*/

#include <IXT/descriptor.hpp>



namespace _ENGINE_NAMESPACE {



template< typename T > concept is_std_ostringstream_pushable = requires {
    std::ostringstream{} << T{};
};



};
