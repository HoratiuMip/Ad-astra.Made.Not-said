#pragma once

#include "descriptor.hpp"
#include "concepts.hpp"



namespace _ENGINE_NAMESPACE {



class Com {
public:
    Com() = default;

_ENGINE_PROTECTED:
    std::ostringstream   _content   = {};

public:
    template< typename T >
    requires is_std_ostringstream_pushable< T >
    Com& operator << ( const T& frag ) {
        _content << frag;

        return *this;
    }

};



class Comms {
public:
    using out_stream_t = std::ostream;

public:
    Comms() = default;

    Comms( out_stream_t& stream )
    : _stream{ &stream }
    {}

_ENGINE_PROTECTED:
    out_stream_t*   _stream    = nullptr;

    std::mutex      _out_mtx   = {};

public:
    Comms& stream_to( out_stream_t& stream ) {
        _stream = &stream; return *this;
    }

public:
    Comms& out( const Com& com ) {
        std::unique_lock< decltype( _out_mtx ) > lock{ _out_mtx };
    }

} comms{ std::cout };



};
