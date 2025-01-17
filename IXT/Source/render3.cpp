/*
*/

#include <IXT/render3.hpp>

namespace _ENGINE_NAMESPACE {
 


#define _ENGINE_RENDER_CLUSTER3_PUSH_REZERVATION_FUNC( func_name, station, type, name_field ) \
DWORD RenderCluster3::func_name( HVEC< type > rezervation, bool owr, _ENGINE_COMMS_ECHO_RT_ARG ) { \
    std::unique_lock lock{ station.map_mtx }; \
    auto& record = station.map[ rezervation->name_field ]; \
    lock.unlock(); \
    if( !owr && record->glidx() != 0 ) { \
        echo( this, ECHO_LEVEL_ERROR ) << "Pushing rezervation \"" << record->name_field << "\" overwrites existing one. Aborted."; \
        return -1; \
    } \
    record.vector( std::move( rezervation ) ); \
    return 0; \
}

#define _ENGINE_RENDER_CLUSTER3_POP_REZERVATION_FUNC( func_name, station ) \
DWORD RenderCluster3::func_name( std::variant< const char*, XtDx > id, _ENGINE_COMMS_ECHO_RT_ARG ) { \
    std::unique_lock lock{ station.map_mtx }; \
    switch( id.index() ) { \
        case 0: return station.map.erase( std::get< 0 >( id ) ) == 1 ? 0 : -1; \
        case 1: { \
            auto itr = std::find_if( station.map.begin(), station.map.end(), [ &id ] ( const auto& pair ) -> bool { \
                return pair.second->xtdx() == std::get< 1 >( id ); \
            } ); \
            if( itr == station.map.end() ) return -1; \
            station.map.erase( itr ); \
            return 0; \
        } \
    } \
    return -1; \
}

#define _ENGINE_RENDER_CLUSTER3_QUERY_REZERVATION_FUNC( func_name, station, type ) \
HVEC< type > RenderCluster3::func_name( const char* name, bool hot, _ENGINE_COMMS_ECHO_RT_ARG ) { \
    if( !hot ) goto l_cold; \
l_hot: \
    { \
    std::unique_lock lock{ station.map_mtx }; \
    HVEC< type >& ret = station.map[ name ]; \
    lock.unlock(); \
    if( ret == nullptr ) ret.vector( HVEC< type >::allocc() ); \
    return ret;  \
    } \
l_cold: \
    { \
    std::unique_lock lock{ station.map_mtx }; \
    auto record = station.map.find( name ); \
    if( record == station.map.end() ) return nullptr; \
    lock.unlock(); \
    return record->second; \
    } \
}

#define _ENGINE_RENDER_CLUSTER3_DEEP_QUERY_REZERVATION_FUNC( func_name, station, type ) \
HVEC< type >* RenderCluster3::func_name( const char* name, bool hot, _ENGINE_COMMS_ECHO_RT_ARG ) { \
    if( !hot ) goto l_cold; \
l_hot: \
    { \
    std::unique_lock lock{ station.map_mtx }; \
    HVEC< type >* ret = &station.map[ name ]; \
    lock.unlock(); \
    return ret; \
    } \
l_cold: \
    { \
    std::unique_lock lock{ station.map_mtx }; \
    auto record = station.map.find( name ); \
    if( record == station.map.end() ) return nullptr; \
    lock.unlock(); \
    return &record->second; \
    } \
}

_ENGINE_RENDER_CLUSTER3_PUSH_REZERVATION_FUNC( push_shader, _shaders, Shader3, _name )
_ENGINE_RENDER_CLUSTER3_POP_REZERVATION_FUNC( pop_shader, _shaders )
_ENGINE_RENDER_CLUSTER3_QUERY_REZERVATION_FUNC( query_for_shader, _shaders, Shader3 )
_ENGINE_RENDER_CLUSTER3_DEEP_QUERY_REZERVATION_FUNC( deep_query_for_shader, _shaders, Shader3 ) 

_ENGINE_RENDER_CLUSTER3_PUSH_REZERVATION_FUNC( push_pipe, _pipes, ShadePipe3, _name )
_ENGINE_RENDER_CLUSTER3_POP_REZERVATION_FUNC( pop_pipe, _pipes )
_ENGINE_RENDER_CLUSTER3_QUERY_REZERVATION_FUNC( query_for_pipe, _pipes, ShadePipe3 )
_ENGINE_RENDER_CLUSTER3_DEEP_QUERY_REZERVATION_FUNC( deep_query_for_pipe, _pipes, ShadePipe3 ) 



};
