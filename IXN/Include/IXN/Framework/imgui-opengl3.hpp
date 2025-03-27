/*===== IXN - Dear ImGui on OpenGL3 framework - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> 
|
======*/

#include <IXN/descriptor.hpp>
#include <IXN/init.hpp>
#include <IXN/comms.hpp>
#include <IXN/render3.hpp>
#include <IXN/tempo.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <atomic>


namespace _ENGINE_NAMESPACE { namespace _ENGINE_FRAMEWORK_NAMESPACE {


class ImGui_on_OpenGL3 {
public:
    struct {
        std::atomic_bool   is_running            = false;

        DWORD              runtime_begin_flags   = 0; 
        DWORD              runtime_end_flags     = 0;

        const char*        title                 = "ixN::Fwk::ImGui_on_OpenGL3";
        int                width                 = 64;
        int                height                = 64;
        glm::vec4          clear_color           = { 0.05, 0.05, 0.1, 1.0 };
        bool               iconify               = false;
    } params;

public:
    typedef   std::function< DWORD( double ) >   frame_callback_t;
    frame_callback_t   loop   = nullptr;

public:
    int main( int argc, char* argv[] ) {
    /* Setup */
        glfwSetErrorCallback( [] ( int err, const char* desc ) -> void {
            ixN::comms( ixN::EchoLevel_Error ) << "GLFW error code ( " << err << " ): " << desc << ".";
        } );

        if( int ret = ixN::begin_runtime( argc, argv, params.runtime_begin_flags, nullptr, nullptr ); ret != 0 ) return ret;
    
    /* GLFW window */
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        GLFWwindow* window = glfwCreateWindow( params.width, params.height, params.title, nullptr, nullptr);
        glfwSetWindowPos( window, 50, 50 );
        IXN_ASSERT( window != nullptr, -1 );
        glfwMakeContextCurrent( window );
        ixN::Render3 render( window );

    /* ImGui */
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

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL( render.handle(), true );
        ImGui_ImplOpenGL3_Init();
        
    /* Last checks */
        if( params.iconify ) glfwIconifyWindow( render.handle() );

        while( params.is_running.load( std::memory_order_relaxed ) && !glfwWindowShouldClose( render.handle() ) ) {
            glfwPollEvents();

            // if( glfwGetWindowAttrib( render.handle(), GLFW_ICONIFIED ) != 0 ) {
            //     ImGui_ImplGlfw_Sleep( 10 );
            //     continue;
            // }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            render.clear( params.clear_color );

            static ixN::Ticker frame_tick;
            if( this->loop( frame_tick.lap() ) != 0 ) params.is_running.store( false, std::memory_order_seq_cst );
            

            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent( render.handle() );

            render.swap();
        }

        params.is_running.store( false, std::memory_order_seq_cst );

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow( window );
        
        return ixN::end_runtime( argc, argv, params.runtime_end_flags, nullptr, nullptr );
    }

};


}; };