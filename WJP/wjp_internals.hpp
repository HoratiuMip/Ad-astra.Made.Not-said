/*===== Warp Joint Protocol - Internals - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Structures and functions used internally by the protocol.
|
======*/


#if defined( WJP_ENVIRONMENT_MINGW )
    #define _WJP_forceinline __forceinline
#elif defined( WJP_ENVIRONMENT_RTOS ) || defined( WJP_ENVIRONMENT_AVR )
    #define _WJP_forceinline inline
#else
    #error "[ WJP ] Environment not specified or unsupported."
#endif 


#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
    #include <atomic>
    #include <deque>
    #include <mutex>
    #include <functional>
#elif defined( WJP_ENVIRONMENT_AVR )
    
#endif


#define _WJP_EMPTY_FUNCTION


/*
|>  INTERLOCKED ======
*/
/* Memory orders. */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
    #define _WJP_MEM_ORD         std::memory_order
    #define _WJP_MEM_ORD_ARG     std::memory_order mem_ord
    #define _WJP_MEM_ORD_DFT_ARG _WJP_MEM_ORD_ARG = _WJP_MEM_ORD_SEQ_CST

    #define _WJP_MEM_ORD_RELEASE std::memory_order_release
    #define _WJP_MEM_ORD_SEQ_CST std::memory_order_seq_cst
#else
    #define _WJP_MEM_ORD bool
    #define _WJP_MEM_ORD_ARG [[maybe_unused]]char mem_ord
    #define _WJP_MEM_ORD_DFT_ARG [[maybe_unused]]char mem_ord = 0x0

    #define _WJP_MEM_ORD_RELEASE 0x0
    #define _WJP_MEM_ORD_SEQ_CST 0x0
#endif


/*
|>  MISC ======
*/
struct WJP_BUFFER {
#if defined WJP_ENVIRONMENT_AVR
    WJP_BUFFER() = default;
    WJP_BUFFER( void* a, int32_t s ) : addr{ a }, sz{ s } {}
#endif

    void*     addr   = nullptr;
    int32_t   sz     = 0;
    
    _WJP_forceinline char& operator [] ( int offset ) { return ( ( char* )addr )[ offset ]; }
    operator char* () { return ( char* )addr; }
    operator void* () { return addr; }
};
struct WJP_CBUFFER {
#if defined WJP_ENVIRONMENT_AVR
    WJP_CBUFFER() = default;
    WJP_CBUFFER( const void* a, int32_t s ) : addr{ a }, sz{ s } {}
#endif

    const void*   addr   = nullptr;
    int32_t       sz     = 0;
    
    _WJP_forceinline const char& operator [] ( int offset ) { return ( ( const char* )addr )[ offset ]; }
    operator const char* () { return ( const char* )addr; }
    operator const void* () { return addr; }
};

template< typename _T > struct WJP_BUFFER_EX {
#if defined WJP_ENVIRONMENT_AVR
    WJP_BUFFER_EX() = default;
    WJP_BUFFER_EX( _T* a, int32_t s ) : addr{ a }, sz{ s } {}
#endif

    _T*       addr   = nullptr;
    int32_t   sz     = 0;
    
    _WJP_forceinline _T& operator [] ( int offset ) { return addr[ offset ]; }
    operator _T* () { return addr; }
    operator void* () { return ( void* )addr; }
};
template< typename _T > struct WJP_CBUFFER_EX {
#if defined WJP_ENVIRONMENT_AVR
    WJP_CBUFFER_EX() = default;
    WJP_CBUFFER_EX( const _T* a, int32_t s ) : addr{ a }, sz{ s } {}
#endif

    const _T*   addr   = nullptr;
    int32_t     sz     = 0;
    
    _WJP_forceinline const _T& operator [] ( int offset ) { return addr[ offset ]; }
    operator const _T* () { return addr; }
    operator const void* () { return ( void* )addr; }
};


/*
|>  MUTEX ======
*/
struct _WJP_INTERLOCKED_BOOL {
/* Flag. */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
    std::atomic_bool   _flag;
#elif defined( WJP_ENVIRONMENT_AVR )
    bool               _flag;
#endif

/* Read/Write. */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
    _WJP_forceinline bool read( _WJP_MEM_ORD_DFT_ARG ) { return _flag.load( mem_ord ); }
    _WJP_forceinline void write( bool flag, _WJP_MEM_ORD_DFT_ARG ) { return _flag.store( flag, mem_ord ); }
#else
    _WJP_forceinline bool read( _WJP_MEM_ORD_DFT_ARG ) { return _flag; }
    _WJP_forceinline void write( bool flag, _WJP_MEM_ORD_DFT_ARG ) { return _flag = flag; }
#endif

/* Signal/Wait. */
#if defined( WJP_ENVIRONMENT_MINGW )
    _WJP_forceinline void sig_one() { _flag.notify_one(); }
    _WJP_forceinline void sig_all() { _flag.notify_all(); }

    _WJP_forceinline void wait( bool flag ) { _flag.wait( flag ); }

#elif defined( WJP_ENVIRONMENT_RTOS ) || defined( WJP_ENVIRONMENT_AVR )
    _WJP_forceinline void sig_one() { _WJP_EMPTY_FUNCTION }
    _WJP_forceinline void sig_all() { _WJP_EMPTY_FUNCTION }

    _WJP_forceinline void wait( bool flag ) { _WJP_EMPTY_FUNCTION }

#endif

/* Internal only. */
    _WJP_forceinline void set( _WJP_MEM_ORD_DFT_ARG ) { return this->write( true, mem_ord ); }
    _WJP_forceinline void reset( _WJP_MEM_ORD_DFT_ARG ) { return this->write( false, mem_ord ); }
    _WJP_forceinline bool is_set( _WJP_MEM_ORD_DFT_ARG ) { return this->read( mem_ord ); }
    _WJP_forceinline bool is_reset( _WJP_MEM_ORD_DFT_ARG ) { return !this->is_set( mem_ord ); }

};

struct _WJP_MUTEX {
/* Mutex. */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
    std::mutex   _mtx;
#endif

/* Lock/Unlock. */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
    _WJP_forceinline void lock() { _mtx.lock(); }
    _WJP_forceinline void unlock() { _mtx.unlock(); }
#else
    _WJP_forceinline void lock() { _WJP_EMPTY_FUNCTION }
    _WJP_forceinline void unlock() { _WJP_EMPTY_FUNCTION }
#endif
};

struct _WJP_SCOPED_LOCK {
    _WJP_SCOPED_LOCK( _WJP_MUTEX* mtx )
    : _mtx{ mtx }
    {
        this->lock();
    }

    ~_WJP_SCOPED_LOCK() {
        if( _acqd == true ) this->unlock();
    }

    _WJP_MUTEX*   _mtx;
    bool          _acqd;

    _WJP_forceinline void lock() { _mtx->lock(); _acqd = true; }
    _WJP_forceinline void unlock() { _acqd = false; _mtx->unlock(); }
};


/*
|>  QUEUE ======
*/
template< typename _T > struct _WJP_CIRCULAR_QUEUE {
    #if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
        typedef std::function< void( _T& ) > fe_cb_t;
    #else
        typedef void ( *fe_cb_t )( _T& );
    #endif

    WJP_BUFFER_EX< _T >   _buffer   = {};
    int                   _head     = 0;
    int                   _tail     = 0;
    int                   _span     = 0;

    _WJP_forceinline void reset_buffer( WJP_BUFFER_EX< _T > buffer ) {
        _buffer = buffer;
    }

    _WJP_forceinline bool is_empty( void ) const {
        return _span == 0;
    }

    _T* push( _T&& arg ) { 
        if( _span == _buffer.sz ) return nullptr;
    
    #if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
        _T* ptr = new ( ( void* )&_buffer[ _tail++ ] ) _T{ ( _T&& )arg };
    #else 
        _T* ptr = &_buffer[ _tail++ ];
        *ptr = _T( ( _T&& )arg );
    #endif
        if( _tail >= _buffer.sz ) _tail = 0; 

        ++_span;
        return ptr;
    }

    int pop( void ) {
        if( this->is_empty() ) return -1;

        _buffer[ _head++ ].~_T();
        if( _head >= _buffer.sz ) _head = 0;

        --_span;
        return 0;
    }

    _WJP_forceinline void clear( void ) {
        while( this->pop() == 0 );
    }

    _WJP_forceinline _T& front( void ) {
        return _buffer[ _head ];
    }

    _WJP_forceinline void for_each( fe_cb_t cb ) {
        int head = _head;
        int span = _span;

        while( span-- >= 0 ) {
            cb( _buffer[ head++ ] );
            if( head >= _buffer.sz ) head = 0;
        }
    }
};


// template< typename _T > struct _WJP_INTERLOCKED_QUEUE {

// #if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
//     std::deque< _T >   _deq;
//     std::mutex         _mtx;

//     _WJP_forceinline bool empty() { return _deq.empty(); }
//     template< typename ...Args > _WJP_forceinline _T& emplace( Args&&... args ) { std::unique_lock< decltype( _mtx ) > lock{ _mtx }; return _deq.emplace_back( std::forward< Args >( args )... ); }
//     _WJP_forceinline void pop() { std::unique_lock< decltype( _mtx ) > lock{ _mtx }; _deq.pop_front(); }
//     _WJP_forceinline void clear() { std::unique_lock< decltype( _mtx ) > lock{ _mtx }; _deq.clear(); }
//     _WJP_forceinline _T& front() { return _deq.front(); }

//     _WJP_forceinline void for_each( std::function< void( _T& ) > cb ) { std::unique_lock< decltype( _mtx ) > lock{ _mtx }; for( auto& t : _deq ) cb( t ); }
// #else
//     _WJP_forceinline bool empty() { _WJP_EMPTY_FUNCTION; }
//     template< typename ...Args > _WJP_forceinline _T& emplace( Args&&... args ) { _WJP_EMPTY_FUNCTION; }
//     _WJP_forceinline void pop() { _WJP_EMPTY_FUNCTION; }
//     _WJP_forceinline void clear() { _WJP_EMPTY_FUNCTION; }
//     _WJP_forceinline _T& front() { _WJP_EMPTY_FUNCTION; }

//     _WJP_forceinline void for_each( std::function< void( _T& ) > cb ) { _WJP_EMPTY_FUNCTION; }
// #endif

// };
