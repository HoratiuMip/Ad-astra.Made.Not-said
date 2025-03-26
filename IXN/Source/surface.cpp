/*
*/
#include <IXN/surface.hpp>



namespace _ENGINE_NAMESPACE {



#if defined( _ENGINE_UNIQUE_SURFACE )
    template< typename ...Keys >
    DWORD SurfKey::down_any( Keys... keys ) {
        return Surface::get()->down_any( keys... );
    }

    template< typename ...Keys >
    DWORD SurfKey::down_tgl( Keys... keys ) {
        return Surface::get()->down_tgl( keys... );
    }

    template< typename ...Keys >
    DWORD SurfKey::down_all( Keys... keys ) {
        return Surface::get()->down_all( keys... );
    }

    DWORD SurfKey::down( Key key ) {
        return Surface::get()->down( key );
    }
#endif



};