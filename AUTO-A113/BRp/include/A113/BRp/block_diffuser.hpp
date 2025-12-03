#pragma once
/**
 * @file: BRp/block_difusser.hpp
 * @brief:
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/brp/descriptor.hpp>

namespace a113 { namespace clst {


#ifdef _BLOCK_PTR
    #error "[A113] - _BLOCK_PTR already defined and cannot be used by clst::Block_diffuser."
#else
    #define _BLOCK_PTR( ptr ) (*((block_t**)(ptr)))
#endif

template< typename _T > class Block_diffuser {
public:
    struct block_t {
        block_t*   _blink    = nullptr;
        block_t*   _flink    = nullptr;
        _T         content   = {};
    };

_A113_PROTECTED:
    block_t*   _memory   = nullptr;
    size_t     _cap      = 0x0;

    block_t*   _head     = nullptr;
    block_t*   _inj      = nullptr;
    size_t     _sz       = 0x0;

public:
    void bind_memory( block_t* memory_, size_t cap_ ) {
        _memory = memory_;
        _cap    = cap_;

        _head = nullptr;
        _inj  = _memory;
        _sz   = 0x0;

        size_t b;
        for( b = 0x0; b < _cap - 1; ++b ) _BLOCK_PTR( &_memory[ b ] ) = &_memory[ b ] + 0x1;
        _BLOCK_PTR( &_memory[ b ] ) = 0x0;
    }

public:
    template< typename ...Args >
    _T* inject( Args&&... args_ ) {
        if( _sz == _cap ) return nullptr;

        block_t* bck = ( nullptr == _head || _inj < _head ) ? nullptr : _inj - 0x1; 
        block_t* fwd;
        if( nullptr != bck ) {
            fwd = bck->_flink;
            bck->_flink = _inj;
        } else {
            fwd = _head;
            _head = _inj;
        }
        if( nullptr != fwd ) fwd->_blink = _inj;

        block_t* next_inj = _BLOCK_PTR( _inj );

        block_t* ptr = new ( _inj ) block_t{
            ._blink = bck,
            ._flink = nullptr,
            .content{ args_... }
        };

        _inj = next_inj;
        ++_sz;
        return &ptr->content;
    }

    void eject( block_t* blk_ ) {
        if( nullptr != blk_->_blink ) blk_->_blink->_flink = blk_->_flink;
        if( nullptr != blk_->_flink ) blk_->_flink->_blink = blk_->_blink;
        if( blk_ == _head ) _head = blk_->_flink;

        blk_->content.~_T();

        if( _inj < blk_ ) { _BLOCK_PTR( _inj ) = blk_; }
        else { _BLOCK_PTR( blk_ ) = _inj; _inj = blk_; }
        --_sz;
    }

};


} };
#undef _BLOCK_PTR
