/*===== Warp Joint Protocol - Internals - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Structures and functions used internally by the protocol.
|
======*/


#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_ARDUINO )
    #include <atomic>
    #include <deque>
    #include <mutex>
#endif


#if defined( WJP_ENVIRONMENT_MINGW )
    #define _WJP_forceinline __forceinline
#elif defined( WJP_ENVIRONMENT_ARDUINO )
    #define _WJP_forceinline inline
#else
    #error "[ WJP ] Environment not specified or unsupported."
#endif 


#define _WJP_SUICIDE ( *( int* )-1 = *( int* )0 )


/*
|>  INTERLOCKED ======
*/
/* Memory orders. */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_ARDUINO )
    #define _WJP_MEM_ORD         std::memory_order
    #define _WJP_MEM_ORD_ARG     std::memory_order mem_ord
    #define _WJP_MEM_ORD_DFT_ARG _WJP_MEM_ORD_ARG = _WJP_MEM_ORD_SEQ_CST

    #define _WJP_MEM_ORD_RELEASE std::memory_order_release
    #define _WJP_MEM_ORD_SEQ_CST std::memory_order_seq_cst
#else
    #define _WJP_MEM_ORD
    #define _WJP_MEM_ORD_ARG
    #define _WJP_MEM_ORD_DFT_ARG

    #define _WJP_MEM_ORD_RELEASE
    #define _WJP_MEM_ORD_SEQ_CST
#endif

struct _WJP_INTERLOCKED_BOOL {

/* Flag. */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_ARDUINO )
    std::atomic_bool   _flag;
#endif

/* Read/Write. */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_ARDUINO )
    _WJP_forceinline bool read( _WJP_MEM_ORD_DFT_ARG ) { return _flag.load( mem_ord ); }
    _WJP_forceinline void write( bool flag, _WJP_MEM_ORD_DFT_ARG ) { return _flag.store( flag, mem_ord ); }
#else
    _WJP_forceinline bool read( _WJP_MEM_ORD_DFT_ARG ) { _WJP_SUICIDE; }
    _WJP_forceinline void write( bool flag, _WJP_MEM_ORD_DFT_ARG ) { _WJP_SUICIDE; }
#endif

/* Signal/Wait. */
#if defined( WJP_ENVIRONMENT_MINGW )
    _WJP_forceinline void sig_one() { _flag.notify_one(); }
    _WJP_forceinline void sig_all() { _flag.notify_all(); }

    _WJP_forceinline void wait( bool flag ) { _flag.wait( flag ); }
#else
    _WJP_forceinline void sig_one() { _WJP_SUICIDE; }
    _WJP_forceinline void sig_all() { _WJP_SUICIDE; }
    _WJP_forceinline void wait( bool flag ) { _WJP_SUICIDE; }
#endif

/* Internal only. */
    _WJP_forceinline void set( _WJP_MEM_ORD_DFT_ARG ) { return this->write( true, mem_ord ); }
    _WJP_forceinline void reset( _WJP_MEM_ORD_DFT_ARG ) { return this->write( false, mem_ord ); }
    _WJP_forceinline bool is_set( _WJP_MEM_ORD_DFT_ARG ) { return this->read( mem_ord ); }
    _WJP_forceinline bool is_reset( _WJP_MEM_ORD_DFT_ARG ) { return !this->is_set( mem_ord ); }

};


/*
|>  QUEUE ======
*/
template< typename _T > struct _WJP_INTERLOCKED_QUEUE {

#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_ARDUINO )
    std::deque< _T >   _deq;
    std::mutex         _mtx;

    _WJP_forceinline bool empty() { return _deq.empty(); }
    template< typename ...Args > _WJP_forceinline _T& emplace( Args&&... args ) { std::unique_lock< decltype( _mtx ) > lock{ _mtx }; return _deq.emplace_back( std::forward< Args >( args )... ); }
    _WJP_forceinline void pop() { std::unique_lock< decltype( _mtx ) > lock{ _mtx }; _deq.pop_front(); }
    _WJP_forceinline void clear() { std::unique_lock< decltype( _mtx ) > lock{ _mtx }; _deq.clear(); }
    _WJP_forceinline _T& front() { return _deq.front(); }

    _WJP_forceinline void for_each( std::function< void( _T& ) > cb ) { std::unique_lock< decltype( _mtx ) > lock{ _mtx }; for( auto& t : _deq ) cb( t ); }
#else
    _WJP_forceinline bool empty() { _WJP_SUICIDE; }
    template< typename ...Args > _WJP_forceinline _T& emplace( Args&&... args ) { _WJP_SUICIDE; }
    _WJP_forceinline void pop() { _WJP_SUICIDE; }
    _WJP_forceinline void clear() { _WJP_SUICIDE; }
    _WJP_forceinline _T& front() { _WJP_SUICIDE; }

    _WJP_forceinline void for_each( std::function< void( _T& ) > cb ) { _WJP_SUICIDE; }
#endif

};


/*
|>  MISC ======
*/
struct WJP_BUFFER {
    void*     addr   = nullptr;
    int32_t   sz     = 0;
    
    _WJP_forceinline char& operator [] ( int offset ) { return ( ( char* )addr )[ offset ]; }
    operator char* () { return ( char* )addr; }
};
struct WJP_CBUFFER {
    const void*   addr   = nullptr;
    int32_t       sz     = 0;
    
    _WJP_forceinline const char& operator [] ( int offset ) { return ( ( const char* )addr )[ offset ]; }
    operator const char* () { return ( const char* )addr; }
};
