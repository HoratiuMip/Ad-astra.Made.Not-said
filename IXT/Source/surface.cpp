/*
*/
#include <IXT/surface.hpp>



namespace _ENGINE_NAMESPACE {



#if defined( _ENGINE_UNIQUE_SURFACE )
    template< typename ...Keys >
    size_t SurfKey::any_down( Keys... keys ) {
        return Surface::get()->any_down( keys... );
    }

    template< typename ...Keys >
    size_t SurfKey::tgl_down( Keys... keys ) {
        return Surface::get()->tgl_down( keys... );
    }

    template< typename ...Keys >
    bool SurfKey::all_down( Keys... keys ) {
        return Surface::get()->all_down( keys... );
    }

    bool SurfKey::down( Key key ) {
        return Surface::get()->down( key );
    }
#endif



};