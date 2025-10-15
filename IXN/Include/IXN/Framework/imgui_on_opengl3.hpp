/*===== IXN Framework - Dear ImGui on OpenGL3 - Vatca "Mipsan" Tudor-Horatiu
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

        void*              arg                   = nullptr;

        const char*        title                 = "ixN::Fwk::ImGui_on_OpenGL3";
        int                width                 = 64;
        int                height                = 64;
        glm::vec4          clear_color           = { 0.05, 0.05, 0.1, 1.0 };
        bool               iconify               = false;
        bool               maximize              = false;

        int                lens_scheme           = 0;
    } params;

    struct {
        Uniform3< glm::mat4 >   view   = { "view", glm::mat4{ 1.0 } };
        Uniform3< glm::mat4 >   proj   = { "proj", glm::mat4{ 1.0 } };
    } uniforms;

public:
    typedef   std::function< DWORD( double, void* ) >   frame_callback_t;
    typedef   std::function< DWORD( void* ) >           init_callback_t;

public:
    frame_callback_t   loop            = nullptr;
    init_callback_t    init            = nullptr;

    std::atomic_bool   init_complete   = false;
    std::atomic_bool   init_hold       = true;

    Render3            render          = {};
    Lens3              lens            = { glm::vec3( 0.0, 0.0, 3.0 ), glm::vec3( 0.0, 0.0, 0.0 ), glm::vec3( 0.0, 1.0, 0.0 ) };

public:
    void Wait_init_complete( void ) {
        init_complete.wait( false, std::memory_order_acquire );
        glfwMakeContextCurrent( render.handle() );
    }

    void Release_init_hold( void ) {
        glfwMakeContextCurrent( nullptr );
        init_hold.store( false, std::memory_order_release );
        init_hold.notify_one();
    }

public:
    int main( int argc, char* argv[] ) {
    /* Setup */
        glfwSetErrorCallback( [] ( int err, const char* desc ) -> void {
            ixN::comms( ixN::EchoLevel_Error ) << "GLFW error code ( " << err << " ): " << desc << ".";
        } );

        if( DWORD status = ixN::begin_runtime( argc, argv, params.runtime_begin_flags, nullptr, nullptr ); status != 0 ) return status;
    
    /* GLFW window */
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
        glfwWindowHint( GLFW_DECORATED, GL_TRUE );
        if( params.maximize ) glfwWindowHint( GLFW_MAXIMIZED, GL_TRUE );

        GLFWwindow* window = glfwCreateWindow( params.width, params.height, params.title, nullptr, nullptr );

        IXN_ASSERT( window != nullptr, -1 );
        glfwMakeContextCurrent( window );

        new ( &render ) Render3{ window };

        glfwSetWindowUserPointer( render.handle(), params.arg );

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

        ImGui_ImplGlfw_InitForOpenGL( render.handle(), true );
        ImGui_ImplOpenGL3_Init();
        
    /* Last checks */
        if( params.iconify ) glfwIconifyWindow( render.handle() );

    /* Client sync */
        if( init ) if( init( params.arg ) != 0 ) goto l_end;

        glfwMakeContextCurrent( nullptr );
        init_complete.store( true, std::memory_order_release);
        init_complete.notify_all();

        init_hold.wait( true, std::memory_order_acquire );
        glfwMakeContextCurrent( render.handle() );

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
            if( this->loop( frame_tick.lap(), params.arg ) != 0 ) params.is_running.store( false, std::memory_order_seq_cst );
            // ImGui::ShowDemoWindow();
            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent( render.handle() );

            render.swap();
        }

    l_end:
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