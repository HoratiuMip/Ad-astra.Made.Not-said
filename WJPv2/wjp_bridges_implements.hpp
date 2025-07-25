/*===== Warp Joint Protocol v2 - Bridges implements - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Implementation for structures used inside the protocol.
|
======*/
#include "wjp_internals.hpp"
#include "wjp_bridges.hpp"


/* Concurrent. */
template< typename _T > struct WJP_Interlocked : WJP_BRIDGE_Interlocked< _T > {
#if defined( _WJP_SEMANTICS_STL_ATOMICS )
    std::atomic< _T >   _flag;
#else
    _T                  _flag;
#endif

#if defined( _WJP_SEMANTICS_STL_ATOMICS )
    _WJP_forceinline _T read( _WJP_MEM_ORD_DFT_ARG ) override { return _flag.load( mem_ord ); }
    _WJP_forceinline void write( _T flag, _WJP_MEM_ORD_DFT_ARG ) override { return _flag.store( flag, mem_ord ); }
#else
    _WJP_forceinline _T read( _WJP_MEM_ORD_DFT_ARG ) override { return _flag; }
    _WJP_forceinline void write( _T flag, _WJP_MEM_ORD_DFT_ARG ) override { return _flag = flag; }
#endif

#if defined( _WJP_SEMANTICS_STL_ATOMICS_SIGNALABLE )
    _WJP_forceinline void sig_one( void ) override { _flag.notify_one(); }
    _WJP_forceinline void sig_all( void ) override { _flag.notify_all(); }

    _WJP_forceinline void wait( _T flag ) override { _flag.wait( flag ); }
#else
    _WJP_forceinline void sig_one( void ) override { _WJP_EMPTY_FUNCTION }
    _WJP_forceinline void sig_all( void ) override { _WJP_EMPTY_FUNCTION }

    _WJP_forceinline void wait( _T flag ) override { _WJP_EMPTY_FUNCTION }
#endif
};

struct WJP_Mutex : WJP_BRIDGE_Mutex {
#if defined( _WJP_SEMANTICS_STL_MUTEX )
    std::mutex   _mtx;
#endif 

#if defined( _WJP_SEMANTICS_STL_MUTEX )
    _WJP_forceinline void lock( void ) override { _mtx.lock(); }
    _WJP_forceinline void unlock( void ) override { _mtx.unlock(); }
    _WJP_forceinline bool try_lock( void ) override { return _mtx.try_lock(); }
#else
    _WJP_forceinline void lock( void ) override { _WJP_EMPTY_FUNCTION }
    _WJP_forceinline void unlock( void ) override { _WJP_EMPTY_FUNCTION }
    _WJP_forceinline bool try_lock( void ) override { return true; }
#endif
};


/* Containers. */
template< typename _T > struct WJP_CircularQueue : WJP_BRIDGE_Queue< _T > {
    #if defined( _WJP_SEMANTICS_STL_FUNCTION )
        typedef   std::function< void( _T& ) >   for_each_cb_t;
    #else
        typedef   void ( *for_each_cb_t )( _T& );
    #endif

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
        return this->empty() ? nullptr : this->front();
    }

    _WJP_forceinline void clear( void ) override {
        while( this->pop() == 0 );
    }

    _WJP_forceinline _T* front( void ) override {
        return &_mdsc.addr[ _head ];
    }

    _WJP_forceinline void for_each( for_eache_cb_t cb ) override {
        int head = _head;
        int span = _span;

        while( span-- >= 0 ) {
            cb( _mdsc.addr[ head++ ] );
            if( head >= _mdsc.sz ) head = 0;
        }
    }
};