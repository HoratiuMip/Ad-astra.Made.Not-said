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
