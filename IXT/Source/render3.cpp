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

_ENGINE_RENDER_CLUSTER3_PUSH_REZERVATION_FUNC( push_pipe, _pipes, ShaderPipe3, _name )
_ENGINE_RENDER_CLUSTER3_POP_REZERVATION_FUNC( pop_pipe, _pipes )
_ENGINE_RENDER_CLUSTER3_QUERY_REZERVATION_FUNC( query_for_pipe, _pipes, ShaderPipe3 )
_ENGINE_RENDER_CLUSTER3_DEEP_QUERY_REZERVATION_FUNC( deep_query_for_pipe, _pipes, ShaderPipe3 ) 


HVEC< Shader3 > RenderCluster3::make_or_pull_shader_from_path( const std::filesystem::path& path, SHADER3_PHASE phase, _ENGINE_COMMS_ECHO_RT_ARG ) {
    HVEC< Shader3 > ret{};

    Shader3 shader{ path, phase, 
        [ &ret, &echo, this ] ( SHADER3_DIRECTIVE directive, const std::string& arg, void* ptr ) -> DWORD {
            switch( directive ) {
                case SHADER3_DIRECTIVE_NAME: {
                    ret.vector( this->query_for_shader( arg.c_str(), true, echo ) );
                    if( ret->glidx() != 0 ) return -1;
                break; }

                case SHADER3_DIRECTIVE_INCLUDE: break;

                default: echo( this, ECHO_LEVEL_WARNING ) << "No support for directive ( " << directive << " ), of argument \"" << arg << "\".";
            }
            return 0;
        }, echo 
    };
    
    if( ret->glidx() == 0 ) 
        new ( ret.get() ) Shader3{ std::move( shader ) };

    return ret;
}

HVEC< ShaderPipe3 > RenderCluster3::make_or_pull_pipe_from_ptr_arr( Shader3* shaders[ 5 ], _ENGINE_COMMS_ECHO_RT_ARG ) {
    HVEC< ShaderPipe3 > ret{};

    ShaderPipe3 pipe{ shaders,
        [ &ret, &echo, this ] ( SHADER_PIPE3_ATTR attr, const std::string& arg, void* ptr ) -> DWORD {
            switch( attr ) {
                case SHADER_PIPE3_ATTR_NAME: {
                    ret.vector( this->query_for_pipe( arg.c_str(), true, echo ) );
                    if( ret->glidx() != 0 ) return -1;
                break; }

                default: echo( this, ECHO_LEVEL_WARNING ) << "No support for attribute ( " << attr << " ), of argument \"" << arg << "\".";
            }
            return 0;
        }, echo 
    };

    if( ret->glidx() == 0 )
        new( ret.get() ) ShaderPipe3{ std::move( pipe ) };

    return ret;
}

HVEC< ShaderPipe3 > RenderCluster3::make_or_pull_pipe_from_prefixed_path( const std::filesystem::path& path, _ENGINE_COMMS_ECHO_RT_ARG ) {
    struct PHASE_INFO {
        int             idx;
        SHADER3_PHASE   phase;
        const char*     str;
    };

    Shader3* shaders_ptrs[ 5 ]; memset( shaders_ptrs, 0, sizeof( shaders_ptrs ) );

    for( auto phase : std::initializer_list< PHASE_INFO >{ 
        { 0, SHADER3_PHASE_VERTEX,    ".vert" }, 
        { 1, SHADER3_PHASE_TESS_CTRL, ".tesc" }, 
        { 2, SHADER3_PHASE_TESS_EVAL, ".tese" }, 
        { 3, SHADER3_PHASE_GEOMETRY,  ".geom" }, 
        { 4, SHADER3_PHASE_FRAGMENT,  ".frag" } 
    } ) {
        std::filesystem::path phase_path{ path };
        phase_path += phase.str;

        if( !std::filesystem::exists( phase_path ) ) continue;
        
        shaders_ptrs[ phase.idx ] = this->make_or_pull_shader_from_path( phase_path, phase.phase, echo ).get();
    }

    return this->make_or_pull_pipe_from_ptr_arr( shaders_ptrs, echo );
}



};
