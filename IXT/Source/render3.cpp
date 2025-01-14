/*
*/

#include <IXT/render3.hpp>

namespace _ENGINE_NAMESPACE {



DWORD RenderCluster3::push_shader( HVEC< Shader3 > shader, bool owr, _ENGINE_COMMS_ECHO_RT_ARG ) {
    std::unique_lock lock{ _shaders.map_mtx };
    auto& record = _shaders.map[ shader->_name ];
    lock.unlock();

    if( !owr && record->glidx() != 0 ) {
        echo( this, ECHO_LEVEL_ERROR ) << "Pushing shader \"" << record->_name << "\" overwrites existing one. Aborted.";
        return -1;
    }

    record.vector( std::move( shader ) );
    return 0;
}

DWORD RenderCluster3::pop_shader( std::variant< const char*, XtDx > id, _ENGINE_COMMS_ECHO_RT_ARG ) {
    std::unique_lock lock{ _shaders.map_mtx };

    switch( id.index() ) {
        case 0: return _shaders.map.erase( std::get< 0 >( id ) ) == 1 ? 0 : -1;

        case 1: {
            auto itr = std::find_if( _shaders.map.begin(), _shaders.map.end(), [ &id ] ( const auto& pair ) -> bool {
                return pair.second->xtdx() == std::get< 1 >( id );
            } );

            if( itr == _shaders.map.end() ) return -1;
            _shaders.map.erase( itr );
            return 0;
        }
    }

    return -1;
}

HVEC< Shader3 > RenderCluster3::query_for_shader( const char* name, bool hot, _ENGINE_COMMS_ECHO_RT_ARG ) {
    if( !hot ) goto l_cold;
    
l_hot:
    { 
    std::unique_lock lock{ _shaders.map_mtx };
    HVEC< Shader3 > ret = _shaders.map[ name ];
    lock.unlock();

    if( ret == nullptr ) ret.vector( HVEC< Shader3 >::allocc() );
    return ret; 
    }
l_cold:  
    {
    std::unique_lock lock{ _shaders.map_mtx };
    auto record = _shaders.map.find( name );
    if( record == _shaders.map.end() ) return nullptr;
    lock.unlock();

    return record->second;
    }
}

HVEC< Shader3 >* RenderCluster3::deep_query_for_shader( const char* name, bool hot, _ENGINE_COMMS_ECHO_RT_ARG ) {
    if( !hot ) goto l_cold;

l_hot:
    {
    std::unique_lock lock{ _shaders.map_mtx };
    HVEC< Shader3 >* ret = &_shaders.map[ name ];
    lock.unlock();

    return ret;
    }
l_cold:
    {
    std::unique_lock lock{ _shaders.map_mtx };
    auto record = _shaders.map.find( name );
    if( record == _shaders.map.end() ) return nullptr;
    lock.unlock();

    return &record->second;
    }

}





};
