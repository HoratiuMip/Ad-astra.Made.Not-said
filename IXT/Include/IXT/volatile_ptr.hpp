#pragma once
/*
[] CAUTION - SHALL BE USED ONLY BY PROFESSIONALS.
*/

#include <IXT/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



struct enslave_t {};

template< typename _T, bool _is_array = std::is_array_v< _T > >
class VolatilePtr : public SPtr< _T > {
public:
    using Base = SPtr< _T >;

public:
    using T = std::remove_pointer_t< std::decay_t< _T > >;

public:
    VolatilePtr() = default;

    VolatilePtr( std::nullptr_t )
    : Base{ nullptr }
    {}

    VolatilePtr( T* raw_ptr ) noexcept
    : Base{ raw_ptr, [] ( [[maybe_unused]] T* ) -> void {} }
    {}

    VolatilePtr( T* raw_ptr, [[maybe_unused]] enslave_t )
    : Base{ raw_ptr }
    {}

    VolatilePtr( T& raw_ref ) noexcept
    : VolatilePtr{ &raw_ref }
    {}

    VolatilePtr( T&& raw_move_ref ) noexcept
    : Base{ std::make_shared< T >( std::move( raw_move_ref ) ) }
    {}

    VolatilePtr( const SPtr< T >& other ) noexcept
    : Base{ other }
    {}

    VolatilePtr( SPtr< T >&& other ) noexcept
    : Base{ std::move( other ) }
    {}

public:
    T* operator + ( ptrdiff_t diff ) const {
        return this->get() + diff;
    }

    T* operator - ( ptrdiff_t diff ) const {
        return this->get() - diff;
    }

    T& operator [] ( ptrdiff_t idx ) {
        return *( this->get() + idx );
    }

    const T& operator [] ( ptrdiff_t idx ) const {
        return *( this->get() + idx );
    }

public:
    operator T* () const {
        return this->get();
    }

    operator const T* () const {
        return this->get();
    }

    operator T& () {
        return *this->get();
    }

    operator const T& () const {
        return *this->get();
    }

};

template< typename T >
using VPtr = VolatilePtr< T >;



};
