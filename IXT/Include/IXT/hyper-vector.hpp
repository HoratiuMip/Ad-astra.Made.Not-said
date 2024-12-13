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

    VolatilePtr( T* raw_ptr, [[maybe_unused]] no_free_t ) : Base{ raw_ptr, [] ( [[maybe_unused]] T* ) -> void {} } {}

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
    // operator T* () const {
    //     return this->get();
    // }

    // operator const T* () const {
    //     return this->get();
    // }

    // operator T& () {
    //     return *this->get();
    // }

    // operator const T& () const {
    //     return *this->get();
    // }

    // template< typename V >
    // operator V* () const {
    //     return ( V* )this->get();
    // }

    // template< typename V >
    // operator const V* () const {
    //     return ( const V* )this->get();
    // }

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

    VolatilePtr< _T >& reset( VolatilePtr< _T >&& vptr_move_ref ) {
        static_cast< Base& >( *this ) = std::move( vptr_move_ref );
        return *this;
    }
};

template< typename T >
using VPtr = VolatilePtr< T >;



constexpr QWORD HYPER_VECTOR_SIZE = 16;

enum HYPER_VECTOR_TAG : WORD {
    HYPER_VECTOR_TAG_HARD = 1 << 0,
    HYPER_VECTOR_TAG_INFO = 1 << 1,

    _HYPER_VECTOR_TAG_FORCE_WORD = 0x7f'ff
};

template< typename _T, bool _is_array = std::is_array_v< _T > >
class HYPER_VECTOR {
public:
    template< typename _Tf, bool _is_array_f > friend class HYPER_VECTOR;

public:
    using T = std::remove_pointer_t< std::decay_t< _T > >;

public:
    HYPER_VECTOR() = default;

    template< typename Td > requires( !std::is_class_v< Td > || std::is_base_of_v< T, Td > )
    HYPER_VECTOR( const HYPER_VECTOR< Td >& other ) 
    : _ptr{ ( T* )other._ptr }, _tags{ other._tags }
    {std::cout << 1 << ' ';
        ( ( _ALLOC_INFO* )( ( BYTE* )_ptr - sizeof( _ALLOC_INFO ) ) )->ref_count.fetch_add( 1, std::memory_order_acquire);
    }

    template< typename Td > requires( !std::is_class_v< Td > || std::is_base_of_v< T, Td > )
    HYPER_VECTOR( HYPER_VECTOR< Td >&& other ) 
    : _ptr{ ( T* )other._ptr }, _tags{ other._tags }
    {
        std::cout << 2 << ' ';
        memset( ( void* )&other, 0, HYPER_VECTOR_SIZE );
    }

    template< typename Td > requires( !std::is_class_v< Td > || std::is_base_of_v< T, Td > )
    HYPER_VECTOR( Td* under )
    : _ptr{ ( T* )under }, _tags{ HYPER_VECTOR_TAG_HARD } 
    { std::cout << 3 << ' ';}

    template< typename Td > requires( !std::is_class_v< Td > || std::is_base_of_v< T, Td > )
    HYPER_VECTOR( Td& under )
    : _ptr{ ( T* )&under }, _tags{ 0 } 
    {std::cout << 4 << ' ';}

    HYPER_VECTOR( decltype( nullptr ) )
    {std::cout << 6 << ' ';}

_ENGINE_PROTECTED:
    explicit HYPER_VECTOR( T* ptr, WORD tags )
    : _ptr{ ptr }, _tags{ ptr == nullptr ? ( WORD )0 : tags }
    {}

public:
    ~HYPER_VECTOR() {
        this->_free();
    }

_ENGINE_PROTECTED:
    struct _ALLOC_INFO_SCALAR {
        std::atomic< QWORD >   ref_count   = { 0 };
    };
    struct _ALLOC_INFO_VECTOR : _ALLOC_INFO_SCALAR {
        QWORD   count   = 0;
    };
    using _ALLOC_INFO = std::conditional_t< _is_array, _ALLOC_INFO_VECTOR, _ALLOC_INFO_SCALAR >;

_ENGINE_PROTECTED:
    T*     _ptr             = nullptr;
    WORD   _tags            = 0;
    BYTE   _reserved[ 6 ]   = {};

public:
    template< typename ...Args >
    requires( !std::is_abstract_v< T > )
    static HYPER_VECTOR< _T > alloc( QWORD count, Args&&... args ) {std::cout <<  "cnt" << count<< ' ';
        void* base = malloc( sizeof( _ALLOC_INFO ) + sizeof( T ) * count );
        if( base == nullptr ) return HYPER_VECTOR< _T >{ nullptr, 0 };

        _ALLOC_INFO& info = *( _ALLOC_INFO* )base;
        T* ptr = ( T* )( ( BYTE* )base + sizeof( _ALLOC_INFO ) );

        if constexpr( _is_array ) {
            for( QWORD idx = 0; idx < count; ++idx )
                new( ptr + idx ) T{ std::forward< Args >( args )... };
            
            info.count = count;
        } else {
            new( ptr ) T{ std::forward< Args >( args )... };
        }

        info.ref_count.store( 1, std::memory_order_release );
        std::cout << 8 << ' ';
        return HYPER_VECTOR< _T >{ ptr, HYPER_VECTOR_TAG_HARD | HYPER_VECTOR_TAG_INFO };
    }

_ENGINE_PROTECTED:
    void _free() {
        void* free_ptr = nullptr; std::cout << 9;

        if( _ptr == nullptr ) goto l_reset;
        if( ( _tags & HYPER_VECTOR_TAG_HARD ) == 0 ) goto l_reset;
        if( ( _tags & HYPER_VECTOR_TAG_INFO ) == 0 ) { free_ptr = ( void* )_ptr; goto l_free; }
        
        {
        _ALLOC_INFO& info = *( _ALLOC_INFO* )( ( BYTE* )_ptr - sizeof( _ALLOC_INFO ) );
        QWORD old_ref_count = info.ref_count.fetch_sub( 1, std::memory_order_seq_cst );

        if( old_ref_count != 1 ) goto l_reset;

        if constexpr( _is_array ) {
            for( QWORD idx = 0; idx < info.count; ++idx )
                ( _ptr + idx )->~T();
        } else {
            _ptr->~T();
        }

        free_ptr = ( void* )&info;
        }

    l_free: std::cout<<'X';
        free( free_ptr );

    l_reset:
        memset( ( void* )this, 0, HYPER_VECTOR_SIZE ); std::cout << 10;
    }

public:
    template< typename ...Args >
    HYPER_VECTOR< _T >& vector( Args&&... args ) {
        this->_free();

        new( ( void* )this ) HYPER_VECTOR< _T >{ std::forward< Args >( args )... };
        return *this;
    }

    QWORD count() const {
        return ( ( _ALLOC_INFO* )( ( BYTE* )_ptr - sizeof( _ALLOC_INFO ) ) )->ref_count.load( std::memory_order_relaxed );
    }

public:
    const T* operator -> () const { return _ptr; }
    const T* get() const { return _ptr; }
    const T& operator * () const { return *_ptr; }
    const T& operator [] ( ptrdiff_t diff ) const { return _ptr[ diff ]; }
    const T* operator + ( ptrdiff_t diff ) const { return _ptr + diff; }
    const T* operator - ( ptrdiff_t diff ) const { return _ptr - diff; }
    const T& operator >> ( ptrdiff_t diff ) const { return *( _ptr + diff ); }
    const T& operator << ( ptrdiff_t diff ) const { return *( _ptr - diff ); }

    T* operator -> () { return _ptr; }
    T* get() { return _ptr; }
    T& operator * () { return *_ptr; }
    T& operator [] ( ptrdiff_t diff ) { return _ptr[ diff ]; }
    T* operator + ( ptrdiff_t diff ) { return _ptr + diff; }
    T* operator - ( ptrdiff_t diff ) { return _ptr - diff; }
    T& operator >> ( ptrdiff_t diff ) { return *( _ptr + diff ); }
    T& operator << ( ptrdiff_t diff ) { return *( _ptr - diff ); }

public: 
    bool operator != ( T* ptr ) const { return _ptr != ptr; }
    operator bool () const { return _ptr != nullptr; }

public:
    HYPER_VECTOR< _T >&& reloc() {
        return ( HYPER_VECTOR< _T >&& )*this;
    }

};
static_assert( sizeof( HYPER_VECTOR< BYTE > ) == HYPER_VECTOR_SIZE );

template< typename T >
using HVEC = HYPER_VECTOR< T >;



};
