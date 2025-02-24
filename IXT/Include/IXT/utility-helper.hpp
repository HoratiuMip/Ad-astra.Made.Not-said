#pragma once
/*
*/

#include <IXT/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



template< typename Tx, typename Tl, typename Tu >
inline DWORD is_between( const Tx& x, const Tl& l, const Tu& u ) {
    if( x > u ) return false; if( x < l ) return false; return true;
}

template< typename Tx, typename Tl, typename Tu >
inline DWORD is_between_ex( const Tx& x, const Tl& l, const Tu& u ) {
    if( x > u ) return 1; if( x < l ) return -1; return 0;
}



};
