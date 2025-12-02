#pragma once
/**
 * @file: OSp/render3.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/OSp/core.hpp>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <tiny_obj_loader.h>


namespace a113 { namespace render {


enum ShaderPhase_ {
    ShaderPhase_None     = -0x1,

    ShaderPhase_Vertex   = 0x0,
    ShaderPhase_TessCtrl = 0x1,
    ShaderPhase_TessEval = 0x2,
    ShaderPhase_Geometry = 0x3,
    ShaderPhase_Fragment = 0x4,

    ShaderPhase_COUNT    = 0x5
};

inline constexpr const GLuint ShaderPhase_MAP[ ShaderPhase_COUNT ] = {
    GL_VERTEX_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_GEOMETRY_SHADER,
    GL_FRAGMENT_SHADER
};

inline constexpr const char* const ShaderPhase_FILE_EXTENSION[ ShaderPhase_COUNT ] = {
    ".vert", ".tesc", ".tese", ".geom", ".frag"
};


enum MeshFlag_ {
    MeshFlag_MakePipe = _BV( 0x0 )
};

class mesh_t {
public:
    Mesh3() = default;

    Mesh3( 
        const std::filesystem::path& root_dir_, 
        std::string_view             prefix_, 
        MeshFlag_                    flags_ 
    ) {
        status_t                           status;

		tinyobj::attrib_t                  attrib;
		std::vector< tinyobj::shape_t >    sub_meshes;
        std::vector< tinyobj::material_t > mtls;
		std::string                        error_str;

        size_t                             total_vrtx_count = 0;

        std::filesystem::path root_dir_p = root_dir_ / prefix_.data();
        std::filesystem::path obj_path   = root_dir_p; obj_path += ".obj";

        _Hyper->_Log::info( "Compiling the obj: \"{}\".", obj_path.string() );

		status = tinyobj::LoadObj( 
            &attrib, &sub_meshes, &mtls, &error_str, 
            obj_path.string().c_str(), root_dir.string().c_str(), 
            true
        );

		if( not error_str.empty() ) _Hyper->_Log::warn( "TinyObj says: \"{}\".", error_str );

        ASSERT_OR( status ) {
            _Hyper->_Log::error( "Failed to compile the obj." );
            return;
        }

		_Hyper->_Log::info( "Compiled [{}] materials over [{}] sub-meshes.", materials.size(), sub_meshes.size() );

        _mtls.reserve( mtls.size() );
        for( tinyobj::material_t& mtl_data : mtls ) { 
            _mtl_t& mtl = _mtls.emplace_back(); 
            mtl.data = std::move( mtl_data ); 

            GLuint tex_slot = 0x0;

            struct _std_tex_t {
                const char*    key;
                std::string*   name;
            } std_texs[] = {
                { "map_Ka", &mtl.data.ambient_texname },
                { "map_Kd", &mtl.data.diffuse_texname },
                { "map_Ks", &mtl.data.specular_texname },
                { "map_Ns", &mtl.data.specular_highlight_texname },
                { "map_bump", &mtl.data.bump_texname }
            };

            for( auto& [ key, name ] : std_texs ) {
                if( name->empty() ) continue;

                A113_ASSERT_OR( 0x0 == this->_push_tex( root_dir / *name, key, tex_slot, echo ) ) continue;

                mtl.tex_idxs.push_back( _texs.size() - 1 );
                ++tex_slot;
            }
        
            for( auto& [ key, value ] : mtl.data.unknown_parameter ) {
                if( not key.starts_with( "A113" ) ) {
                    _Hyper->_Log::warn( "Unknown parameter: \"{}\".", key );
                    continue;
                }

                if( std::string::npos != key.find( "map" ) && 0x0 == this->_push_tex( root_dir / value, key, tex_slot ) ) {
                    mtl.tex_idxs.push_back( _texs.size() - 1 );
                    ++tex_slot;
                    continue;
                }
 
                _Hyper->_Log::warn( "Unrecognized A113 parameter \"{}\".", key );
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

_A113_PROTECTED:
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
    struct _mtl_t {
        tinyobj::material_t     data;
        std::vector< size_t >   tex_idxs;
    };
    std::vector< _mtl_t >      _mtls;
    struct _tex_t {
        GLuint                 glidx;
        std::string            name;
        GLuint                 slot;
        Uniform3< glm::u32 >   ufrm;
    };
    std::vector< _tex_t >      _texs;

public:
    Uniform3< glm::mat4 >     model;
    Uniform3< glm::vec3 >     Kd;

    HVec< ShaderPipe3 >       pipe;

_A113_PROTECTED:
    status_t _push_tex( 
        const std::filesystem::path& path_, 
        std::string_view             name_, 
        GLuint                       slot_
    ) {
        GLuint tex_glidx;

        int x, y, n;
        unsigned char* img_buf = stbi_load( path_.string().c_str(), &x, &y, &n, 4 );

        A113_ASSERT_OR( img_buf ) {
            _Hyper->_Log::error( "Failed to load texture from: \"{}\".", path_.string() );
            return -0x1;
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

        glBindTexture( GL_TEXTURE_2D, 0x0 );
        stbi_image_free( img_buf );

        _texs.emplace_back( _tex_t{
            .glidx =  tex_glidx,
            .name  = name.data(),
            .slot  = slot_,
            .ufrm  = {}
        } );

        _Hyper->_Log::info( "Pushed texture on slot [{}], from: \"{}\". ", slot_, path.string() );
        return 0x0;
    }

public:
    Mesh3& dock_in( HVec< ShaderPipe3 > other_pipe, _ENGINE_COMMS_ECHO_RT_ARG ) {
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


class Cluster : public st_att::_Log {
public:
    /* MIP here after almost one year. Yeah.*/
    //std::cout << "GOD I SUMMON U. GIVE MIP TEO FOR A FEW DATES (AT LEAST 100)"; 
    //std::cout << "TY";

public:
    enum CacheQueryMode_ {
        CacheQueryMode_Cold, CacheQueryMode_Hot
    };

public:
    Cluster( GLFWwindow* glfwnd )
    : _Log{ std::format( "{}//render::Cluster//{}", A113_VERSION_STRING, glfwGetWindowTitle( glfwnd ) ) }, 
      _glfwnd{ glfwnd } 
    {
        glfwMakeContextCurrent( _glfwnd );

        _rend_str = ( const char* )glGetString( GL_RENDERER ); 
        _gl_str   = ( const char* )glGetString( GL_VERSION );

        glDepthFunc( GL_LESS );
        glEnable( GL_DEPTH_TEST );

        glFrontFace( GL_CCW );

        glCullFace( GL_BACK );
        glEnable( GL_CULL_FACE ); 

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable( GL_BLEND );

        glewExperimental = GL_TRUE; 
        glewInit();

        int wnd_w, wnd_h;
        glfwGetFramebufferSize( _glfwnd, &wnd_w, &wnd_h );
        glViewport( 0, 0, wnd_w, wnd_h );

        stbi_set_flip_vertically_on_load( true );

        _Log::info( "Docked on {}, using {}.", _rend_str ? _rend_str : "NULL", _gl_str ? _gl_str : "NULL" );
    }

    Cluster( const Cluster& ) = delete;
    Cluster( Cluster&& ) = delete;

_A113_PROTECTED:
    GLFWwindow*              _glfwnd     = nullptr;
 
    const char*              _rend_str   = nullptr;     
    const char*              _gl_str     = nullptr;  

_A113_PROTECTED:
    struct _internal_struct_t{ _internal_struct_t( Cluster* Hyper_ ) : _Hyper{ Hyper_ } {} Cluster* _Hyper = nullptr; };

_A113_PROTECTED:
    struct shader_t {
        shader_t() = default;
        shader_t( const std::string& strid_, GLuint glidx_ ) : strid{ strid_ }, glidx{ glidx_ } {}

        std::string   strid   = {};
        GLuint        glidx   = 0x0;
    };
    struct _shader_cache_t : public _internal_struct_t {
        std::map< std::string_view, HVec< shader_t > >   _buckets[ ShaderPhase_COUNT ]   = {};

        template< CacheQueryMode_ MODE_ >
        HVec< shader_t > query_bucket( ShaderPhase_ phase_, const std::string& strid_ ) {
            A113_ASSERT_OR( phase_ >= ShaderPhase_Vertex && phase_ < ShaderPhase_COUNT ) {
                _Hyper->_Log::error( "Shader phase for bucket querying is out of bounds." );
                return nullptr;
            }
            
            if constexpr( CacheQueryMode_Hot == MODE_ ) {
                auto [ itr, hot ] = _buckets[ phase_ ].emplace( std::make_pair( 
                    std::string_view{ strid_ }, HVec< shader_t >{}
                ) );
                if( true == hot ) {
                    itr->second = new shader_t{ strid_, 0x0 };
                }
                return itr->second;
            } else if constexpr( CacheQueryMode_Cold == MODE_ ) {
                auto itr = _buckets[ phase_ ].find( strid_ );
                return itr != _buckets[ phase_ ].end() ? itr->second : nullptr;
            }
            return nullptr;
        }

        shader_t* query_bucket_weak( ShaderPhase_ phase_, const std::string& strid_ ) {
            A113_ASSERT_OR( phase_ >= ShaderPhase_Vertex && phase_ < ShaderPhase_COUNT ) {
                _Hyper->_Log::error( "Shader phase for bucket querying is out of bounds." );
                return nullptr;
            }

            auto itr = _buckets[ phase_ ].find( strid_ );
            return itr != _buckets[ phase_ ].end() ? itr->second.get() : nullptr;
        }

        HVec< shader_t > make_shader_from_file( const std::filesystem::path& path_, ShaderPhase_ phase_ = ShaderPhase_None ) {
            status_t    status   = 0x0;
            std::string source   = {};
            std::string line     = {};

            std::string strid    = "";

            if( ShaderPhase_None == phase_ ) {
                int index = ShaderPhase_Vertex;
                for( auto file_ext : ShaderPhase_FILE_EXTENSION ) {
                    if( path_.string().ends_with( file_ext ) ) { phase_ = ( ShaderPhase_ )index; break; }   
                    ++index;
                }     
            }

            A113_ASSERT_OR( phase_ >= ShaderPhase_Vertex && phase_ < ShaderPhase_COUNT ) {
                _Hyper->_Log::error( "Shader phase for bucket querying is out of bounds." );
                return nullptr;
            }
            
            std::function< void( const std::filesystem::path& ) > accumulate_glsl = [ & ] ( const std::filesystem::path& path_ ) -> void {
                std::ifstream file{ path_, std::ios_base::binary };

                A113_ASSERT_OR( file.operator bool() ) {
                    _Hyper->_Log::error( "Could NOT open file \"{}\".", path_.string() );
                    status = -0x1; return;
                }

                while( std::getline( file, line ) ) {
                    struct _directive_t {
                        const char*   str;
                        void*         lbl;
                    } directives[] = {
                        { str: "//A113#include", lbl: &&l_directive_include },
                        { str: "//A113#strid", lbl: &&l_directive_strid }
                    };

                    std::string arg;

                    for( auto& d : directives ) {
                        if( !line.starts_with( d.str ) ) continue;
                        
                        auto q1 = line.find_first_of( '<' );
                        auto q2 = line.find_last_of( '>' );

                        A113_ASSERT_OR( q1 != std::string::npos && q2 != std::string::npos ) {
                            _Hyper->_Log::error( "DIRECTIVE[{}] argument of SHADER[{}] is ill-formed. It shall be quoted between \"<>\". ", d.str, strid );
                            status = -0x1; return;
                        }
                        
                        arg = std::string{ line.c_str() + q1 + 1, q2 - q1 - 1 };
                        goto *d.lbl;
                    }
                    goto l_code_line;
                
                l_directive_include:
                    accumulate_glsl( path_.parent_path() / arg );
                    continue;
                
                l_directive_strid:
                    A113_ASSERT_OR( strid.empty() ) {
                        _Hyper->_Log::error( "Multiple string identifiers given for SHADER[{}]<->[{}].", strid, arg );
                        status = -0x1; return;
                    }
                    strid = std::move( arg );
                    continue;

                l_code_line:
                    source += line; source += '\n';
                }
            };

            accumulate_glsl( path_ );
            
            A113_ASSERT_OR( 0x0 == status ) {
                _Hyper->_Log::error( "General failure during accumulation of source code for SHADER[{}].", strid );
                return nullptr;
            }

            if( strid.empty() ) strid = std::to_string( std::hash< std::string >{}( source ) );

            auto shader = this->query_bucket< CacheQueryMode_Hot >( phase_, strid );

            if( 0x0 == shader->glidx ) {
                A113_ASSERT_OR( 0x0 != ( shader->glidx = glCreateShader( ShaderPhase_MAP[ phase_ ] ) ) ) {
                    _Hyper->_Log::error( "Failed to create SHADER[{}].", shader->strid );
                    return nullptr;
                }

                const GLchar* const_const_const_const_const_const = source.c_str();
                glShaderSource( shader->glidx, 1, &const_const_const_const_const_const, NULL );
                glCompileShader( shader->glidx );

                GLint status; glGetShaderiv( shader->glidx, GL_COMPILE_STATUS, &status );
                A113_ASSERT_OR( status ) {
                    GLchar log_buf[ 512 ];
                    glGetShaderInfoLog( shader->glidx, sizeof( log_buf ), NULL, log_buf );
                    _Hyper->_Log::error( "Failed to compile SHADER[{}], GLIDX[{}]: \"{}\".", shader->strid, shader->glidx, log_buf );
                    return nullptr;
                }
            }

            _Hyper->_Log::info( "Created SHADER[{}], GLIDX[{}] from file: \"{}\".", shader->strid, shader->glidx, path_.string() );
            return shader;
        }

    } _shader_cache{ this };

    struct pipe_t {
        pipe_t() = default;
        pipe_t( const std::string& strid_, GLuint glidx_ ) : strid{ strid_ }, glidx{ glidx_ } {}

        std::string   strid   = {};
        GLuint        glidx   = 0x0;
    };
    struct _pipe_cache_t : public _internal_struct_t {
        inline static constexpr char   STAGE_NAME_SEP   = '\\';

        std::map< std::string, HVec< pipe_t > >   _bucket   = {};

        template< CacheQueryMode_ MODE_ >
        HVec< pipe_t > query( const std::string& strid_ ) {
            if constexpr( CacheQueryMode_Hot == MODE_ ) {
                auto [ itr, hot ] = _bucket.emplace( std::make_pair( 
                    std::string_view{ strid_ }, HVec< pipe_t >{}
                ) );
                if( true == hot ) {
                    itr->second = new pipe_t{ strid_, 0x0 };
                }
                return itr->second;
            } else if constexpr( CacheQueryMode_Cold == MODE_ ) {
                auto itr = _bucket.find( strid_ );
                return itr != _bucket.end() ? itr->second : nullptr;
            }
            return nullptr;
        }

        pipe_t* query_weak( const std::string& strid_ ) {
            auto itr = _bucket.find( strid_ );
            return itr != _bucket.end() ? itr->second.get() : nullptr;
        }

        HVec< pipe_t > make_pipe_from_array( shader_t* arr_[ ShaderPhase_COUNT ] ) {
            static const char* stage_pretties[ ShaderPhase_COUNT ] = {
                "VRTX-", ">TESC-", ">TESE-", ">GEOM-", ">FRAG"
            };
            std::string pretty = {};
            std::string strid  = {};

            for( int phase = ShaderPhase_Vertex; phase <= ShaderPhase_Fragment; ++phase ) {
                shader_t* shader = arr_[ phase ];
                if( nullptr == shader ) continue;

                pretty += stage_pretties[ phase ];
                strid += shader->strid + STAGE_NAME_SEP;
            }

            A113_ASSERT_OR( 
                false == pretty.starts_with( '>' ) && false == pretty.ends_with( '-' ) 
                && 
                nullptr != arr_[ ShaderPhase_Vertex ] && nullptr != arr_[ ShaderPhase_Fragment ]
            ) {
                _Hyper->_Log::error( "Shader stages ill-arranged: [{}]", pretty );
                return nullptr;
            }

            auto pipe = this->query< CacheQueryMode_Hot >( strid );

            if( 0x0 == pipe->glidx ) {
                pipe->glidx = glCreateProgram();

                A113_ASSERT_OR( 0x0 != pipe->glidx ) {
                    _Hyper->_Log::error( "Failed to create PIPE[{}].", pipe->strid );
                    return nullptr;
                }
    
                for( int phase = ShaderPhase_Vertex; phase <= ShaderPhase_Fragment; ++phase ) {
                    if( nullptr == arr_[ phase ] ) continue;
                    glAttachShader( pipe->glidx, arr_[ phase ]->glidx );
                }

                glLinkProgram( pipe->glidx );

                GLint status;
                glGetProgramiv( pipe->glidx, GL_LINK_STATUS, &status );
                A113_ASSERT_OR( status ) {
                    GLchar log_buf[ 512 ];
                    glGetProgramInfoLog( pipe->glidx, sizeof( log_buf ), NULL, log_buf );
                    _Hyper->_Log::error( "Failed to create PIPE[{}], GLIDX[{}]: \"{}\".", pipe->strid, pipe->glidx, log_buf );
                    return nullptr;
                }
            }

            _Hyper->_Log::info( "Created PIPE[{}], GLIDX[{}] from shader array.", pipe->strid, pipe->glidx );
            return pipe;
        }

        HVec< pipe_t > make_pipe_from_prefixed_path( const std::filesystem::path& path_ ) {
            shader_t* shaders[ ShaderPhase_COUNT ]; memset( shaders, 0x0, sizeof( shaders ) );

            for( int phase = ShaderPhase_Vertex; phase <= ShaderPhase_Fragment; ++phase ) {
                std::filesystem::path path_phase{ path_ };
                path_phase += ShaderPhase_FILE_EXTENSION[ phase ];

                if( !std::filesystem::exists( path_phase ) ) continue;
        
                shaders[ phase ] = _Hyper->_shader_cache.make_shader_from_file( path_phase, ( ShaderPhase_ )phase ).get();
            }

            return this->make_pipe_from_array( shaders );
        }

    } _pipe_cache{ this };

public:
    auto& shader_cache( void ) { return _shader_cache; }
    auto& pipe_cache( void ) { return _pipe_cache; }

public:
    void clear( glm::vec4 c = { .0, .0, .0, 1.0 } ) {
        glClearColor( c.r, c.g, c.b, c.a );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }

    void swap( void ) {
        glfwSwapBuffers( _glfwnd );
    }

public:
    void engage_face_culling( void ) {
        glEnable( GL_CULL_FACE );
    }

    void disengage_face_culling( void ) {
        glDisable( GL_CULL_FACE );
    }

    void mode_fill( void ) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    void mode_wireframe( void ) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }

    void mode_points( void ) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
    }

public:
    float aspect_ratio( void ) const {
        int wnd_w, wnd_h;
        glfwGetFramebufferSize( _glfwnd, &wnd_w, &wnd_h );
        return ( float )wnd_w / ( float )wnd_h;
    }

};


} };
