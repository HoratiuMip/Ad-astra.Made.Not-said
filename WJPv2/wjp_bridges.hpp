/*===== Warp Joint Protocol v2 - Bridges - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> 
|
======*/
#include "wjp_core.hpp"


/**
 * @brief Bridge over interlocked BYTE/WORD/DWORD/QWORD, wait/notify-ready where applicable. 
 */
template< typename _T > struct WJP_BRIDGE_Interlocked {
    /**
     * @brief Atomically read the underlying memory, using the given memory order where applicable.
     * @returns The atomically read value.
     */
    virtual _T read( WJP_MEM_ORD_DFT_ARG ) = 0;
    /**
     * @brief Atomically write the underlying memory, using the given memory order where applicable.
     */
    virtual void write( _T flag, WJP_MEM_ORD_DFT_ARG ) = 0;

    /**
     * @brief Atomically adds to the underlying memory.
     * @returns The value preceding the addition.
     */
    virtual _T fetch_add( _T arg, WJP_MEM_ORD_DFT_ARG ) = 0;

    /**
     * @brief Called to signal one thread waiting on the underlying memory via hold().
     */
    virtual void sig_one( void ) = 0;
    /**
     * @brief Called to signal all threads waiting on the underlying memory via hold().
     */
    virtual void sig_all( void ) = 0;

    /**
     * @brief Block the current thread if the underlying memory compares equal with the argument, until signaled.
     */
    virtual void hold( _T flag ) = 0;
};

/**
 * @brief Bridge over MUTEX.
 */
struct WJP_BRIDGE_Mutex {
    /**
     * @brief Acquire the mutex.
     */
    virtual void acquire( void ) = 0;

    /**
     * @brief Release the mutex.
     */
    virtual void release( void ) = 0;

    /**
     * @brief Try to acquire the mutex.
     * @returns Wether the mutex was acquired.
     */
    virtual bool try_acquire( void ) = 0;
};

/**
 * @brief Bridge over First-In First-Out queue.
 */
template< typename _T > struct WJP_BRIDGE_Queue {
#if defined( _WJP_SEMANTICS_STL_FUNCTION )
    typedef   std::function< void( _T& ) >   for_each_cb_t;
#else
    typedef   void ( *for_each_cb_t )( _T& );
#endif

    /**
     * @brief Returns wether the queue is empty.
     */
    virtual bool is_empty( void ) const = 0;

    /**
     * @brief Push at the back of the queue.
     * @returns Pointer to the pushed value.
     */
    virtual _T* push( _T&& arg ) = 0;
    /**
     * @brief Pop from the front of the queue.
     * @returns Pointer to the immediately following value, or NULL if queue empty.
     */
    virtual _T* pop( void ) = 0;
    /**
     * @brief Clear the entire queue.
     * @returns The count of popped values, effectively the queue size before the call.
     */
    virtual int clear( void ) = 0;

    /**
     * @brief Pointer to the front value of the queue.
     */
    virtual _T* front( void ) = 0;
    /**
     * @brief Execute given callback for each value in the queue.
     */
    virtual void for_each( for_each_cb_t cb ) = 0;
};

/**
 * @brief Bridge over inter-endpoints communication.
 */
struct WJP_BRIDGE_InterMech {
    /**
     * @brief Called to send bytes over the wire.
     * @warning This function MUST guarantee that all the requested bytes are sent, or return an error code elsewise. NO in-between.
     * @returns The count of requested bytes to send. Negative for errors, zero for connection reset.
     */
    virtual int send( WJP_MDsc_v mdsc, int flags, void* arg ) = 0;

    /**
     * @brief Called to receive bytes over the wire.
     * @warning This function MUST guarantee that all the requested bytes are receive, or return an error code elsewise. NO in-between.
     * @returns The count of requested bytes to receive. Negative for errors, zero for connection reset.
     */
    virtual int recv( WJP_MDsc_v mdsc, int flags, void* arg ) = 0;
};
