#pragma once
/*
*/

#include <IXT/descriptor.hpp>



namespace _ENGINE_NAMESPACE {



template< typename T > concept is_std_ostringstream_pushable = requires{
    std::ostringstream{} << T{};
};


template< typename T > concept is_descriptor_tolerant = 
    std::is_base_of_v< Descriptor, std::remove_pointer_t< std::remove_cv_t< T > > >;



template< bool C, typename T >
struct volatile_attr {
    using type = std::conditional_t< C, T, std::monostate >;
};
template< bool C, typename T >
using volatile_attr_t = typename volatile_attr< C, T >::type;



};
