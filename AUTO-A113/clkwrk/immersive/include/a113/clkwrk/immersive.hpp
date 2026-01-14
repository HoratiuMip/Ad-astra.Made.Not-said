#pragma once

#include <a113/osp/render3.hpp>
#include <a113/osp/tempo.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <ImGuiFileDialog.h>

namespace a113::clkwrk {


class Imm_Frame {
public:
    virtual status_t imm_frame( double dt_, void* arg_ ) = 0;

};

class Immersive  {
public:
    typedef   std::function< status_t( double, void* ) >   frame_callback_t;

public:
    enum SrfBeginAs_ {
        SrfBeginAs_Default, SrfBeginAs_Iconify, SrfBeginAs_Maximize
    };

public:
    struct config_t {
        void*              arg                   = nullptr;

        const char*        title                 = A113_VERSION_STRING"::clkwrk::immersive";
        int                width                 = 64;
        int                height                = 64;
       // glm::vec4          clear_color           = { 0.05, 0.05, 0.1, 1.0 };
        SrfBeginAs_        srf_bgn_as            = SrfBeginAs_Default;

        frame_callback_t   loop                  = nullptr;

    } config;

public:
    typedef   std::function< status_t( void* ) >           init_callback_t;

public:
    init_callback_t    init            = nullptr;

    std::atomic_bool   init_complete   = false;
    std::atomic_bool   init_hold       = true;

    // Render3            render          = {};
    // Lens3              lens            = { glm::vec3( 0.0, 0.0, 3.0 ), glm::vec3( 0.0, 0.0, 0.0 ), glm::vec3( 0.0, 1.0, 0.0 ) };

_A113_PROTECTED:
    std::atomic_bool   _is_running   = false;

public:
    // void Wait_init_complete( void ) {
    //     init_complete.wait( false, std::memory_order_acquire );
    //     glfwMakeContextCurrent( render.handle() );
    // }

    // void Release_init_hold( void ) {
    //     glfwMakeContextCurrent( nullptr );
    //     init_hold.store( false, std::memory_order_release );
    //     init_hold.notify_one();
    // }

public:
    int main( int argc_, char* argv_[], const config_t& config_ ) {
        config = config_;

    
        A113_LOGI_IMM( "Beginning clockwork initialization..." );

        glfwInit();
        glewInit();

        glfwSetErrorCallback( [] ( int err_, const char* desc_ ) -> void {
            A113_LOGE_IMM( "GLFW error [{}] occured: \"{}\".", err_, desc_ );
        } );
    
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
        glfwWindowHint( GLFW_DECORATED, GL_TRUE );
        if( SrfBeginAs_Maximize == config.srf_bgn_as ) glfwWindowHint( GLFW_MAXIMIZED, GL_TRUE );
        
        GLFWwindow* window = glfwCreateWindow( config.width, config.height, config.title, nullptr, nullptr );

        A113_ASSERT_OR( window ) { A113_LOGE_IMM( "Bad window handle." ); return -0x1; }
        A113_LOGI_IMM( "Graphics Library Window handle ok..." );

        glfwMakeContextCurrent( window );

        // new ( &render ) Render3{ window };

        // glfwSetWindowUserPointer( render.handle(), params.arg );

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        struct {
            ImGuiIO&      io      = ImGui::GetIO();
            ImGuiStyle&   style   = ImGui::GetStyle();
        } imgui;

        imgui.io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

        imgui.style.WindowRounding = 0.0f; 
        imgui.style.Colors[ ImGuiCol_WindowBg ].w = 1.0f;
        imgui.style.WindowPadding = { 10, 10 };

        ImGui_ImplGlfw_InitForOpenGL( window, true );
        ImGui_ImplOpenGL3_Init();

        A113_LOGI_IMM( "Immediate Mode Graphical User Interface ( ImGui ) ok..." );
        
        if( SrfBeginAs_Iconify == config.srf_bgn_as ) glfwIconifyWindow( window );

        //if( init ) if( init( params.arg ) != 0 ) goto l_end;

        // glfwMakeContextCurrent( nullptr );
        // init_complete.store( true, std::memory_order_release);
        // init_complete.notify_all();

        // init_hold.wait( true, std::memory_order_acquire );
        // glfwMakeContextCurrent( render.handle() );
        
        A113_LOGI_IMM( "Clockwork initialization complete." );

        _is_running.store( true, std::memory_order_release );
        while( _is_running.load( std::memory_order_relaxed ) && !glfwWindowShouldClose( window ) ) {
            glfwPollEvents();

            // if( glfwGetWindowAttrib( render.handle(), GLFW_ICONIFIED ) != 0 ) {
            //     ImGui_ImplGlfw_Sleep( 10 );
            //     continue;
            // }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //render.clear( params.clear_color );

            A113_ASSERT_OR( this->config.loop( imgui.io.DeltaTime, config.arg ) == 0x0 ) _is_running.store( false, std::memory_order_seq_cst );

            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent( window );
                glfwSwapBuffers( window );
            //render.swap();
        }

    l_end:
        _is_running.store( false, std::memory_order_seq_cst );

        A113_LOGI_IMM( "Shutting down framework..." );

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow( window );

        A113_LOGI_IMM( "Shutdown ok." );
        return 0x0;
    }

};


}