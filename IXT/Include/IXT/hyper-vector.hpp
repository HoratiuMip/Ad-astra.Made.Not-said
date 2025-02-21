#pragma once
/*
[] CAUTION - SHALL BE USED ONLY BY PROFESSIONALS.
*/

#include <IXT/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



constexpr QWORD HYPER_VECTOR_SIZE = 16;

enum HYPER_VECTOR_TAG : WORD {
    HYPER_VECTOR_TAG_HARD        = 1 << 0,
    HYPER_VECTOR_TAG_INFO        = 1 << 1,
    HYPER_VECTOR_TAG_CONSTRUCTED = 1 << 2,

    _HYPER_VECTOR_TAG_FORCE_WORD = 0x7f'ff
};

enum HYPER_VECTOR_ALLOC_FLAG : WORD {
    HYPER_VECTOR_ALLOC_FLAG_CONSTRUCT  = 1 << 0,
    //HYPER_VECTOR_ALLOC_FLAG_TRACK_REFS = 1 << 1,

    _HYPER_VECTOR_ALLOC_FLAG_FORCE_WORD = 0x7f'ff
};

struct hvec_soft_t{};

template< typename _T, bool _is_array = std::is_array_v< _T > >
class HYPER_VECTOR {
public:
    template< typename, bool > friend class HYPER_VECTOR;

public:
    using T = std::remove_pointer_t< std::decay_t< _T > >;

public:
    HYPER_VECTOR() = default;

    HYPER_VECTOR( const HYPER_VECTOR& other )
    : _ptr{ ( T* )other._ptr }, _tags{ other._tags }
    {
        this->_copy( other );
    }

    template< typename Thv > requires( !std::is_class_v< Thv > || std::is_base_of_v< T, Thv > )
    HYPER_VECTOR( const HYPER_VECTOR< Thv >& other ) 
    : _ptr{ ( T* )other._ptr }, _tags{ other._tags }
    {
        this->_copy( other );
    }

    HYPER_VECTOR( HYPER_VECTOR&& other )
    : _ptr{ ( T* )other._ptr }, _tags{ other._tags }
    {
        this->_move( std::move( other ) );
    }
    template< typename Thv > requires( !std::is_class_v< Thv > || std::is_base_of_v< T, Thv > )
    HYPER_VECTOR( HYPER_VECTOR< Thv >&& other ) 
    : _ptr{ ( T* )other._ptr }, _tags{ other._tags }
    {
         this->_move( std::move( other ) );
    }

    template< typename Tp > requires( !std::is_class_v< Tp > || std::is_base_of_v< T, Tp > )
    HYPER_VECTOR( Tp* under )
    : _ptr{ ( T* )under }, _tags{ HYPER_VECTOR_TAG_HARD } 
    {}

    template< typename Tp > requires( !std::is_class_v< Tp > || std::is_base_of_v< T, Tp > )
    HYPER_VECTOR( Tp* under, [[maybe_unused]]hvec_soft_t )
    : _ptr{ ( T* )under }, _tags{ 0 } 
    {}

    template< typename Tr > requires( !std::is_class_v< Tr > || std::is_base_of_v< T, Tr > )
    HYPER_VECTOR( Tr& under )
    : _ptr{ ( T* )&under }, _tags{ 0 } 
    {}

    HYPER_VECTOR( decltype( nullptr ) )
    {}

_ENGINE_PROTECTED:
    explicit HYPER_VECTOR( T* ptr, WORD tags )
    : _ptr{ ptr }, _tags{ ptr == nullptr ? ( WORD )0 : tags }
    {}

_ENGINE_PROTECTED:
    template< typename Thv >
    inline void _copy( const HYPER_VECTOR< Thv >& other ) {
        if( _ALLOC_INFO* info = this->_info(); info != nullptr ) {
            info->ref_count.fetch_add( 1, std::memory_order_acquire );
        } else {
            _tags &= ~HYPER_VECTOR_TAG_HARD;
        }
    }

    template< typename Thv >
    inline void _move( HYPER_VECTOR< Thv >&& other ) {
        memset( ( void* )&other, 0, HYPER_VECTOR_SIZE );
    }

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

_ENGINE_PROTECTED:
    template< HYPER_VECTOR_ALLOC_FLAG a_flag, typename ...Args > requires( !std::is_abstract_v< T > )
    static HYPER_VECTOR< _T > _alloc( QWORD count, Args&&... args ) {
        void* base = malloc( sizeof( _ALLOC_INFO ) + sizeof( T ) * count );

        if( base == nullptr ) return HYPER_VECTOR< _T >{ nullptr, ( WORD )0 };

        _ALLOC_INFO& info = *( _ALLOC_INFO* )base;
        T* ptr = ( T* )( ( BYTE* )base + sizeof( _ALLOC_INFO ) );

        WORD tag = ( WORD )( HYPER_VECTOR_TAG_HARD | HYPER_VECTOR_TAG_INFO );

        if constexpr( a_flag & HYPER_VECTOR_ALLOC_FLAG_CONSTRUCT ) {
            tag = WORD( tag | HYPER_VECTOR_TAG_CONSTRUCTED );

            if constexpr( _is_array ) {
                for( QWORD idx = 0; idx < count; ++idx )
                    new( ptr + idx ) T{ std::forward< Args >( args )... };
                
                info.count = count;
            } else {
                new( ptr ) T{ std::forward< Args >( args )... };
            }
        }

        info.ref_count.store( 1, std::memory_order_release );
       
        return HYPER_VECTOR< _T >{ ptr, tag };
    }

public:
    template< typename ...Args > requires( !std::is_abstract_v< T > )
    inline static HYPER_VECTOR< _T > alloc( Args&&... args ) {
        return _alloc< ( WORD )0 >( 1, std::forward< Args >( args )... );
    }

    template< typename ...Args > requires( !std::is_abstract_v< T > )
    inline static HYPER_VECTOR< _T > allocc( Args&&... args ) {
        return _alloc< HYPER_VECTOR_ALLOC_FLAG_CONSTRUCT >( 1, std::forward< Args >( args )... );
    }

    template< typename ...Args > requires( !std::is_abstract_v< T > )
    inline static HYPER_VECTOR< _T > allocv( QWORD count, Args&&... args ) {
        return _alloc< ( WORD )0 >( count, std::forward< Args >( args )... );
    }

    template< typename ...Args > requires( !std::is_abstract_v< T > )
    inline static HYPER_VECTOR< _T > allocvc( QWORD count, Args&&... args ) {
        return _alloc< HYPER_VECTOR_ALLOC_FLAG_CONSTRUCT >( count, std::forward< Args >( args )... );
    }

_ENGINE_PROTECTED:
    void _free() {
        void* free_ptr = nullptr;

        if( _ptr == nullptr ) goto l_reset;

        if( !this->hard() ) goto l_reset;

    {     
        _ALLOC_INFO* info = this->_info();

        if( info == nullptr ) { free_ptr = ( void* )_ptr; goto l_free; }
    {
        QWORD old_ref_count = info->ref_count.fetch_sub( 1, std::memory_order_seq_cst );

        if( old_ref_count != 1 ) goto l_reset;

        if constexpr( _is_array ) {
            for( QWORD idx = 0; idx < info->count; ++idx )
                ( _ptr + idx )->~T();
        } else {
            _ptr->~T();
        }

        free_ptr = ( void* )info;
    }
    }

    l_free:
        free( free_ptr );

    l_reset:
        memset( ( void* )this, 0, HYPER_VECTOR_SIZE );
    }

public:
    template< typename ...Args >
    HYPER_VECTOR< _T >& vector( Args&&... args ) { 
        this->_free();

        new( ( void* )this ) HYPER_VECTOR< _T >{ std::forward< Args >( args )... };
        return *this;
    }

    template< typename ...Args >
    HYPER_VECTOR< _T >& operator = ( Args&&... args ) {
        return this->vector( std::forward< Args >( args )... );
    }

public:
    inline QWORD count() const {
        _ALLOC_INFO* info = this->_info();

        return info ? info->ref_count.load( std::memory_order_relaxed ) : 1;
    }
    
    inline bool hard() const {
        return ( _tags & HYPER_VECTOR_TAG_HARD ) != 0;
    }

_ENGINE_PROTECTED:
    inline _ALLOC_INFO* _info() const {
        if( ( _tags & HYPER_VECTOR_TAG_INFO ) == 0 ) return nullptr;

        return ( _ALLOC_INFO* )( ( BYTE* )_ptr - sizeof( _ALLOC_INFO ) );
    }

public:
    const T* operator -> () const { return _ptr; }
    const T* get() const { return _ptr; }
    std::enable_if_t< !std::is_same_v< T, void >, const T& > operator * () const { return *_ptr; }
    std::enable_if_t< !std::is_same_v< T, void >, const T& > operator [] ( ptrdiff_t diff ) const { return _ptr[ diff ]; }
    const T* operator + ( ptrdiff_t diff ) const { return _ptr + diff; }
    const T* operator - ( ptrdiff_t diff ) const { return _ptr - diff; }
    std::enable_if_t< !std::is_same_v< T, void >, const T& > operator >> ( ptrdiff_t diff ) const { return *( _ptr + diff ); }
    std::enable_if_t< !std::is_same_v< T, void >, const T& > operator << ( ptrdiff_t diff ) const { return *( _ptr - diff ); }

    T* operator -> () { return _ptr; }
    T* get() { return _ptr; }
    std::enable_if_t< !std::is_same_v< T, void >, T& > operator * () { return *_ptr; }
    std::enable_if_t< !std::is_same_v< T, void >, T& > operator [] ( ptrdiff_t diff ) { return _ptr[ diff ]; }
    T* operator + ( ptrdiff_t diff ) { return _ptr + diff; }
    T* operator - ( ptrdiff_t diff ) { return _ptr - diff; }
    std::enable_if_t< !std::is_same_v< T, void >, T& > operator >> ( ptrdiff_t diff ) { return *( _ptr + diff ); }
    std::enable_if_t< !std::is_same_v< T, void >, T& > operator << ( ptrdiff_t diff ) { return *( _ptr - diff ); }

public: 
    bool operator == ( T* ptr ) const { return _ptr == ptr; }
    bool operator != ( T* ptr ) const { return _ptr != ptr; }
    bool operator == ( decltype( nullptr ) ) const { return _ptr == nullptr; }
    bool operator != ( decltype( nullptr ) ) const { return _ptr != nullptr; }
    operator bool () const { return _ptr != nullptr; }

};
static_assert( sizeof( HYPER_VECTOR< BYTE > ) == HYPER_VECTOR_SIZE );

template< typename T >
using HVEC = HYPER_VECTOR< T >;



};
