#pragma once
/*
[] CAUTION - SHALL BE USED ONLY BY PROFESSIONALS.
*/

#include <IXT/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



struct no_free_t{};

template< typename _T, bool _is_array = std::is_array_v< _T > >
class VolatilePtr : public SPtr< _T > {
public:
    using Base = SPtr< _T >;

public:
    using T = std::remove_pointer_t< std::decay_t< _T > >;

public:
    VolatilePtr() = default;

    VolatilePtr( const VolatilePtr& other ) : Base{ other } {}

    VolatilePtr( VolatilePtr&& other ) : Base{ std::move( other ) } {}

    VolatilePtr( std::nullptr_t ) : Base{ nullptr } {}

    VolatilePtr( decltype( NULL ) ) : Base{ nullptr } {}

    VolatilePtr( T* raw_ptr ) : Base{ raw_ptr } {}

    VolatilePtr( T& raw_ptr, [[maybe_unused]] no_free_t ) : Base{ &raw_ptr, [] ( [[maybe_unused]] T* ) -> void {} } {}

    VolatilePtr( T& raw_ref ) : Base{ &raw_ref, [] ( [[maybe_unused]] T* ) -> void {} } {}

    VolatilePtr( T&& raw_move_ref ) noexcept : Base{ std::make_shared< T >( std::move( raw_move_ref ) ) } {}

    VolatilePtr( const SPtr< T >& other ) : Base{ other } {}

    VolatilePtr( SPtr< T >&& other ) noexcept : Base{ std::move( other ) } {}

public:
    T* operator + ( ptrdiff_t diff ) const {
        return this->get() + diff;
    }

    T* operator - ( ptrdiff_t diff ) const {
        return this->get() - diff;
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

    template< typename V >
    operator V* () const {
        return ( V* )this->get();
    }

    template< typename V >
    operator const V* () const {
        return ( const V* )this->get();
    }

public:
    VolatilePtr< _T >& reset( std::nullptr_t ) {
        this->Base::reset();
        return *this;
    }

    VolatilePtr< _T >& reset( decltype( NULL ) ) {
        this->Base::reset();
        return *this;
    }

    VolatilePtr< _T >& reset( T* raw_ptr ) {
        this->Base::reset( raw_ptr );
        return *this;
    }

    VolatilePtr< _T >& reset( T* raw_ptr, [[maybe_unused]] no_free_t ) {
        this->Base::reset( raw_ptr, [] ( [[maybe_unused]] T* ) -> void {} );
        return *this;
    }

    VolatilePtr< _T >& reset( T& raw_ref ) {
        this->Base::reset( &raw_ref, [] ( [[maybe_unused]] T* ) -> void {} );
        return *this;
    }

    VolatilePtr< _T >& reset( T&& raw_move_ref ) {
        this->Base::reset( std::make_shared< T >( raw_move_ref ) );
        return *this;
    }

    template< typename V >
    auto& operator = ( V&& raw ) {
        if constexpr( std::is_pointer_v< V > )
            return this->reset( ( T* ) raw );
        else
            return this->reset( std::forward< V >( raw ) );
    }

};

template< typename T >
using VPtr = VolatilePtr< T >;



};
