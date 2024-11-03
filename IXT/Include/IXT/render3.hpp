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
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Uniform3Unknwn" );

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
    GLint   _loc   = -1;

public:


public:
    virtual Uniform3Unknwn& uplink() = 0;

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
    {}

_ENGINE_PROTECTED:
    T       _under   = {};

public:
    T& get() { return _under; }
    const T& get() const { return _under; }

public:
    Uniform3Unknwn& uplink() override {
        glUniformMatrix4fv( this->_loc, 1, GL_FALSE, glm::value_ptr( _under ) ); 
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

        echo( this, ECHO_LEVEL_OK ) << "Created.";
    }

    Renderer3( const Renderer3& ) = delete;
    Renderer3( Renderer3&& ) = delete;

_ENGINE_PROTECTED:
    VPtr< Surface >   _surface    = NULL;

    const char*       _rend_str   = NULL;     
    const char*       _gl_str     = NULL;    

};



};

#else
    #warning Compiling for OpenGL without choosing this GL.
#endif
