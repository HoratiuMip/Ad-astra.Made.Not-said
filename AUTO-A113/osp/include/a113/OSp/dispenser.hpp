#pragma once
/**
 * @file: OSp/dispenser.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/core.hpp>

namespace a113 {


enum DispenserMode_ {
    DispenserMode_Lock, DispenserMode_Drop, DispenserMode_Swap, DispenserMode_ReverseSwap
};

template< typename _T_, bool _IS_CONTROL_ > struct _dispenser_acquire;

enum DispenserFlags_ {
    DispenserFlags_SwapMode_CopyWhenReverseWatchAcquire = ( 1 << 0 )
};

struct dispenser_config_t {
    uint32_t   flags   = 0x0;
};

template< typename _T_ > class Dispenser {
public:
    template< typename, bool > friend struct _dispenser_acquire;

public:
    using dispensed_t = _T_;
    using watch_t     = _dispenser_acquire< _T_, false >;
    using control_t   = _dispenser_acquire< _T_, true >;

public:
    Dispenser( const DispenserMode_ mode_ ) : _mode{ mode_ } {
        switch( _mode ) {
            case DispenserMode_Lock: {
                _M_.lock.block = HVec< _T_ >::make();
                new ( &_M_.lock.mtx ) std::shared_mutex{};
            break; }
            case DispenserMode_Drop: {
                _M_.drop.block = HVec< _T_ >::make();
            break; }
            case DispenserMode_Swap: [[fallthrough]];
            case DispenserMode_ReverseSwap: {
                _M_.swap.blocks[ 0x0 ] = HVec< _T_ >::make();
                _M_.swap.blocks[ 0x1 ] = HVec< _T_ >::make();
                new ( &_M_.swap.mtxs[ 0x0 ] ) std::shared_mutex{};
                new ( &_M_.swap.mtxs[ 0x1 ] ) std::shared_mutex{};
                _M_.swap.ctl_idx.store( 0x0, std::memory_order_release );
            break; }
        }
    }

    ~Dispenser( void ) {
        switch( _mode ) {
            case DispenserMode_Lock: {
                _M_.lock.~_lock_mode_t();
            break; }
            case DispenserMode_Drop: {
                _M_.drop.~_drop_mode_t();
            break; }
            case DispenserMode_Swap: [[fallthrough]]; 
            case DispenserMode_ReverseSwap: {
                _M_.swap.~_swap_mode_t();
            break; }
        }
    }

_A113_PROTECTED:
    DispenserMode_       _mode;
    dispenser_config_t   _config;

    union _M_t { _M_t( void ) {} ~_M_t( void ) {}
        struct _lock_mode_t {
            HVec< _T_ >         block;
            std::shared_mutex   mtx;
        } lock;
        struct _drop_mode_t {
            HVec< _T_ >   block;
        } drop;
        struct _swap_mode_t {
            HVec< _T_ >            blocks[ 2 ];
            std::shared_mutex      mtxs[ 2 ];
            std::atomic_uint8_t    ctl_idx;
        } swap;
    } _M_;

public:
    A113_inline DispenserMode_ switch_swap_mode( DispenserMode_ swap_mode_, std::function< void( dispenser_config_t& ) > modify_config_ ) {
        std::lock_guard lock_1{ _M_.swap.mtxs[ 0x0 ] };
        std::lock_guard lock_2{ _M_.swap.mtxs[ 0x1 ] };
       
        if( modify_config_ ) modify_config_( _config );

        return std::exchange( _mode, swap_mode_ );
    }

public:
    A113_inline DispenserMode_ mode( void ) const { return _mode; }

    A113_inline _dispenser_acquire< _T_, false > watch( void ); 
    A113_inline _dispenser_acquire< _T_, true > control( void ); 

public:
    [[nodiscard]] A113_inline HVec< _T_ > hold_latest( void ) {
        switch( _mode ) {
            case DispenserMode_Lock: return _M_.lock.block;
            case DispenserMode_Drop: return _M_.drop.block;
            case DispenserMode_Swap: [[fallthrough]];
            case DispenserMode_ReverseSwap: return _M_.swap.blocks[ _M_.swap.ctl_idx.load( std::memory_order_relaxed ) ];
        }
        A113_UNREACHABLE;
    }

};

template< typename _T_, bool _IS_CONTROL_ > struct _dispenser_acquire {
public:
    [[gnu::hot]] _dispenser_acquire( Dispenser< _T_ >& disp_ ) : _disp{ &disp_ } {
        switch( _disp->_mode ) {
            case DispenserMode_Lock: {
                if constexpr( _IS_CONTROL_ ) {
                    _disp->_M_.lock.mtx.lock();
                } else {
                    _disp->_M_.lock.mtx.lock_shared();
                }
            break; }
            case DispenserMode_Drop: {
                if constexpr( _IS_CONTROL_ ) {
                    _M_.drop.block = HVec< _T_ >::make();
                } else {
                    _M_.drop.block = _disp->_M_.drop.block;
                }
            break; }
            case DispenserMode_Swap: {
                _M_.swap.ctl_idx = _disp->_M_.swap.ctl_idx.load( std::memory_order_acquire ); 
                
                if constexpr( _IS_CONTROL_ ) {
                    _M_.swap.ctl_idx ^= 0x1;
                    _disp->_M_.swap.mtxs[ _M_.swap.ctl_idx ].lock(); 
                } else {
                    _disp->_M_.swap.mtxs[ _M_.swap.ctl_idx ].lock_shared();
                }

                _M_.swap.block = _disp->_M_.swap.blocks[ _M_.swap.ctl_idx ].get();
            break; }
            case DispenserMode_ReverseSwap: {
                if constexpr( _IS_CONTROL_ ) {
                    _M_.swap.ctl_idx = _disp->_M_.swap.ctl_idx.load( std::memory_order_acquire );
                    _disp->_M_.swap.mtxs[ _M_.swap.ctl_idx ].lock();
                } else {
                    if( _disp->_config.flags & DispenserFlags_SwapMode_CopyWhenReverseWatchAcquire ) {
                        auto crt_ctl_idx = _disp->_M_.swap.ctl_idx.load( std::memory_order_acquire );
                        auto sis_ctl_idx = crt_ctl_idx ^ 0x1;
                        std::lock_guard lock_sis{ _disp->_M_.swap.mtxs[ sis_ctl_idx ] };
                        std::lock_guard lock_crt{ _disp->_M_.swap.mtxs[ crt_ctl_idx ] };
                        _disp->_M_.swap.blocks[ sis_ctl_idx ]->operator=( *_disp->_M_.swap.blocks[ crt_ctl_idx ] );
                    }

                    _M_.swap.ctl_idx = _disp->_M_.swap.ctl_idx.fetch_xor( 0x1, std::memory_order_acquire );
                    _disp->_M_.swap.mtxs[ _M_.swap.ctl_idx ].lock_shared();
                }

                _M_.swap.block = _disp->_M_.swap.blocks[ _M_.swap.ctl_idx ].get();
            break; }
        }
    }

    _dispenser_acquire( const _dispenser_acquire& ) = delete;

    _dispenser_acquire( _dispenser_acquire&& other_ )
    : _disp{ std::exchange( other_._disp, nullptr ) }
    {
        switch( _disp->_mode ) {
            case DispenserMode_Lock: {
            break; }
            case DispenserMode_Drop: {
                _M_.drop.block = std::move( other_._M_.drop.block );
            break; }
            case DispenserMode_Swap: [[fallthrough]];
            case DispenserMode_ReverseSwap: {
                _M_.swap.block = std::exchange( other_._M_.swap.block, nullptr );
            break; }
        }
    }

    ~_dispenser_acquire( void ) {
        this->release();
    }

public:
    [[gnu::hot]] void release( void ) {
        if( not _disp ) return;
        switch( _disp->_mode ) {
            case DispenserMode_Lock: {
                if constexpr( _IS_CONTROL_ ) {
                    _disp->_M_.lock.mtx.unlock();
                } else {
                    _disp->_M_.lock.mtx.unlock_shared();
                }
            break; }
            case DispenserMode_Drop: {
                if constexpr( _IS_CONTROL_ ) {
                    _disp->_M_.drop.block = std::move( _M_.drop.block );
                } else {
                    _M_.drop.block.reset();
                }
            break; }
            case DispenserMode_Swap: {
                if constexpr( _IS_CONTROL_ ) {
                    _disp->_M_.swap.ctl_idx.store( _M_.swap.ctl_idx, std::memory_order_release );
                    _disp->_M_.swap.mtxs[ _M_.swap.ctl_idx ].unlock();
                } else {
                    _disp->_M_.swap.mtxs[ _M_.swap.ctl_idx ].unlock_shared();
                }
                _M_.swap.block = nullptr;
            break; }
            case DispenserMode_ReverseSwap: {
                if constexpr( _IS_CONTROL_ ) {
                    _disp->_M_.swap.mtxs[ _M_.swap.ctl_idx ].unlock();
                } else {
                    _disp->_M_.swap.mtxs[ _M_.swap.ctl_idx ].unlock_shared();
                }
                _M_.swap.block = nullptr;
            break; }
        }
        _disp = nullptr;
    }

    A113_inline void commit( void ) { return this->release(); }

_A113_PROTECTED:
    Dispenser< _T_ >*   _disp;
    
    union _M_t { _M_t( void ) {} ~_M_t( void ) {}
        struct _lock_mode_t {
        } lock;
        struct _drop_mode_t {
            HVec< _T_ >   block;
        } drop;
        struct _swap_mode_t {
            _T_*      block;
            uint8_t   ctl_idx;
        } swap;
    } _M_;

public:
    A113_inline _T_* get( void ) {
        switch( _disp->_mode ) {
            case DispenserMode_Lock: return _disp->_M_.lock.block.get();
            case DispenserMode_Drop: return _M_.drop.block.get();
            case DispenserMode_Swap: [[fallthrough]];
            case DispenserMode_ReverseSwap: return _M_.swap.block;
        }
        return nullptr;
    }

public:
    A113_inline _T_* operator -> ( void ) { return this->get(); }
    A113_inline _T_& operator * ( void ) { return *this->get(); }

};

template< typename _T_ > _dispenser_acquire< _T_, false > Dispenser< _T_ >::watch( void ) { return _dispenser_acquire< _T_, false >{ *this }; }
template< typename _T_ > _dispenser_acquire< _T_, true > Dispenser< _T_ >::control( void ) { return _dispenser_acquire< _T_, true >{ *this }; }

template< typename _T_ > using dispenser_watch   = _dispenser_acquire< _T_, false >;
template< typename _T_ > using dispenser_control = _dispenser_acquire< _T_, true >;


}