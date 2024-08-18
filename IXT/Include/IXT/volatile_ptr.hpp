#pragma once
/*
[] CAUTION - SHALL BE USED ONLY BY PROFESSIONALS.
*/

#include <IXT/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



struct strong_link_t {};
struct weak_link_t {};

template< typename _T, bool _is_array = std::is_array_v< _T > >
class VolatilePtr : public SPtr< _T > {
public:
    using Base = SPtr< _T >;

public:
    using T = std::remove_pointer_t< std::decay_t< _T > >;

public:
    VolatilePtr() = default;

    VolatilePtr( const VolatilePtr& other ) = default;

    VolatilePtr( VolatilePtr&& other ) = default;

    VolatilePtr( std::nullptr_t )
    : Base{ nullptr }
    {}

    VolatilePtr( T* raw_ptr, [[maybe_unused]] weak_link_t ) noexcept
    : Base{ raw_ptr, [] ( [[maybe_unused]] T* ) -> void {} }
    {}

    VolatilePtr( T* raw_ptr, [[maybe_unused]] strong_link_t )
    : Base{ raw_ptr }
    {}

    VolatilePtr( T& raw_ref ) noexcept
    : VolatilePtr{ &raw_ref, weak_link_t{} }
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

public:
    auto& reset( T* raw_ptr, [[maybe_unused]] weak_link_t ) {
        this->Base::reset( raw_ptr, [] ( [[maybe_unused]] T* ) -> void {} );
        return *this;
    }

    auto& reset( T* raw_ptr, [[maybe_unused]] strong_link_t ) {
        this->Base::reset( raw_ptr );
        return *this;
    }

    auto& reset( void* raw_ptr, [[maybe_unused]] weak_link_t ) {
        return this->reset( reinterpret_cast< T* >( raw_ptr ), weak_link_t{} );
    }

    auto& reset( void* raw_ptr, [[maybe_unused]] strong_link_t ) {
        return this->reset( reinterpret_cast< T* >( raw_ptr ), strong_link_t{} );
    }

};

template< typename T >
using VPtr = VolatilePtr< T >;



};
