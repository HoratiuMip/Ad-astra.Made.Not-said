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
#include <stb_image.h>
#include <tiny_obj_loader.h>


namespace A113 { namespace OSp {


enum ShaderPhase_ {
    ShaderPhase_Vertex   = 0,
    ShaderPhase_TessCtrl = 1,
    ShaderPhase_TessEval = 2,
    ShaderPhase_Geometry = 3,
    ShaderPhase_Fragment = 4,

    ShaderPhase_COUNT    = 5
};

inline constexpr const GLuint ShaderPhase_MAP[ ShaderPhase_COUNT ] = {
    GL_VERTEX_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_GEOMETRY_SHADER,
    GL_FRAGMENT_SHADER
};


class RenderCluster : public PRZ_logz_stuff {
public:
    /* MIP here after almost one year. Yeah.*/
    //std::cout << "GOD I SUMMON U. GIVE MIP TEO FOR A FEW DATES (AT LEAST 100)"; 
    //std::cout << "TY";

public:
    RenderCluster( GLFWwindow* glfwnd )
    : PRZ_logz_stuff{ std::format( "{}//RenderCluster", A113_VERSION_STRING ).c_str() }, 
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

        _Log->info( "Docked on {}, using {}.", _rend_str, _gl_str );
    }

    RenderCluster( const RenderCluster& ) = delete;
    RenderCluster( RenderCluster&& ) = delete;

_A113_PROTECTED:
    GLFWwindow*              _glfwnd     = nullptr;
 
    const char*              _rend_str   = nullptr;     
    const char*              _gl_str     = nullptr;  

_A113_PROTECTED:
    struct shader_t {
        std::string   strid   = {};
        GLuint        glidx   = 0x0;
    };
    struct _shader_cache_t : public PRZ_logz_stuff {
        std::map< std::string_view, HVEC< shader_t > >   _buckets[ ShaderPhase_COUNT ]   = {};

        HVEC< shader_t > query_bucket( ShaderPhase_ phase_, const std::string& strid_, bool make_hot_ ) {
            A113_ASSERT_OR( phase_ >= 0 && phase_ < ShaderPhase_COUNT ) {
                _Log->error( "Shader phase for bucket querying is out of bounds." );
                return nullptr;
            }
            
            if( true == make_hot_ ) {
                 HVEC< shader_t > shader = new shader_t{ strid: strid_, glidx: 0x0 };
                auto [ itr, hot ] = _buckets[ phase_ ].emplace( std::make_pair( 
                    std::string_view{ shader->strid }, std::move( shader )
                ) );
                return itr->second;
            } else {
                auto itr = _buckets[ phase_ ].find( strid_ );
                return itr != _buckets[ phase_ ].end() ? itr->second : nullptr;
            }
            return nullptr;
        }

        HVEC< shader_t > make_shader_from_file( const std::filesystem::path& path_, ShaderPhase_ phase_ ) {
            RESULT      result   = 0x0;
            std::string source   = {};
            std::string line     = {};

            std::string strid    = "";
            

            std::function< void( const std::filesystem::path& ) > accumulate_glsl = [ & ] ( const std::filesystem::path& path_ ) -> void {
                std::ifstream file{ path_, std::ios_base::binary };

                A113_ASSERT_OR( file.operator bool() ) {
                    _Log->error( "Could NOT open file \"{}\".", path_.string() );
                    result = -0x1; return;
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
                            _Log->error( "DIRECTIVE[{}] argument of SHADER[{}] is ill-formed. It shall be quoted between \"<>\". ", d.str, strid );
                            result = -0x1; return;
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
                        _Log->error( "Multiple string identifiers given for SHADER[{}]<->[{}].", strid, arg );
                        result = -0x1; return;
                    }
                    strid = std::move( arg );
                    continue;

                l_code_line:
                    source += line; source += '\n';
                }
            };

            accumulate_glsl( path_ );
            
            A113_ASSERT_OR( 0x0 == result ) {
                _Log->error( "General failure during accumulation of source code for SHADER[{}].", strid );
                return nullptr;
            }

            if( strid.empty() ) strid = std::to_string( std::hash< std::string >{}( source ) );

            auto shader = this->query_bucket( phase_, strid, true );

            if( 0x0 == shader->glidx ) {
                A113_ASSERT_OR( 0x0 != ( shader->glidx = glCreateShader( ShaderPhase_MAP[ phase_ ] ) ) ) {
                    _Log->error( "Failed to create SHADER[{}].", shader->strid );
                    return nullptr;
                }

                const GLchar* const_const_const_const_const_const = source.c_str();
                glShaderSource( shader->glidx, 1, &const_const_const_const_const_const, NULL );
                glCompileShader( shader->glidx );

                GLint status; glGetShaderiv( shader->glidx, GL_COMPILE_STATUS, &status );
                A113_ASSERT_OR( status ) {
                    GLchar log_buf[ 512 ];
                    glGetShaderInfoLog( shader->glidx, sizeof( log_buf ), NULL, log_buf );
                    _Log->error( "Failed to compile SHADER[{}], GLIDX[{}]: \"{}\".", shader->strid, shader->glidx, log_buf );
                    return nullptr;
                }
            }

            _Log->info( "Created SHADER[{}], GLIDX[{}] from file: \"{}\".", shader->strid, shader->glidx, path_.string() );
            return shader;
        }

    } _shader_cache{ _Log };

    // struct _pipe_cache_t : public PRZ_logz_stuff {
    //     std::map< std::string, HVEC< GLuint > >   _bucket   = {};

    //     HVEC< GLuint > query( const std::string& strid_, bool make_hot_ ) {
    //         if( true == make_hot_ ) {
    //             auto [ itr, hot ] = _bucket.emplace( std::make_pair( strid_, HVEC< GLuint >{ new GLuint{ 0x0 } } ) );
    //             return itr->second;
    //         } else {
    //             auto itr = _bucket.find( strid_ );
    //             return itr != _bucket.end() ? itr->second : nullptr;
    //         }
    //         return nullptr;
    //     }

    //     HVEC< GLuint > make_pipe_from_array( GLuint* arr_[ ShaderPhase_COUNT ] ) {
    //         static const char* stage_pretties[ ShaderPhase_COUNT ] = {
    //             "VRTX-", ">TESC-", ">TESE-", ">GEOM-", ">FRAG"
    //         };
    //         for( int idx = 0; idx < ShaderPhase_COUNT; ++idx ) {

    //         }
    //     }

    // } _pipe_cache{ _Log };

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
