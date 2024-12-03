#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/surface.hpp>
#include <IXT/volatile-ptr.hpp>

#if defined( _ENGINE_GL_OPEN_GL )

namespace _ENGINE_NAMESPACE {



enum SHADER3_PHASE : DWORD {
    SHADER3_PHASE_VERTEX    = GL_VERTEX_SHADER,
    SHADER3_PHASE_TESS_CTRL = GL_TESS_CONTROL_SHADER,
    SHADER3_PHASE_TESS_EVAL = GL_TESS_EVALUATION_SHADER,
    SHADER3_PHASE_GEOMETRY  = GL_GEOMETRY_SHADER,
    SHADER3_PHASE_FRAGMENT  = GL_FRAGMENT_SHADER,

    SHADER3_PHASE_VERTEX_IDX    = 0,
    SHADER3_PHASE_TESS_CTRL_IDX = 1,
    SHADER3_PHASE_TESS_EVAL_IDX = 2,
    SHADER3_PHASE_GEOMETRY_IDX  = 3,
    SHADER3_PHASE_FRAGMENT_IDX  = 4,
    SHADER3_PHASE_IDX_RESERVED  = 5,

    _SHADER3_PHASE_FORCE_DWORD = 0x7F'FF'FF'FF
};

class Shader3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Shader3" );

public:
    Shader3() = default;

    Shader3( const std::filesystem::path& path, SHADER3_PHASE phase, _ENGINE_COMMS_ECHO_ARG ) {
        std::string source;
        std::string line;
        DWORD       status;

        std::function< void( const std::filesystem::path& ) > accumulate_glsl = [ & ] ( const std::filesystem::path& path ) -> void {
            std::ifstream file{ path, std::ios_base::binary };

            if( !file ) {
                echo( this, ECHO_LEVEL_ERROR ) << "Could NOT open file: \"" << path.string().c_str() << "\".";
                status = -1;
            }

            while( std::getline( file, line ) ) {
                static const char* DIRECTIVE_INCLUDE_STR = "//IXT#include";

                if( !line.starts_with( DIRECTIVE_INCLUDE_STR ) ) {
                    source += line; source += '\n';
                    continue;
                }

                size_t q1 = line.find_first_of( '<' );
                size_t q2 = line.find_last_of( '>' );

                if( q1 == std::string::npos ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Include directive path must be quoted between \"<>\".";
                    status = -1;
                    return;
                }
                line.pop_back();

                accumulate_glsl( path.parent_path() / std::string{ line.c_str() + q1 + 1, q2 - q1 - 1 } );
            }
        };

        accumulate_glsl( path );
        
        if( status != 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "General failure during reading and generating source.";
            return;
        }

        GLuint glidx = glCreateShader( phase );

        const GLchar* const_const_const_const_const_const = source.c_str();

        glShaderSource( glidx, 1, &const_const_const_const_const_const, NULL );
        glCompileShader( glidx );

        GLint success;
        glGetShaderiv( glidx, GL_COMPILE_STATUS, &success );
        if( !success ) {
            GLchar log_buf[ 512 ];
            glGetShaderInfoLog( glidx, sizeof( log_buf ), NULL, log_buf );
            
            echo( this, ECHO_LEVEL_ERROR ) << "Fault compiling: \"" << log_buf << "\", from: \"" << path.string().c_str() << "\".";
            return;
        }

        _glidx = glidx;
        echo( this, ECHO_LEVEL_OK ) << "Created from: \"" << path.string().c_str() << "\".";
    }

    ~Shader3() {
        glDeleteShader( _glidx );
    }

_ENGINE_PROTECTED:
    GLuint   _glidx   = NULL;

public:
    operator decltype( _glidx ) () const {
        return _glidx;
    }

    GLuint glidx() const {
        return _glidx;
    }

};

class ShadingPipe3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "ShadingPipe3" );

public:
    using SP_t = VPtr< Shader3 >;

public:
    ShadingPipe3() = default;

    ShadingPipe3( 
        SP_t vert,
        SP_t tesc,
        SP_t tese,
        SP_t geom, 
        SP_t frag, 
        _ENGINE_COMMS_ECHO_ARG 
    ) {
        GLuint glidx = glCreateProgram();

        std::string pretty;

        if( vert != nullptr ) { pretty += "VRTX->"; glAttachShader( glidx, vert->glidx() ); _shaders[ 0 ] = std::move( vert ); }
        if( tesc != nullptr ) { pretty += "TESC->"; glAttachShader( glidx, tesc->glidx() ); _shaders[ 1 ] = std::move( tesc ); }
        if( tese != nullptr ) { pretty += "TESE->"; glAttachShader( glidx, tese->glidx() ); _shaders[ 2 ] = std::move( tese ); }
        if( geom != nullptr ) { pretty += "GEOM->"; glAttachShader( glidx, geom->glidx() ); _shaders[ 3 ] = std::move( geom ); }
        if( frag != nullptr ) { pretty += "FRAG"; glAttachShader( glidx, frag->glidx() ); _shaders[ 4 ] = std::move( frag ); }

        if( tesc == nullptr && tese == nullptr )
            this->draw_mode = GL_TRIANGLES;
        else
            this->draw_mode = GL_PATCHES;

        glLinkProgram( glidx );

        GLint success;
        glGetProgramiv( glidx, GL_LINK_STATUS, &success );
        if( !success ) {
            GLchar log_buf[ 512 ];
            glGetProgramInfoLog( glidx, sizeof( log_buf ), NULL, log_buf );
            
            echo( this, ECHO_LEVEL_ERROR ) << "Attaching shaders: \"" << log_buf << "\".";
            return;
        }

        _glidx = glidx;
        echo( this, ECHO_LEVEL_OK ) << "Created with glidx( " << _glidx << " ) --- | " << pretty << " |.";
    }

    ShadingPipe3(
        const Shader3& vert,
        const Shader3& geom,
        const Shader3& frag,
        _ENGINE_COMMS_ECHO_ARG 
    )
    : ShadingPipe3{ SP_t{ vert }, SP_t{ nullptr }, SP_t{ nullptr }, SP_t{ geom }, SP_t{ frag } }
    {}

    ShadingPipe3(
        const Shader3& vert,
        const Shader3& frag,
        _ENGINE_COMMS_ECHO_ARG 
    )
    : ShadingPipe3{ SP_t{ vert } , SP_t{ nullptr }, SP_t{ nullptr }, SP_t{ nullptr }, SP_t{ frag } }
    {}

_ENGINE_PROTECTED:
    GLuint            _glidx                                   = NULL;
    VPtr< Shader3 >   _shaders[ SHADER3_PHASE_IDX_RESERVED ]   = {};

public: 
    GLuint            draw_mode                                = NULL;

public:
    operator GLuint () const {
        return _glidx;
    }

    GLuint glidx() const {
        return _glidx;
    }

public:
    ShadingPipe3& uplink() {
        glUseProgram( _glidx );
        return *this;
    }

public:
    template< typename ...Args >
    ShadingPipe3& pull( Args&&... args );

};



class Uniform3Unknwn : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Uniform3" );

public:
    Uniform3Unknwn() = default;

    Uniform3Unknwn( 
        const char* anchor, 
        _ENGINE_COMMS_ECHO_ARG 
    ) 
    : _anchor{ anchor }
    {
        echo( this, ECHO_LEVEL_OK ) << "Created. Ready to dock \"" << anchor << "\".";
    }

    Uniform3Unknwn( 
        ShadingPipe3& pipe,
        const char*   anchor, 
        _ENGINE_COMMS_ECHO_ARG 
    ) 
    : Uniform3Unknwn{ anchor, echo }
    {
        this->push( pipe, echo );
    }

_ENGINE_PROTECTED:
    std::string                  _anchor;
    std::map< GLuint, GLuint >   _locs;

public:
    Uniform3Unknwn& operator = ( const Uniform3Unknwn& other ) {
        _anchor = other._anchor;
        _locs.clear();
        _locs.insert( other._locs.begin(), other._locs.end() );
        return *this;
    }

public:
    DWORD push( ShadingPipe3& pipe, _ENGINE_COMMS_ECHO_RT_ARG ) {
        pipe.uplink();
        GLuint loc = glGetUniformLocation( pipe, _anchor.c_str() );

        if( loc == -1 ) {
            echo( this, ECHO_LEVEL_WARNING ) << "Shading pipe( " << pipe.glidx() << " ) has no uniform \"" << _anchor << "\".";
            return -1;
        }

        _locs.insert( { pipe.glidx(), loc } );
        return 0;
    }

};

template< typename ...Args >
ShadingPipe3& ShadingPipe3::pull( Args&&... args ) {
    ( args.push( *this ), ... );
    return *this;
}

template< typename T >
class Uniform3 : public Uniform3Unknwn {
public:
    Uniform3() = default;

    Uniform3( 
        const char* name, 
        const T&    under, 
        _ENGINE_COMMS_ECHO_ARG 
    ) : Uniform3Unknwn{ name, echo }, _under{ under }
    {}

    Uniform3( 
        const char* name, 
        _ENGINE_COMMS_ECHO_ARG 
    ) : Uniform3Unknwn{ name, echo }
    {}

    Uniform3( 
        ShadingPipe3& pipe,
        const char*   name, 
        const T&      under, 
        _ENGINE_COMMS_ECHO_ARG 
    ) : Uniform3Unknwn{ pipe, name, echo }, _under{ under }
    {}

    Uniform3( const Uniform3< T >& other )
    : Uniform3Unknwn{ other },
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
template<> inline DWORD Uniform3< glm::f32 >::_uplink( GLuint loc ) {
    glUniform1f( loc, _under ); 
    return 0;
}
template<> inline DWORD Uniform3< glm::vec3 >::_uplink( GLuint loc ) {
    glUniform3f( loc, _under.x, _under.y, _under.z  ); 
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
template< > inline DWORD Uniform3< glm::vec3[ 3 ] >::_uplink( GLuint loc ) {
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



#define _ENGINE_MESH3__PUSH_TEX( mtl_attr, name, unit )

enum MESH3_FLAG : DWORD {
    MESH3_FLAG_MAKE_SHADING_PIPE = 1,

    _MESH3_FLAG = 0x7F'FF'FF'FF
};

class Mesh3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Mesh3" );

public:
    Mesh3( const std::filesystem::path& root_dir, std::string_view prefix, DWORD flags, _ENGINE_COMMS_ECHO_ARG ) 
    : model{ "model", glm::mat4{ 1.0 }, echo }
    {
        DWORD                              status;
		tinyobj::attrib_t                  attrib;
		std::vector< tinyobj::shape_t >    meshes;
        std::vector< tinyobj::material_t > materials;
        size_t                             total_vrtx_count = 0;
		std::string                        error_str;

        std::filesystem::path root_dir_p = root_dir / prefix.data();
        std::filesystem::path obj_path   = root_dir_p; obj_path += ".obj";

        echo( this, ECHO_LEVEL_INTEL ) << "Compiling the object: \"" << obj_path.string().c_str() << "\".";

		status = tinyobj::LoadObj( 
            &attrib, &meshes, &materials, &error_str, 
            obj_path.string().c_str(), root_dir.string().c_str(), 
            GL_TRUE
        );

		if( !error_str.empty() )
            echo( this, ECHO_LEVEL_WARNING ) << "TinyObj says: \"" << error_str << "\".";

		if( status == 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Failed to compile the object.";
            return;
		}

		echo( this, ECHO_LEVEL_OK ) << "Compiled " << materials.size() << " materials over " << meshes.size() << " meshes."; 

        _mtls.reserve( materials.size() );
        for( tinyobj::material_t& mtl_base : materials ) { 
            _Mtl& mtl = _mtls.emplace_back(); 
    
            mtl.data = std::move( mtl_base ); 
            
            std::string* tex_name = &mtl.data.ambient_texname;
            if( !tex_name->empty() ) {
                if( this->_push_tex( root_dir / *tex_name, "map_Ka", 0, echo ) != 0 )
                    continue;

                mtl.tex_Ka_idx = _texs.size() - 1;
            }

            tex_name = &mtl.data.diffuse_texname;
            if( !tex_name->empty() ) {
                 if( this->_push_tex( root_dir / *tex_name, "map_Kd", 1, echo ) != 0 )
                    continue;

                mtl.tex_Kd_idx = _texs.size() - 1;
            }

            tex_name = &mtl.data.specular_texname;
            if( !tex_name->empty() ) {
                if( this->_push_tex( root_dir / *tex_name, "map_Ks", 2, echo ) != 0 )
                    continue;

                mtl.tex_Ks_idx = _texs.size() - 1;
            }

            tex_name = &mtl.data.specular_highlight_texname;
            if( !tex_name->empty() ) {
                if( this->_push_tex( root_dir / *tex_name, "map_Ns", 3, echo ) != 0 )
                    continue;

                mtl.tex_Ns_idx = _texs.size() - 1;
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

        if( flags & MESH3_FLAG_MAKE_SHADING_PIPE ) {  
            struct PHASE_DATA {
                int             idx;
                SHADER3_PHASE   phase;
                const char*     str;
            };

            ShadingPipe3::SP_t shaders[ SHADER3_PHASE_IDX_RESERVED + 1 ];

            for( auto phase : std::initializer_list< PHASE_DATA >{ 
                { SHADER3_PHASE_VERTEX_IDX, SHADER3_PHASE_VERTEX, ".vert" }, 
                { SHADER3_PHASE_TESS_CTRL_IDX, SHADER3_PHASE_TESS_CTRL, ".tesc" }, 
                { SHADER3_PHASE_TESS_EVAL_IDX, SHADER3_PHASE_TESS_EVAL, ".tese" }, 
                { SHADER3_PHASE_GEOMETRY_IDX, SHADER3_PHASE_GEOMETRY, ".geom" }, 
                { SHADER3_PHASE_FRAGMENT_IDX, SHADER3_PHASE_FRAGMENT, ".frag" } } 
            ) {
                std::filesystem::path phase_path{ root_dir_p };
                phase_path += phase.str;

                if( !std::filesystem::exists( phase_path ) ) continue;

                shaders[ phase.idx ].reset( std::make_shared< Shader3 >( phase_path, phase.phase, echo ) );
            }

            this->pipe.reset( std::make_shared< ShadingPipe3 >( 
                std::move( shaders[ SHADER3_PHASE_VERTEX_IDX ] ), 
                std::move( shaders[ SHADER3_PHASE_TESS_CTRL_IDX ] ),
                std::move( shaders[ SHADER3_PHASE_TESS_EVAL_IDX ] ),
                std::move( shaders[ SHADER3_PHASE_GEOMETRY_IDX ] ), 
                std::move( shaders[ SHADER3_PHASE_FRAGMENT_IDX ] ), 
                echo 
            ) );

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
        tinyobj::material_t   data;
        size_t                tex_Ka_idx   = -1;
        size_t                tex_Kd_idx   = -1;
        size_t                tex_Ks_idx   = -1;
        size_t                tex_Ns_idx   = -1;
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

    VPtr< ShadingPipe3 >      pipe;

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
            echo( this, ECHO_LEVEL_ERROR ) << "Failed to load texture data from: \"" << path.string().c_str() << "\".";
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

        echo( this, ECHO_LEVEL_OK ) << "Pushed texture from: \"" << path.string().c_str() << "\", on pipe unit: " << pipe_unit << ".";
        return 0;
    }

public:
    Mesh3& dock_in( VPtr< ShadingPipe3 > other_pipe, _ENGINE_COMMS_ECHO_RT_ARG ) {
        if( other_pipe.get() == this->pipe.get() ) {
            echo( this, ECHO_LEVEL_WARNING ) << "Multiple docks on same pipe( " << this->pipe->glidx() << " ) detected.";
        }

        if( other_pipe != nullptr ) this->pipe = std::move( other_pipe );

        this->model.push( *this->pipe );

        for( auto& tex : _texs )
            tex.ufrm.push( *this->pipe );

        return *this;
    }

public:
    Mesh3& splash() {
        return this->splash( *this->pipe );
    }

    Mesh3& splash( ShadingPipe3& pipe ) {
        pipe.uplink();

        for( _SubMesh& sub : _sub_meshes ) {
            glBindVertexArray( sub.VAO );

            for( auto& burst : sub.bursts ) {
                for( size_t tex_idx : {
                    _mtls[ burst.mtl_idx ].tex_Ka_idx, _mtls[ burst.mtl_idx ].tex_Kd_idx, _mtls[ burst.mtl_idx ].tex_Ks_idx,
                    _mtls[ burst.mtl_idx ].tex_Ns_idx
                } ) {
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



class Renderer3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Renderer3" );
    //std::cout << "GOD I SUMMON U. GIVE MIP TEO FOR A FEW DATES (AT LEAST 100)"; 
    //std::cout << "TY";
public:
    Renderer3() = default;
    
    Renderer3( VPtr< Surface > surface, _ENGINE_COMMS_ECHO_ARG )
    : _surface{ std::move( surface ) } {
        _surface->uplink_context_on_this_thread( echo );

        _rend_str = ( const char* )glGetString( GL_RENDERER ); 
        _gl_str   = ( const char* )glGetString( GL_VERSION );

        echo( this, ECHO_LEVEL_INTEL ) << "Docked on \"" << _rend_str << "\".";
        echo( this, ECHO_LEVEL_INTEL ) << "OpenGL on \"" << _gl_str << "\".";

        glDepthFunc( GL_LESS );
        glEnable( GL_DEPTH_TEST );

        glFrontFace( GL_CCW );

        glCullFace( GL_BACK );
        glEnable( GL_CULL_FACE ); 

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable( GL_BLEND );

        glPatchParameteri( GL_PATCH_VERTICES, 3 );

        glViewport( 0, 0, ( int )_surface->width(), ( int )_surface->height() );

        stbi_set_flip_vertically_on_load( true );

        echo( this, ECHO_LEVEL_OK ) << "Created.";
    }

    Renderer3( const Renderer3& ) = delete;
    Renderer3( Renderer3&& ) = delete;

_ENGINE_PROTECTED:
    VPtr< Surface >   _surface    = NULL;

    const char*       _rend_str   = NULL;     
    const char*       _gl_str     = NULL;    

public:
    Renderer3& clear( glm::vec4 c = { .0, .0, .0, 1.0 } ) {
        glClearColor( c.r, c.g, c.b, c.a );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        return *this;
    }

    Renderer3& swap() {
        glfwSwapBuffers( _surface->handle() );
        return *this;
    }

public:
    Renderer3& uplink_face_culling() {
        glEnable( GL_CULL_FACE );
        return *this;
    }

    Renderer3& downlink_face_culling() {
        glDisable( GL_CULL_FACE );
        return *this;
    }

    Renderer3& downlink_wireframe() {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        return *this;
    }

    Renderer3& uplink_wireframe() {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        return *this;
    }

};



};

#else
    #warning Compiling for OpenGL without choosing this GL.
#endif
