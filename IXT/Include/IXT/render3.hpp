#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/surface.hpp>
#include <IXT/volatile-ptr.hpp>

#if defined( _ENGINE_GL_OPEN_GL )

namespace _ENGINE_NAMESPACE {



enum SHADER3_PHASE {
    SHADER3_PHASE_VERTEX   = GL_VERTEX_SHADER,
    SHADER3_PHASE_FRAGMENT = GL_FRAGMENT_SHADER
};

class Shader3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Shader3" );

public:
    Shader3() = default;

    Shader3( std::string_view path, SHADER3_PHASE phase, _ENGINE_COMMS_ECHO_ARG ) {
        std::ifstream file{ path.data(), std::ios_base::binary };

        if( !file ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Could NOT open file: \"" << path.data() << "\".";
            return;
        }

        auto file_bc = File::byte_count( file );

        GLchar* buffer = ( GLchar* ) malloc( file_bc * sizeof( GLchar ) + 1 );

        if( buffer == NULL ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Could NOT allocate for file buffer.";
            return;
        }

        struct _EXIT_PROC {
            ~_EXIT_PROC() { std::invoke( proc ); } std::function< void() > proc;
        } _exit_proc{ proc: [ &buffer ] () -> void {
            free( ( void* )std::exchange( buffer, nullptr ) );
        } };

        file.read( ( char* )buffer, file_bc );
        file.close();

        buffer[ file_bc * sizeof( GLchar ) ] = '\0';

        GLuint glidx = glCreateShader( phase );

        glShaderSource( glidx, 1, &buffer, NULL );
        glCompileShader( glidx );

        GLint success;
        glGetShaderiv( glidx, GL_COMPILE_STATUS, &success );
        if( !success ) {
            GLchar log_buf[ 512 ];
            glGetShaderInfoLog( glidx, sizeof( log_buf ), NULL, log_buf );
            
            echo( this, ECHO_LEVEL_ERROR ) << "Fault compiling: \"" << log_buf << "\", from: \"" << path.data() << "\".";
            return;
        }

        _glidx = glidx;
        echo( this, ECHO_LEVEL_OK ) << "Created from: \"" << path.data() << "\".";
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

};

class ShaderPipe3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "ShaderPipe3" );

public:
    ShaderPipe3() = default;

    ShaderPipe3( const Shader3& vert, const Shader3& frag, _ENGINE_COMMS_ECHO_ARG ) {
        GLuint glidx = glCreateProgram();

        glAttachShader( glidx, vert );
        glAttachShader( glidx, frag );

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
        echo( this, ECHO_LEVEL_OK ) << "Created.";
    }

_ENGINE_PROTECTED:
    GLuint   _glidx   = NULL;

public:
    operator decltype( _glidx ) () const {
        return _glidx;
    }

public:
    ShaderPipe3& uplink() {
        glUseProgram( _glidx );
        return *this;
    }

};



class Uniform3Unknwn : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Uniform3" );

public:
    Uniform3Unknwn() = default;

    Uniform3Unknwn( 
        ShaderPipe3&     pipe,
        std::string_view name, 
        _ENGINE_COMMS_ECHO_ARG 
    ) {
        _loc = glGetUniformLocation( pipe, name.data() );

        if( _loc == -1 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Given shader pipe has no uniform \"" << name.data() << "\".";
            return;
        }

        echo( this, ECHO_LEVEL_OK ) << "Created. Docking \"" << name.data() << "\".";
    }

_ENGINE_PROTECTED:
    GLint                      _loc           = -1;
    std::function< DWORD() >   _uplink_proc   = nullptr;

public:
    DWORD uplink() {
        return std::invoke( _uplink_proc );
    }

};

template< typename T >
class Uniform3 : public Uniform3Unknwn {
public:
    Uniform3() = default;

    Uniform3( 
        ShaderPipe3&     pipe,
        std::string_view name, 
        const T&         under = {}, 
        _ENGINE_COMMS_ECHO_ARG 
    ) : Uniform3Unknwn{ pipe, name, echo }, _under{ under }
    {
        this->_set_uplink_proc();
    }

_ENGINE_PROTECTED:
    T   _under   = {};

public:
    T& get() { return _under; }
    const T& get() const { return _under; }

public:
    using Uniform3Unknwn::uplink;

    DWORD uplink( const T& new_under ) {
        _under = new_under;
        return this->uplink();
    } 

public:
    void _set_uplink_proc(); 

};


template<> inline void Uniform3< glm::u32 >::_set_uplink_proc() {
    Uniform3Unknwn::_uplink_proc = [ this ] () -> DWORD {
        glUniform1i( _loc, _under ); 
        return 0;
    };
}
template<> inline void Uniform3< glm::vec3 >::_set_uplink_proc() {
    Uniform3Unknwn::_uplink_proc = [ this ] () -> DWORD {
        glUniform3f( _loc, _under.x, _under.y, _under.z  ); 
        return 0;
    };
}
template<> inline void Uniform3< glm::mat4 >::_set_uplink_proc() {
    Uniform3Unknwn::_uplink_proc = [ this ] () -> DWORD {
        glUniformMatrix4fv( _loc, 1, GL_FALSE, glm::value_ptr( _under ) ); 
        return 0;
    };
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

public:
    Lens3& zoom( ggfloat_t p ) {
        pos += this->forward() * p;
        return *this;
    }

    Lens3& roll( ggfloat_t angel ) {
        up = glm::rotate( up, angel, this->forward() );
        return *this;
    }

    Lens3& spin( glm::vec2 delta ) {
        glm::vec3 old_pos = pos;
        glm::vec3 fwd     = this->forward();

        ggfloat_t mag = glm::length( fwd );

        pos += this->right_n()*delta.x + up*delta.y;

        fwd = this->forward();
        pos += glm::normalize( fwd ) * ( glm::length( fwd ) - mag );

        up = glm::normalize( glm::cross( this->right(), fwd ) );
        return *this;
    }

};



class Mesh3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Mesh3" );

public:
    Mesh3( std::string_view obj_path, std::string root_dir, _ENGINE_COMMS_ECHO_ARG ) {
        DWORD                              status;
		tinyobj::attrib_t                  attrib;
		std::vector< tinyobj::shape_t >    meshes;
        std::vector< tinyobj::material_t > materials;
        size_t                             total_vrtx_count = 0;
		std::string                        error_str;

        echo( this, ECHO_LEVEL_INTEL ) << "Compiling the object: \"" << obj_path.data() << "\".";

		status = tinyobj::LoadObj( 
            &attrib, &meshes, &materials, &error_str, 
            obj_path.data(), root_dir.data(), 
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
            
            std::string* tex_name = &mtl.data.diffuse_texname;

            if( !tex_name->empty() ) {
                 if( this->_push_tex( root_dir + *tex_name, "diffuse_texture", 0, echo ) != 0 )
                    continue;

                mtl.tex_d_idx = _texs.size() - 1;
            }

            tex_name = &mtl.data.ambient_texname;

            if( !tex_name->empty() ) {
                if( this->_push_tex( root_dir + *tex_name, "ambient_texture", 1, echo ) != 0 )
                    continue;

                mtl.tex_a_idx = _texs.size() - 1;
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
        size_t                tex_a_idx   = -1;
        size_t                tex_d_idx   = -1;
        size_t                tex_s_idx   = -1;
    };
    std::vector< _Mtl >       _mtls;
    struct _Tex {
        GLuint                 glidx;
        std::string            name;
        GLuint                 unit;
        Uniform3< glm::u32 >   ufrm;
    };
    std::vector< _Tex >       _texs;

_ENGINE_PROTECTED:
    DWORD _push_tex( std::string_view path, std::string_view name, GLuint unit,  _ENGINE_COMMS_ECHO_ARG ) {
        GLuint tex_glidx;

        int x, y, n;
        UBYTE* img_buf = stbi_load( path.data(), &x, &y, &n, 4 );

        if( img_buf == nullptr ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Failed to load texture data from: \"" << path.data() << "\".";
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
            unit: unit,
            ufrm: {}
        } );

        echo( this, ECHO_LEVEL_OK ) << "Pushed texture from: \"" << path.data() << "\".";
        return 0;
    }

public:
    Mesh3& dock_in( ShaderPipe3& pipe, _ENGINE_COMMS_ECHO_RT_ARG ) {
        for( size_t idx = 0; idx < _texs.size(); ++idx )
            _texs[ idx ].ufrm = Uniform3< glm::u32 >{ pipe, _texs[ idx ].name, _texs[ idx ].unit, echo };

        return *this;
    }

public:
    Mesh3& splash() {
        for( _SubMesh& sub : _sub_meshes ) {
            glBindVertexArray( sub.VAO );

            for( auto& burst : sub.bursts ) {
                for( size_t tex_idx : {
                    _mtls[ burst.mtl_idx ].tex_a_idx, _mtls[ burst.mtl_idx ].tex_d_idx, _mtls[ burst.mtl_idx ].tex_s_idx
                } ) {
                    if( tex_idx == -1 ) continue;

                    _Tex& tex = _texs[ tex_idx ];

                    glActiveTexture( GL_TEXTURE0 + tex.unit );
                    glBindTexture( GL_TEXTURE_2D, tex.glidx );
                    tex.ufrm.uplink();
                }
                
                glDrawElements( GL_TRIANGLES, ( GLsizei )burst.count, GL_UNSIGNED_INT, 0 );
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

        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LESS );
        glEnable( GL_CULL_FACE ); 
        glCullFace( GL_BACK );
        glFrontFace( GL_CCW );
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

};



};

#else
    #warning Compiling for OpenGL without choosing this GL.
#endif
