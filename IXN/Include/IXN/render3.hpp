#pragma once
/*
*/

#include <IXN/descriptor.hpp>
#include <IXN/surface.hpp>
#include <IXN/hyper-vector.hpp>

#if defined( _ENGINE_GL_OPEN_GL )

namespace _ENGINE_NAMESPACE {



class Shader3;
class ShaderPipe3;

enum SHADER3_PHASE : DWORD {
    SHADER3_PHASE_VERTEX    = GL_VERTEX_SHADER,
    SHADER3_PHASE_TESS_CTRL = GL_TESS_CONTROL_SHADER,
    SHADER3_PHASE_TESS_EVAL = GL_TESS_EVALUATION_SHADER,
    SHADER3_PHASE_GEOMETRY  = GL_GEOMETRY_SHADER,
    SHADER3_PHASE_FRAGMENT  = GL_FRAGMENT_SHADER,

    _SHADER3_PHASE_FORCE_DWORD = 0x7F'FF'FF'FF
};



class RenderCluster3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "RenderCluster3" );

_ENGINE_PROTECTED:
    struct _SHADERS {
        std::mutex                                 map_mtx;
        std::map< std::string, HVEC< Shader3 > >   map;
    } _shaders;
    
    struct _PIPES {
        std::mutex                                    map_mtx;
        std::map< std::string, HVEC< ShaderPipe3 > >   map;
    } _pipes;

public:
    DWORD push_shader( HVEC< Shader3 > shader, bool owr, _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    DWORD pop_shader( std::variant< const char*, XtDx > id, _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    HVEC< Shader3 > query_for_shader( const char* name, bool hot, _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    HVEC< Shader3 >* deep_query_for_shader( const char* name, bool hot, _ENGINE_COMMS_ECHO_NO_DFT_ARG );

public:
    DWORD push_pipe( HVEC< ShaderPipe3 > pipe, bool owr, _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    DWORD pop_pipe( std::variant< const char*, XtDx > id, _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    HVEC< ShaderPipe3 > query_for_pipe( const char* name, bool hot, _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    HVEC< ShaderPipe3 >* deep_query_for_pipe( const char* name, bool hot, _ENGINE_COMMS_ECHO_NO_DFT_ARG );

public:
    HVEC< Shader3 > make_or_pull_shader_from_path( const std::filesystem::path& path, SHADER3_PHASE phase, _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    HVEC< ShaderPipe3 > make_or_pull_pipe_from_ptr_arr( Shader3* shaders[ 5 ], _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    HVEC< ShaderPipe3 > make_pipe_from_prefixed_path( const std::filesystem::path& path, _ENGINE_COMMS_ECHO_NO_DFT_ARG );
    HVEC< ShaderPipe3 > make_or_pull_pipe_from_prefixed_path( const std::filesystem::path& path, _ENGINE_COMMS_ECHO_NO_DFT_ARG );

};
inline RenderCluster3 GME_render_cluster3{};



enum SHADER3_DIRECTIVE : DWORD {
    SHADER3_DIRECTIVE_INCLUDE, SHADER3_DIRECTIVE_NAME,

    _SHADER3_DIRECTIVE_FORCE_DWORD = 0x7F'FF'FF'FF
};
#define _ENGINE_SHADER3_EXEC_DIRECTIVE_CALLBACK( func, dir, arg, ptr ) ( create_gl_shader = ( func ? ( ( directive_cb_status = std::invoke( func, dir, arg, ptr ) ) == 0 ) : true ) )

class Shader3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Shader3" );

public:
    friend class RenderCluster3;

public:
    typedef   std::function< DWORD( SHADER3_DIRECTIVE, const std::string&, void* ) >   directive_callback_t;

public:
    Shader3() = default;

    Shader3( 
        const std::filesystem::path& path, 
        SHADER3_PHASE                phase, 
        directive_callback_t         directive_cb,
        _ENGINE_COMMS_ECHO_ARG 
    ) {
        std::string source;
        std::string line;
        DWORD       status              = 0;
        DWORD       directive_cb_status = 0;
        bool        create_gl_shader    = true;

        std::function< void( const std::filesystem::path& ) > accumulate_glsl = [ & ] ( const std::filesystem::path& path ) -> void {
            std::ifstream file{ path, std::ios_base::binary };

            if( !file ) {
                echo( this, EchoLevel_Error ) << "Could NOT open file: \"" << path.string().c_str() << "\".";
                status = -1;
                return;
            }

            while( std::getline( file, line ) ) {
                struct _Directive {
                    const char*   str;
                    void*         lbl;
                } directives[] = {
                    { str: "//IXN#include", lbl: &&l_directive_include },
                    { str: "//IXN#name", lbl: &&l_directive_name }
                };

                std::string arg;

                for( auto& d : directives ) {
                    if( !line.starts_with( d.str ) ) continue;
                    
                    size_t q1 = line.find_first_of( '<' );
                    size_t q2 = line.find_last_of( '>' );

                    if( q1 == std::string::npos && q2 == std::string::npos ) {
                        echo( this, EchoLevel_Error ) << "Directive argument must be quoted between \"<>\".";
                        status = -1;
                        return;
                    }
                    
                    arg = std::string{ line.c_str() + q1 + 1, q2 - q1 - 1 };
                    goto *d.lbl;
                }
                goto l_code_line;
            
            l_directive_include:
                _ENGINE_SHADER3_EXEC_DIRECTIVE_CALLBACK( directive_cb, SHADER3_DIRECTIVE_INCLUDE, arg, nullptr );
                accumulate_glsl( path.parent_path() / arg );
                continue;
            
            l_directive_name:
                if( !_name.empty() ) {
                    echo( this, EchoLevel_Error ) << "Multiple names given. Initial name \"" << _name << "\", conflicting with \"" << arg << "\".";
                    status = -1;
                    return;
                }

                _ENGINE_SHADER3_EXEC_DIRECTIVE_CALLBACK( directive_cb, SHADER3_DIRECTIVE_NAME, arg, nullptr );
                _name = arg;
                continue;

            l_code_line:
                source += line; source += '\n';
            }
        };

        accumulate_glsl( path );
        
        if( status != 0 ) {
            echo( this, EchoLevel_Error ) << "General failure during reading and generating source.";
            return;
        }

        if( _name.empty() ) {
            auto name = std::to_string( std::hash< std::string >{}( source ) );
            _ENGINE_SHADER3_EXEC_DIRECTIVE_CALLBACK( directive_cb, SHADER3_DIRECTIVE_NAME, name, nullptr );
            _name = std::move( name );
        }

        if( !create_gl_shader ) goto l_gl_shader_create_skip;
        {
        GLuint glidx = glCreateShader( phase );
        if( glidx == 0 ) {
            echo( this, EchoLevel_Error ) << "OpenGL returned NULL shader.";
            return;
        }

        const GLchar* const_const_const_const_const_const = source.c_str();
        glShaderSource( glidx, 1, &const_const_const_const_const_const, NULL );
        glCompileShader( glidx );

        GLint success;
        glGetShaderiv( glidx, GL_COMPILE_STATUS, &success );
        if( !success ) {
            GLchar log_buf[ 512 ];
            glGetShaderInfoLog( glidx, sizeof( log_buf ), NULL, log_buf );
            
            echo( this, EchoLevel_Error ) << "Fault compiling: \"" << log_buf << "\", from: \"" << path.string().c_str() << "\".";
            return;
        }

        _glidx = glidx;
        echo( this, EchoLevel_Ok ) << "Created as \"" << _name << "\"( " << _glidx << " ), from \"" << path.string().c_str() << "\".";
        return;
        }
    l_gl_shader_create_skip:
        echo( this, EchoLevel_Ok ) << "Created as \"" << _name << "\", without compiling source ( " << directive_cb_status << " ).";
        return;
    }

    Shader3( const Shader3& ) = delete;

    Shader3( Shader3&& other ) {
        _glidx = std::exchange( other._glidx, NULL );
        _name  = std::move( other._name );
    }

    ~Shader3() {
        if( _glidx ) glDeleteShader( std::exchange( _glidx, NULL ) );
    }

_ENGINE_PROTECTED:
    GLuint        _glidx   = 0;
    std::string   _name    = {};

public:
    operator decltype( _glidx ) () const {
        return _glidx;
    }

    GLuint glidx() const {
        return _glidx;
    }

    const std::string& name() const {
        return _name;
    }

};

enum SHADER_PIPE3_ATTR : DWORD {
    SHADER_PIPE3_ATTR_NAME,

    _SHADER_PIPE3_ATTR_FORCE_DWORD = 0x7F'FF'FF'FF
};
#define _ENGINE_SHADER_PIPE3_EXEC_ATTR_CALLBACK( func, attr, arg, ptr ) ( create_gl_program = ( func ? ( ( attr_cb_status = std::invoke( func, attr, arg, ptr ) ) == 0 ) : true ) )

class ShaderPipe3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "ShaderPipe3" );

public:
    friend class RenderCluster3;

public:
    inline static constexpr char   STAGE_NAME_SEP   = '-';

public:
    typedef   std::function< DWORD( SHADER_PIPE3_ATTR, const std::string&, void* ) >   attr_callback_t;

public:
    ShaderPipe3() = default;

    ShaderPipe3( 
        Shader3*        shaders[ 5 ], 
        attr_callback_t attr_cb,
        _ENGINE_COMMS_ECHO_ARG 
    ) {
        bool  create_gl_program = true;
        DWORD attr_cb_status    = 0;

        std::string pretty = "";

        static const char* stage_pretties[ 5 ] = {
            "VRTX-", ">TESC-", ">TESE-", ">GEOM-", ">FRAG"
        };

        for( DWORD idx = 0; idx < 5; ++idx ) {
            Shader3*& shader = shaders[ idx ];
            if( shader == nullptr ) continue;

            pretty += stage_pretties[ idx ];
            _name += shader->name() + STAGE_NAME_SEP;
        }
        _name.pop_back();

        _ENGINE_SHADER_PIPE3_EXEC_ATTR_CALLBACK( attr_cb, SHADER_PIPE3_ATTR_NAME, _name, nullptr );

        if( pretty.starts_with( '>' ) || pretty.ends_with( '-' ) ) {
            echo( this, EchoLevel_Warning ) << "Shader stages ill-arranged.";
        }

        if( shaders[ 1 ] == nullptr && shaders[ 2 ] == nullptr )
            this->draw_mode = GL_TRIANGLES;
        else
            this->draw_mode = GL_PATCHES;

        if( !create_gl_program ) goto l_program_create_skip;
        {
        GLuint glidx = glCreateProgram();
        if( glidx == 0 ) {
            echo( this, EchoLevel_Error ) << "OpenGL returned NULL shader program.";
            return;
        }

        for( DWORD idx = 0; idx < 5; ++idx ) {
            if( shaders[ idx ] == nullptr ) continue;

            glAttachShader( glidx, shaders[ idx ]->glidx() );
        }

        glLinkProgram( glidx );

        GLint success;
        glGetProgramiv( glidx, GL_LINK_STATUS, &success );
        if( !success ) {
            GLchar log_buf[ 512 ]; memset( log_buf, sizeof( log_buf ), 0 );
            glGetProgramInfoLog( glidx, sizeof( log_buf ), NULL, log_buf );
            
            echo( this, EchoLevel_Error ) << "Fault attaching shaders: \"" << log_buf << "\".";
            return;
        }

        _glidx = glidx;
        echo( this, EchoLevel_Ok ) << "Created as \"" << _name << "\"( " << _glidx << " ) | " << pretty << " |.";
        return;
        }
    l_program_create_skip:
        echo( this, EchoLevel_Ok ) << "Created as \"" << _name << "\", without creating program ( " << attr_cb_status << " ).";
        return;
    }

    ShaderPipe3( ShaderPipe3&& other ) {
        _glidx = std::exchange( other._glidx, NULL );
        _name  = std::move( other._name );
        
        draw_mode = std::exchange( other.draw_mode, NULL );
    }

    ~ShaderPipe3() {
        if( _glidx ) glDeleteProgram( std::exchange( _glidx, NULL ) );
    }

_ENGINE_PROTECTED:
    GLuint            _glidx          = 0;
    std::string       _name           = {};

public: 
    GLuint            draw_mode       = 0;

public:
    operator GLuint () const {
        return _glidx;
    }

    GLuint glidx() const {
        return _glidx;
    }

    const std::string& name() {
        return _name;
    }

public:
    ShaderPipe3& uplink() {
        glUseProgram( _glidx );
        return *this;
    }

public:
    template< typename ...Args >
    ShaderPipe3& pull( Args&&... args );

};



class Uniform3Impl : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Uniform3" );

public:
    Uniform3Impl() = default;

    Uniform3Impl( 
        const char* anchor, 
        _ENGINE_COMMS_ECHO_ARG 
    ) 
    : _anchor{ anchor }
    {
        echo( this, EchoLevel_Ok ) << "Created. Ready to dock \"" << anchor << "\".";
    }

    Uniform3Impl( 
        ShaderPipe3& pipe,
        const char*   anchor, 
        _ENGINE_COMMS_ECHO_ARG 
    ) 
    : Uniform3Impl{ anchor, echo }
    {
        this->push( pipe, echo );
    }

_ENGINE_PROTECTED:
    std::string                  _anchor;
    std::map< GLuint, GLuint >   _locs;

public:
    Uniform3Impl& operator = ( const Uniform3Impl& other ) {
        _anchor = other._anchor;
        _locs.clear();
        _locs.insert( other._locs.begin(), other._locs.end() );
        return *this;
    }

public:
    DWORD push( ShaderPipe3& pipe, _ENGINE_COMMS_ECHO_RT_ARG ) {
        pipe.uplink();
        GLuint loc = glGetUniformLocation( pipe, _anchor.c_str() );

        if( loc == -1 ) {
            echo( this, EchoLevel_Warning ) << "Shading pipe( " << pipe.glidx() << " ) has no uniform \"" << _anchor << "\".";
            return -1;
        }

        _locs.insert( { pipe.glidx(), loc } );
        return 0;
    }

};

template< typename ...Args >
ShaderPipe3& ShaderPipe3::pull( Args&&... args ) {
    ( args.push( *this ), ... );
    return *this;
}

template< typename T >
class Uniform3 : public Uniform3Impl {
public:
    Uniform3() = default;

    Uniform3( 
        const char* name, 
        const T&    under, 
        _ENGINE_COMMS_ECHO_ARG 
    ) : Uniform3Impl{ name, echo }, _under{ under }
    {}

    Uniform3( 
        const char* name, 
        _ENGINE_COMMS_ECHO_ARG 
    ) : Uniform3Impl{ name, echo }
    {}

    Uniform3( 
        ShaderPipe3& pipe,
        const char*   name, 
        const T&      under, 
        _ENGINE_COMMS_ECHO_ARG 
    ) : Uniform3Impl{ pipe, name, echo }, _under{ under }
    {}

    Uniform3( const Uniform3< T >& other )
    : Uniform3Impl{ other },
      _under{ other._under }
    {} 

_ENGINE_PROTECTED:
    T   _under   = {};

public:
    T& get() { return _under; }
    const T& get() const { return _under; }

    operator T& () { return this->get(); }
    operator const T& () const { return this->get(); }

    T* operator -> () {
        return &_under;
    }

public:
    Uniform3& operator = ( const T& under ) {
        _under = under;
        return *this;
    }

public:
    DWORD uplink_pv( GLuint pipe_glidx, const T& under ) {
        _under = under;
        return this->uplink_p( pipe_glidx );
    }

    DWORD uplink_p( GLuint pipe_glidx ) {
        glUseProgram( pipe_glidx );
        return this->_uplink( _locs[ pipe_glidx ] );
    }

    DWORD uplink_v( const T& under ) {
        _under = under;
        return this->uplink();
    }

    DWORD uplink() {
        glUseProgram( _locs.begin()->first );
        return this->_uplink( _locs.begin()->second );
    }

    DWORD uplink_b() {
        for( auto& [ pipe_glidx, ufrm_loc ] : _locs ) {
            glUseProgram( pipe_glidx );
            this->_uplink( ufrm_loc );
        }
        return 0;
    }

    DWORD uplink_bv( const T& under ) {
        _under = under;
        return this->uplink_b();
    }

_ENGINE_PROTECTED:
    DWORD _uplink( GLuint pipe_glidx );

};


template<> inline DWORD Uniform3< bool >::_uplink( GLuint loc ) {
    glUniform1i( loc, ( glm::u32 )_under ); 
    return 0;
}
template<> inline DWORD Uniform3< glm::u32 >::_uplink( GLuint loc ) {
    glUniform1i( loc, _under ); 
    return 0;
}
template<> inline DWORD Uniform3< glm::i32 >::_uplink( GLuint loc ) {
    glUniform1i( loc, _under ); 
    return 0;
}
template<> inline DWORD Uniform3< glm::f32 >::_uplink( GLuint loc ) {
    glUniform1f( loc, _under ); 
    return 0;
}
template<> inline DWORD Uniform3< glm::vec3 >::_uplink( GLuint loc ) {
    glUniform3f( loc, _under.x, _under.y, _under.z ); 
    return 0;
}
template<> inline DWORD Uniform3< glm::vec4 >::_uplink( GLuint loc ) {
    glUniform4f( loc, _under.x, _under.y, _under.z, _under.w  ); 
    return 0;
}
template<> inline DWORD Uniform3< glm::mat4 >::_uplink( GLuint loc ) {
    glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( _under ) ); 
    return 0;
}
template<> inline DWORD Uniform3< glm::vec3[ 3 ] >::_uplink( GLuint loc ) {
    glUniform3fv( loc, sizeof( _under ) / sizeof( glm::vec3 ), ( GLfloat* )_under  ); 
    return 0;
}


class Lens3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Lens3" );

public:
    Lens3() = default;

    Lens3( glm::vec3 p, glm::vec3 t, glm::vec3 u )
    : pos{ p }, tar{ t }, up{ u }
    {}

public:
    glm::vec3   pos;
    glm::vec3   tar;
    glm::vec3   up;

public:
    glm::mat4 view() const {
        return glm::lookAt( pos, tar, up );
    }

    glm::vec3 forward() const {
        return tar - pos;
    }

    glm::vec3 forward_n() const {
        return glm::normalize( this->forward() );
    }

    glm::vec3 right() const {
        return glm::cross( this->forward(), up );
    }

    glm::vec3 right_n() const {
        return glm::normalize( this->right() );
    }

    ggfloat_t l2t() const {
        return glm::length( tar - pos );
    }

public:
    Lens3& zoom( ggfloat_t p, glm::vec2 lim = glm::vec2{ 0.0, std::numeric_limits< ggfloat_t >::max() } ) {
        glm::vec3 fwd   = this->forward_n();
        glm::vec3 n_pos = pos + fwd * p;
        ggfloat_t d     = glm::length( n_pos - tar );

        if( d < lim.s ) {
            pos = n_pos + ( d - lim.s ) * fwd;
        } else if( d > lim.t ) {
            pos = n_pos + ( d - lim.t ) * fwd;
        } else {
            pos = n_pos;
        }

        return *this;
    }

    Lens3& roll( ggfloat_t angel ) {
        up = glm::rotate( up, angel, this->forward() );
        return *this;
    }

    Lens3& spin( glm::vec2 angels ) {
        glm::vec3 rel = pos - tar;
        glm::vec3 sec = glm::cross( tar - pos, up );

        rel = glm::rotate( rel, angels.x, up );
        rel = glm::rotate( rel, -angels.y, sec );

        pos = rel + tar;
        up  = glm::rotate( up, -angels.y, sec );

        return *this;
    }

    Lens3& spin_ul( glm::vec2 angels, glm::vec2 lim = glm::vec2{ -90.0, 90.0 } ) {
        static constexpr const ggfloat_t FPU_OFFSET = 0.001f;

        lim = glm::radians( lim );

        glm::vec3 rel     = pos - tar;
        glm::vec3 wing    = glm::cross( tar - pos, up );
        ggfloat_t angwu   = -( glm::acos( glm::dot( rel, up ) / ( glm::length( rel ) * glm::length( up ) ) ) - PI / 2.0 );
        glm::vec3 lim_rel = glm::rotate( rel, angwu - ( angels.t >= 0.0 ? lim.t : lim.s ), wing );

        glm::vec3 n_rel = glm::rotate( rel, -angels.t, wing );
        glm::vec3 c1    = glm::cross( lim_rel, rel );
        glm::vec3 c2    = glm::cross( lim_rel, n_rel );

        if( glm::dot( c1, c2 ) < 0.0 ) 
            rel = glm::rotate( lim_rel, angels.t >= 0.0 ? FPU_OFFSET : -FPU_OFFSET, wing );
        else
            rel = glm::rotate( rel, -angels.t, wing );  

        rel = glm::rotate( rel, angels.s, up );

        pos = rel + tar;

        return *this;
    }

};



enum MESH3_FLAG : DWORD {
    MESH3_FLAG_MAKE_PIPES = 1 << 0,

    _MESH3_FLAG = 0x7F'FF'FF'FF
};

class Mesh3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Mesh3" );

public:
    Mesh3() = default;

    Mesh3( const std::filesystem::path& root_dir, std::string_view prefix, DWORD flags, _ENGINE_COMMS_ECHO_ARG ) 
    : model{ "model", glm::mat4{ 1.0 }, echo },
      Kd{ "Kd", glm::vec3{}, echo }
    {
        DWORD                              status;
		tinyobj::attrib_t                  attrib;
		std::vector< tinyobj::shape_t >    meshes;
        std::vector< tinyobj::material_t > materials;
        size_t                             total_vrtx_count = 0;
		std::string                        error_str;

        std::filesystem::path root_dir_p = root_dir / prefix.data();
        std::filesystem::path obj_path   = root_dir_p; obj_path += ".obj";

        echo( this, EchoLevel_Info ) << "Compiling the object: \"" << obj_path.string().c_str() << "\".";

		status = tinyobj::LoadObj( 
            &attrib, &meshes, &materials, &error_str, 
            obj_path.string().c_str(), root_dir.string().c_str(), 
            GL_TRUE
        );

		if( !error_str.empty() )
            echo( this, EchoLevel_Warning ) << "TinyObj says: \"" << error_str << "\".";

		if( status == 0 ) {
            echo( this, EchoLevel_Error ) << "Failed to compile the object.";
            return;
		}

		echo( this, EchoLevel_Ok ) << "Compiled " << materials.size() << " materials over " << meshes.size() << " meshes."; 

        _mtls.reserve( materials.size() );
        for( tinyobj::material_t& mtl_base : materials ) { 
            _Mtl& mtl = _mtls.emplace_back(); 
    
            mtl.data = std::move( mtl_base ); 

            GLuint tex_unit = 0;

            struct _GENERAL_TEX {
                const char*    key;
                std::string*   name;
            } general_texs[] = {
                { "map_Ka", &mtl.data.ambient_texname },
                { "map_Kd", &mtl.data.diffuse_texname },
                { "map_Ks", &mtl.data.specular_texname },
                { "map_Ns", &mtl.data.specular_highlight_texname }
            };

            for( auto& [ key, name ] : general_texs ) {
                if( name->empty() ) continue;

                if( this->_push_tex( root_dir / *name, key, tex_unit, echo ) != 0 ) continue;

                mtl.tex_idxs.push_back( _texs.size() - 1 );
                ++tex_unit;
            }
        
            for( auto& [ key, value ] : mtl.data.unknown_parameter ) {
                if( !key.starts_with( "IXN" ) ) {
                    echo( this, EchoLevel_Warning ) << "Unrecognized general parameter \"" << key << "\".";
                    continue;
                }

                bool resolved = false;

                if( key.find( "map" ) != std::string::npos && this->_push_tex( root_dir / value, key, tex_unit, echo ) == 0 ) {
                    mtl.tex_idxs.push_back( _texs.size() - 1 );
                    ++tex_unit;
                    resolved = true;
                }

                if( !resolved ) 
                    echo( this, EchoLevel_Warning ) << "Unrecognized IXN parameter \"" << key << "\".";
            }
        }

        for( tinyobj::shape_t& mesh_ex : meshes ) {
            tinyobj::mesh_t& mesh = mesh_ex.mesh;
            _SubMesh&        sub  = _sub_meshes.emplace_back();

            sub.count = mesh.indices.size();

            struct VrtxData{
                glm::vec3   pos;
                glm::vec3   nrm;
                glm::vec2   txt;
            };
            std::vector< VrtxData > vrtx_data; vrtx_data.reserve( sub.count );
            size_t base_idx = 0;
            size_t v_acc    = 0;
            size_t l_mtl    = mesh.material_ids[ 0 ];
            for( size_t f_idx = 0; f_idx < mesh.num_face_vertices.size(); ++f_idx ) {
                UBYTE f_c = mesh.num_face_vertices[ f_idx ];

                for( UBYTE v_idx = 0; v_idx < f_c; ++v_idx ) {
                    tinyobj::index_t& idx = mesh.indices[ base_idx + v_idx ];

                    vrtx_data.emplace_back( VrtxData{
                        pos: { *( glm::vec3* )&attrib.vertices[ 3 *idx.vertex_index ] },
                        nrm: { *( glm::vec3* )&attrib.normals[ 3 *idx.normal_index ] },
                        txt: { ( idx.texcoord_index != -1 ) ? *( glm::vec2* )&attrib.texcoords[ 2 *idx.texcoord_index ] : glm::vec2{ 1.0 } }
                    } );
                }

                if( mesh.material_ids[ f_idx ] != l_mtl ) {
                    sub.bursts.emplace_back( _SubMesh::Burst{ count: v_acc, mtl_idx: l_mtl } );
                    v_acc = 0;
                    l_mtl = mesh.material_ids[ f_idx ];
                }

                v_acc    += f_c;
                base_idx += f_c;
            }

            sub.bursts.emplace_back( _SubMesh::Burst{ count: v_acc, mtl_idx: l_mtl } );

            glGenVertexArrays( 1, &sub.VAO );
            glGenBuffers( 1, &sub.VBO );

            glBindVertexArray( sub.VAO );
        
            glBindBuffer( GL_ARRAY_BUFFER, sub.VBO );
            glBufferData( GL_ARRAY_BUFFER, vrtx_data.size() * sizeof( VrtxData ), vrtx_data.data(), GL_STATIC_DRAW );

            glEnableVertexAttribArray( 0 );
            glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( VrtxData ), ( GLvoid* )0 );
            
            glEnableVertexAttribArray( 1 );
            glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( VrtxData ), ( GLvoid* )offsetof( VrtxData, nrm ) );
        
            glEnableVertexAttribArray( 2 );
            glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( VrtxData ), ( GLvoid* )offsetof( VrtxData, txt ) );

            std::vector< GLuint > indices; indices.assign( sub.count, 0 );
            for( size_t idx = 1; idx < indices.size(); ++idx ) indices[ idx ] = idx;

            glGenBuffers( 1, &sub.EBO );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, sub.EBO );
		    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sub.count * sizeof( GLuint ), indices.data(), GL_STATIC_DRAW );
        }

        glBindVertexArray( 0 );

        for( auto& tex : _texs )
            tex.ufrm = Uniform3< glm::u32 >{ tex.name.c_str(), tex.unit, echo };

        if( flags & MESH3_FLAG_MAKE_PIPES ) {  
            this->pipe.vector( GME_render_cluster3.make_pipe_from_prefixed_path( root_dir_p, echo ) );
            this->dock_in( nullptr, echo );
        }
	}

_ENGINE_PROTECTED:
    struct _SubMesh {
        GLuint                 VAO;
        GLuint                 VBO;
        GLuint                 EBO;
        size_t                 count;
        struct Burst {
            size_t   count;
            size_t   mtl_idx;
        };
        std::vector< Burst >   bursts;
    };
    std::vector< _SubMesh >   _sub_meshes;
    struct _Mtl {
        tinyobj::material_t     data;
        std::vector< size_t >   tex_idxs;
    };
    std::vector< _Mtl >       _mtls;
    struct _Tex {
        GLuint                 glidx;
        std::string            name;
        GLuint                 unit;
        Uniform3< glm::u32 >   ufrm;
    };
    std::vector< _Tex >       _texs;

public:
    Uniform3< glm::mat4 >     model;
    Uniform3< glm::vec3 >     Kd;

    HVEC< ShaderPipe3 >       pipe;

_ENGINE_PROTECTED:
    DWORD _push_tex( 
        const std::filesystem::path& path, 
        std::string_view             name, 
        GLuint                       pipe_unit,  
        _ENGINE_COMMS_ECHO_ARG 
    ) {
        GLuint tex_glidx;

        int x, y, n;
        UBYTE* img_buf = stbi_load( path.string().c_str(), &x, &y, &n, 4 );

        if( img_buf == nullptr ) {
            echo( this, EchoLevel_Error ) << "Failed to load texture data from: \"" << path.string().c_str() << "\".";
            return -1;
        }

        glGenTextures( 1, &tex_glidx );
        glBindTexture( GL_TEXTURE_2D, tex_glidx );
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_SRGB,
            x, y,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            img_buf
        );
        glGenerateMipmap( GL_TEXTURE_2D );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glBindTexture( GL_TEXTURE_2D, 0 );
        stbi_image_free( img_buf );

        _texs.emplace_back( _Tex{
            glidx: tex_glidx,
            name: name.data(),
            unit: pipe_unit,
            ufrm: {}
        } );

        echo( this, EchoLevel_Ok ) << "Pushed texture from: \"" << path.string().c_str() << "\", on pipe unit: " << pipe_unit << ".";
        return 0;
    }

public:
    Mesh3& dock_in( HVEC< ShaderPipe3 > other_pipe, _ENGINE_COMMS_ECHO_RT_ARG ) {
        if( other_pipe.get() == this->pipe.get() ) {
            echo( this, EchoLevel_Warning ) << "Multiple docks on same pipe( " << this->pipe->glidx() << " ) detected.";
        }

        if( other_pipe != nullptr ) this->pipe.vector( std::move( other_pipe ) );

        this->model.push( *this->pipe );

        Kd.push( *this->pipe );

        for( auto& tex : _texs )
            tex.ufrm.push( *this->pipe );

        return *this;
    }

public:
    Mesh3& splash() {
        return this->splash( *this->pipe );
    }

    Mesh3& splash( ShaderPipe3& pipe ) {
        pipe.uplink();
        this->model.uplink();

        for( _SubMesh& sub : _sub_meshes ) {
            glBindVertexArray( sub.VAO );

            for( auto& burst : sub.bursts ) {
                float* diffuse = _mtls[ burst.mtl_idx ].data.diffuse;
                Kd.uplink_bv( glm::vec3{ diffuse[ 0 ], diffuse[ 1 ], diffuse[ 2 ] } );

                for( size_t tex_idx : _mtls[ burst.mtl_idx ].tex_idxs ) {
                    if( tex_idx == -1 ) continue;

                    _Tex& tex = _texs[ tex_idx ];

                    glActiveTexture( GL_TEXTURE0 + tex.unit );
                    glBindTexture( GL_TEXTURE_2D, tex.glidx );
                    tex.ufrm.uplink();
                }
                
                glDrawElements( pipe.draw_mode, ( GLsizei )burst.count, GL_UNSIGNED_INT, 0 );
            }
        }

        return *this;
    }

};



class Render3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Render3" );
    //std::cout << "GOD I SUMMON U. GIVE MIP TEO FOR A FEW DATES (AT LEAST 100)"; 
    //std::cout << "TY";
public:
    Render3() = default;
    
    Render3( GLFWwindow* glfwnd, _ENGINE_COMMS_ECHO_RT_ARG )
    : _glfwnd{ glfwnd } {
        glfwMakeContextCurrent( _glfwnd );

        _rend_str = ( const char* )glGetString( GL_RENDERER ); 
        _gl_str   = ( const char* )glGetString( GL_VERSION );

        echo( this, EchoLevel_Info ) << "Docked on \"" << _rend_str << "\".";
        echo( this, EchoLevel_Info ) << "OpenGL on \"" << _gl_str << "\".";

        glDepthFunc( GL_LESS );
        glEnable( GL_DEPTH_TEST );

        glFrontFace( GL_CCW );

        glCullFace( GL_BACK );
        glEnable( GL_CULL_FACE ); 

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable( GL_BLEND );

        int wnd_w, wnd_h;
        glfwGetFramebufferSize( _glfwnd, &wnd_w, &wnd_h );
        glViewport( 0, 0, wnd_w, wnd_h );

        stbi_set_flip_vertically_on_load( true );

        glewExperimental = GL_TRUE; 
        glewInit();

        echo( this, EchoLevel_Ok ) << "Created.";
    }

    // explicit Render3( HVEC< Surface > surf, _ENGINE_COMMS_ECHO_RT_ARG ) {
        // comms( this, EchoLevel_Error ) << "This constructor is no longer supported.";
    // }

    Render3( const Render3& ) = delete;
    Render3( Render3&& ) = delete;

_ENGINE_PROTECTED:
    GLFWwindow*   _glfwnd    = NULL;

    const char*   _rend_str   = NULL;     
    const char*   _gl_str     = NULL;   

public:
    GLFWwindow* handle() { return _glfwnd; }

public:
    Render3& clear( glm::vec4 c = { .0, .0, .0, 1.0 } ) {
        glClearColor( c.r, c.g, c.b, c.a );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        return *this;
    }

    Render3& swap() {
        glfwSwapBuffers( _glfwnd );
        return *this;
    }

public:
    Render3& uplink_face_culling() {
        glEnable( GL_CULL_FACE );
        return *this;
    }

    Render3& downlink_face_culling() {
        glDisable( GL_CULL_FACE );
        return *this;
    }

    Render3& uplink_fill() {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        return *this;
    }

    Render3& uplink_wireframe() {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        return *this;
    }

    Render3& uplink_points() {
        glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
        return *this;
    }

public:
    float aspect() const {
        int wnd_w, wnd_h;
        glfwGetFramebufferSize( _glfwnd, &wnd_w, &wnd_h );
        return ( float )wnd_w / ( float )wnd_h;
    }

};



};

#else
    #warning Compiling for OpenGL without choosing this GL.
#endif
