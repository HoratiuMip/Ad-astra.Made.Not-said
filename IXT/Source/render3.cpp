/*
*/

#include <IXT/render3.hpp>

namespace _ENGINE_NAMESPACE {



DWORD RenderCluster3::push_shader( HVEC< Shader3 > shader, _ENGINE_COMMS_ECHO_RT_ARG ) {
    auto& record = _shaders[ shader->_name ];

    if( record->glidx() != 0 ) {
        echo( this, ECHO_LEVEL_ERROR ) << "Pushing shader \"" << record->_name << "\" overwrites existing one. Aborted.";
        return -1;
    }

    record.vector( std::move( shader ) );
    return 0;
}

DWORD RenderCluster3::pop_shader( const char* name, _ENGINE_COMMS_ECHO_RT_ARG ) {
    return _shaders.erase( name ) == 1 ? 0 : -1;
}

HVEC< Shader3 > RenderCluster3::search_shader( const char* name, _ENGINE_COMMS_ECHO_RT_ARG ) {
    auto record = _shaders.find( name );

    if( record == _shaders.end() ) return nullptr;

    return record->second;
}



};
