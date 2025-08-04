#ifndef WJPV2_BRIDGES_RTG_HPP
#define WJPV2_BRIDGES_RTG_HPP
/*===== Warp Joint Protocol v2 - Bridges implements - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Implementation for structures used inside the protocol.
|
======*/
#include "wjpv2_core.hpp"
#include "wjpv2_bridges.hpp"


/* Concurrent. */
template< typename _T > struct WJP_Interlocked : WJP_BRIDGE_Interlocked< _T > {
    WJP_Interlocked( _T flag ) : _flag{ flag } {}

#if defined( _WJP_SEMANTICS_STL_ATOMICS )
    std::atomic< _T >   _flag;
#else
    _T                  _flag;
#endif

#if defined( _WJP_SEMANTICS_STL_ATOMICS )
    _WJP_forceinline _T read( WJP_MEM_ORD_DFT_ARG ) override { return _flag.load( mem_ord ); }
    _WJP_forceinline void write( _T flag, WJP_MEM_ORD_DFT_ARG ) override { return _flag.store( flag, mem_ord ); }

    _WJP_forceinline _T fetch_add( _T arg, WJP_MEM_ORD_DFT_ARG ) override {
        if constexpr( !std::is_same_v< bool, _T > ) 
            return _flag.fetch_add( arg, mem_ord ); 
        else
            return _flag.exchange( true, mem_ord );
    }
#else
    _WJP_forceinline _T read( WJP_MEM_ORD_DFT_ARG ) override { return _flag; }
    _WJP_forceinline void write( _T flag, WJP_MEM_ORD_DFT_ARG ) override { return _flag = flag; }

    _WJP_forceinline _T fetch_add( _T arg, WJP_MEM_ORD_DFT_ARG ) override { return _flag++; }
#endif

#if defined( _WJP_SEMANTICS_STL_ATOMICS_SIGNALABLE )
    _WJP_forceinline void signal( void ) override { _flag.notify_one(); }
    _WJP_forceinline void broadcast( void ) override { _flag.notify_all(); }

    _WJP_forceinline void hold( _T flag ) override { _flag.wait( flag ); }
#else
    _WJP_forceinline void signal( void ) override { _WJP_EMPTY_FUNCTION }
    _WJP_forceinline void broadcast( void ) override { _WJP_EMPTY_FUNCTION }

    _WJP_forceinline void hold( _T flag ) override { _WJP_EMPTY_FUNCTION }
#endif
};

struct WJP_Mutex : WJP_BRIDGE_Mutex {
#if defined( _WJP_SEMANTICS_STL_MUTEX )
    std::mutex   _mtx;
#endif 

#if defined( _WJP_SEMANTICS_STL_MUTEX )
    _WJP_forceinline void acquire( void ) override { _mtx.lock(); }
    _WJP_forceinline void release( void ) override { _mtx.unlock(); }
    _WJP_forceinline bool try_acquire( void ) override { return _mtx.try_lock(); }
#else
    _WJP_forceinline void acquire( void ) override { _WJP_EMPTY_FUNCTION }
    _WJP_forceinline void release( void ) override { _WJP_EMPTY_FUNCTION }
    _WJP_forceinline bool try_acquire( void ) override { return true; }
#endif
};


/* Containers. */
template< typename _T > struct WJP_CircularQueue : WJP_BRIDGE_Queue< _T > {
    WJP_MDsc< _T >   _mdsc   = {};
    int              _head   = 0;
    int              _tail   = 0;
    int              _span   = 0;

    _WJP_forceinline void bind_mdsc( WJP_MDsc< _T > mdsc ) {
        _mdsc = mdsc;
    }

    _WJP_forceinline bool is_empty( void ) const override {
        return _span == 0;
    }

    _T* push( _T&& arg ) override { 
        if( _span == _mdsc.sz ) return nullptr;
    
    #if defined( _WJP_SEMANTICS_ADDRESABLE_NEW )
        _T* ptr = new ( ( void* )&_mdsc.addr[ _tail++ ] ) _T{ ( _T&& )arg };
    #else 
        _T* ptr = &_mdsc.addr[ _tail++ ];
        *ptr = _T( ( _T&& )arg );
    #endif
        if( _tail >= _mdsc.sz ) _tail = 0; 

        ++_span;
        return ptr;
    }

    _T* pop( void ) override {
        if( this->is_empty() ) return nullptr;

        _mdsc.addr[ _head++ ].~_T();
        if( _head >= _mdsc.sz ) _head = 0;

        --_span;
        return this->is_empty() ? nullptr : this->front();
    }

    void trim( void ) override {
        if( this->is_empty() ) return;

        --_tail; if( _tail < 0 ) _tail = _mdsc.sz - 1;

        _mdsc.addr[ _tail ].~_T();
        
        --_span;
    }

    _WJP_forceinline int clear( void ) override {
        int count = 0;
        while( this->pop() != nullptr ) ++count;
        return count;
    }

    _WJP_forceinline _T* front( void ) override {
        return &_mdsc.addr[ _head ];
    }

    _WJP_forceinline void for_each( typename WJP_BRIDGE_Queue< _T >::for_each_cb_t cb ) override {
        int head = _head;
        int span = _span;

        while( span-- >= 0 ) {
            cb( &_mdsc.addr[ head++ ] );
            if( head >= _mdsc.sz ) head = 0;
        }
    }
};


#endif