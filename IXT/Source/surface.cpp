/*
*/
#include <IXT/surface.hpp>



namespace _ENGINE_NAMESPACE {



#if defined( _ENGINE_UNIQUE_SURFACE )
    template< typename ...Keys >
    size_t SurfKey::down_any( Keys... keys ) {
        return Surface::get()->down_any( keys... );
    }

    template< typename ...Keys >
    size_t SurfKey::down_tgl( Keys... keys ) {
        return Surface::get()->down_tgl( keys... );
    }

    template< typename ...Keys >
    bool SurfKey::down_all( Keys... keys ) {
        return Surface::get()->down_all( keys... );
    }

    bool SurfKey::down( Key key ) {
        return Surface::get()->down( key );
    }
#endif



};