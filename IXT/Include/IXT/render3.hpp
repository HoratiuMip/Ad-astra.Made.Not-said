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
    T       _under   = {};

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
        glUniform1i( _loc, _under  ); 
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
    struct Vrtx {
        glm::vec3   pos;
        glm::vec3   nrm;
        glm::vec2   txt;
    };

    struct Txt {
        GLuint        glidx;
        std::string   kind;

        static Txt from_file( std::string_view path, std::string_view kind, _ENGINE_COMMS_ECHO_RT_ARG ) {
            static struct _FuncDescriptor : public Descriptor {
                _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Mesh3::Txt" );
            } _func_descriptor;

            Txt txt{ glidx: 0, kind: kind.data() };

            int x, y, n;
            UBYTE* img_buf = stbi_load( path.data(), &x, &y, &n, 4 );

            if( img_buf == nullptr ) {
                echo( &_func_descriptor, ECHO_LEVEL_ERROR ) << "Could not load image data from \"" << path.data() << "\".";
                return { 0, "" };
            }
           
            if( ( x & ( x - 1 ) ) != 0 || ( y & ( y - 1 ) ) != 0 )
                echo( &_func_descriptor, ECHO_LEVEL_WARNING ) << "One or both image data sizes are not powers of 2: " << x << "x" << y << ".";

            int    width_in_bytes = x * 4;
            UBYTE* top            = nullptr;
            UBYTE* bottom         = nullptr;
            UBYTE  temp           = 0;
            int    half_height    = y / 2;

            for( int row = 0; row < half_height; ++row ) {
                top = img_buf + row * width_in_bytes;
                bottom = img_buf + ( y - row - 1 ) * width_in_bytes;

                for ( int col = 0; col < width_in_bytes; ++col ) {
                    temp    = *top;
                    *top    = *bottom;
                    *bottom = temp;
                    
                    ++top; ++bottom;
                }
            }

            glGenTextures( 1, &txt.glidx );
            glBindTexture( GL_TEXTURE_2D, txt.glidx );
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

            echo( &_func_descriptor, ECHO_LEVEL_OK ) << "Created.";
            return txt;
        }
    };

    struct Mtl {
        glm::vec3   ambient;
        glm::vec3   diffuse;
        glm::vec3   specular;
    };

    struct Buffs {
        GLuint   VAO;
        GLuint   VBO;
        GLuint   EBO;
    };

public:
    Mesh3() = default;

    Mesh3( 
        std::vector< Vrtx >    vrtxs, 
        std::vector< GLuint >  glidxs, 
        std::vector< Txt >     txts,
        _ENGINE_COMMS_ECHO_ARG
    )
    : _vrtxs{ std::move( vrtxs ) }, _glidxs{ std::move( glidxs ) }, _txts{ info: std::move( txts ), ufrms: {} }
    {
		glGenVertexArrays( 1, &_buffs.VAO );
		glGenBuffers( 1, &_buffs.VBO);
		glGenBuffers( 1, &_buffs.EBO);

		glBindVertexArray( _buffs.VAO );
	
		glBindBuffer( GL_ARRAY_BUFFER, _buffs.VBO );
		glBufferData( GL_ARRAY_BUFFER, _vrtxs.size() * sizeof( Vrtx ), _vrtxs.data(), GL_STATIC_DRAW );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _buffs.EBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, _glidxs.size() * sizeof( GLuint ), _glidxs.data(), GL_STATIC_DRAW );

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vrtx ), ( GLvoid* )0 );
		
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vrtx ), ( GLvoid* )offsetof( Vrtx, nrm ) );
	
		glEnableVertexAttribArray( 2) ;
		glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( Vrtx ), ( GLvoid* )offsetof( Vrtx, txt ) );

		glBindVertexArray( 0 );

        echo( this, ECHO_LEVEL_OK ) << "Created, " << _vrtxs.size() << " vertices.";
    }

    Mesh3( 
        ShaderPipe3&           pipe,
        std::vector< Vrtx >    vrtxs, 
        std::vector< GLuint >  glidxs, 
        std::vector< Txt >     txts,
        _ENGINE_COMMS_ECHO_ARG
    ) : Mesh3{ std::move( vrtxs ), std::move( glidxs ), std::move( txts ) }
    {
        this->dock_in( pipe );
    }

    ~Mesh3() {
        glDeleteBuffers( 1, &_buffs.VBO );
        glDeleteBuffers( 1, &_buffs.EBO );
        glDeleteVertexArrays( 1, &_buffs.VAO );
    }

_ENGINE_PROTECTED:
    std::vector< Vrtx >     _vrtxs;
    std::vector< GLuint >   _glidxs;
    struct {
        std::vector< Txt >                    info;
        std::vector< Uniform3< glm::u32 > >   ufrms;
    }                       _txts;
    Buffs                   _buffs;

public:
    Buffs buffs() const {
        return _buffs;
    }

public:
    Mesh3& splash() {
        for( GLuint idx = 0; idx < _txts.info.size(); ++idx ) {
			glActiveTexture( GL_TEXTURE0 + idx );
			_txts.ufrms[ idx ].uplink( idx );
			glBindTexture( GL_TEXTURE_2D, _txts.info[ idx ].glidx );
		}

		glBindVertexArray( _buffs.VAO );
		glDrawElements( GL_TRIANGLES, ( GLsizei )_glidxs.size(), GL_UNSIGNED_INT, 0 );
		glBindVertexArray( 0 );

        for( GLuint idx = 0; idx < _txts.info.size(); ++idx ) {
            glActiveTexture( GL_TEXTURE0 + idx );
            glBindTexture( GL_TEXTURE_2D, 0 );
        }

        return *this;
    }

    Mesh3& splash( ShaderPipe3& pipe ) {
        pipe.uplink();
        return this->splash();
    }

    Mesh3& dock_in( ShaderPipe3& pipe, _ENGINE_COMMS_ECHO_RT_ARG ) {
        _txts.ufrms.clear();
        _txts.ufrms.reserve( _txts.info.size() );
        for( size_t idx = 0; idx < _txts.info.size(); ++idx )
            _txts.ufrms.emplace_back( pipe, _txts.info[ idx ].kind.c_str(), 0, echo );

        return *this;
    }

};

class Object3 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Object3" );

public:
    Object3() = default;
    
	Object3( std::string_view obj_path, std::string root_path, _ENGINE_COMMS_ECHO_ARG ) {
        DWORD                              status;
		tinyobj::attrib_t                  attrib;
		std::vector< tinyobj::shape_t >    shapes;
		std::vector< tinyobj::material_t > materials;
        size_t                             total_vrtx_count = 0;
		std::string                        error;

        echo( this, ECHO_LEVEL_INTEL ) << "Loading obj: \"" << obj_path.data() << "\".";

		status = tinyobj::LoadObj( 
            &attrib, &shapes, &materials, &error, 
            obj_path.data(), root_path.data(), 
            GL_TRUE
        );

		if( !error.empty() )
            echo( this, ECHO_LEVEL_WARNING ) << "Obj load generated the message: \"" << error << "\".";

		if( status == 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Could not load obj.";
            return;
		}

		echo( this, ECHO_LEVEL_OK ) << "Loaded " << shapes.size() << " shapes, and " << materials.size() << " materials."; 

		for( auto& shape : shapes ) {
			std::vector< Mesh3::Vrtx > vrtxs;
			std::vector< GLuint >      glidxs;
			std::vector< Mesh3::Txt >  txts;

			size_t idx_off = 0;
			for( auto& face_vrtx_count : shape.mesh.num_face_vertices ) {
				for( std::remove_reference_t< decltype( face_vrtx_count ) > v_idx = 0; v_idx < face_vrtx_count; ++v_idx ) {
					tinyobj::index_t idx = shape.mesh.indices[ idx_off + v_idx ];

					vrtxs.emplace_back( Mesh3::Vrtx{
                        pos: *( glm::vec3* )( attrib.vertices.data() + 3*idx.vertex_index ),
                        nrm: *( glm::vec3* )( attrib.normals.data() + 3*idx.normal_index ),
                        txt: ( idx.texcoord_index != -1 ) ? ( *( glm::vec2* )( attrib.texcoords.data() + 2*idx.texcoord_index ) ) : glm::vec2{ 1.0 }
                    } );

					glidxs.push_back( ( GLuint )( idx_off + v_idx ) );
				}

				idx_off += face_vrtx_count;
			}

            echo( this, ECHO_LEVEL_OK ) << "Parsed the vertex data.";

			if( shape.mesh.material_ids.size() > 0 && materials.size() > 0 ) {
				DWORD mtl_idx = shape.mesh.material_ids[ 0 ];
				if (mtl_idx != -1) {

					Mesh3::Mtl currentMaterial;
					currentMaterial.ambient = glm::vec3(materials[mtl_idx].ambient[0], materials[mtl_idx].ambient[1], materials[mtl_idx].ambient[2]);
					currentMaterial.diffuse = glm::vec3(materials[mtl_idx].diffuse[0], materials[mtl_idx].diffuse[1], materials[mtl_idx].diffuse[2]);
					currentMaterial.specular = glm::vec3(materials[mtl_idx].specular[0], materials[mtl_idx].specular[1], materials[mtl_idx].specular[2]);

					//ambient texture
					std::string ambientTexturePath = materials[mtl_idx].ambient_texname;

					if (!ambientTexturePath.empty()) {

						IXT::Mesh3::Txt currentTexture;
						currentTexture = Mesh3::Txt::from_file( root_path + ambientTexturePath, "ambient_texture", echo );
						txts.push_back(currentTexture);
					}

					//diffuse texture
					std::string diffuseTexturePath = materials[mtl_idx].diffuse_texname;

					if (!diffuseTexturePath.empty()) {

						IXT::Mesh3::Txt currentTexture;
						currentTexture = Mesh3::Txt::from_file( root_path + diffuseTexturePath, "diffuse_texture", echo );
						txts.push_back(currentTexture);
					}

					//specular texture
					std::string specularTexturePath = materials[mtl_idx].specular_texname;

					if (!specularTexturePath.empty()) {

						IXT::Mesh3::Txt currentTexture;
						currentTexture = Mesh3::Txt::from_file( root_path + specularTexturePath, "specularTexture", echo );
						txts.push_back(currentTexture);
					}
				}
			}

            total_vrtx_count += vrtxs.size();

			_meshes.emplace_back( vrtxs, glidxs, txts, echo );
            echo( this, ECHO_LEVEL_OK ) << "Pushed mesh.";
		}

        echo( this, ECHO_LEVEL_OK ) << "Created, a total of " << total_vrtx_count << " vertices.";
	}

    
	~Object3() {
        for( auto& text : _texts )
            glDeleteTextures( 1, &text.glidx );
	}

_ENGINE_PROTECTED:
    std::vector< Mesh3 >        _meshes;
    std::vector< Mesh3::Txt >   _texts;

public:
    Mesh3& operator [] ( size_t idx ) {
        return _meshes[ idx ];
    }

public:
	Object3& splash() {
        for( auto& mesh : _meshes )
            mesh.splash();

        return *this;
	}

    Object3& splash( ShaderPipe3& pipe ) {
        pipe.uplink();
        return this->splash();
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
