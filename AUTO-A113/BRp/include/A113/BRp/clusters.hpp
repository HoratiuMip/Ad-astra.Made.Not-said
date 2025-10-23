#pragma once
/**
 * @file: BRp/clusters.hpp
 * @brief:
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/descriptor.hpp>

namespace A113 { namespace BRp {


template< typename _T > class Cluster_circular {
    BUFFER   _mdsc   = {};
    int      _head     = 0;
    int      _tail     = 0;
    int      _span     = 0;

    A113_inline void bind_mdsc( BUFFER mdsc_ ) {
        _mdsc = mdsc_;
    }

    A113_inline bool is_empty( void ) const {
        return _span == 0;
    }

    _T* push_back( _T&& arg ) { 
        if( _span == _mdsc.n ) return nullptr;
    
        _T* ptr = &_mdsc.ptr[ _tail++ ];
        *ptr = _T{ ( _T&& )arg };
    
        if( _tail >= _mdsc.n ) _tail = 0; 

        ++_span;
        return ptr;
    }

    _T* pop_front( void ) {
        if( this->is_empty() ) return nullptr;

        _mdsc.ptr[ _head++ ].~_T();
        if( _head >= _mdsc.n ) _head = 0;

        --_span;
        return this->is_empty() ? nullptr : this->front();
    }

    void pop_back( void ) {
        if( this->is_empty() ) return;

        --_tail; if( _tail < 0 ) _tail = _mdsc.n - 1;

        _mdsc.ptr[ _tail ].~_T();
        
        --_span;
    }

    A113_inline int clear( void ) {
        int count = 0;
        while( this->pop() != nullptr ) ++count;
        return count;
    }

    A113_inline _T* front( void ) {
        return &_mdsc.ptr[ _head ];
    }
};


} };
