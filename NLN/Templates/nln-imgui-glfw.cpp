/*===== Template for using NLN + ImGui + GLFW - Vatca "Mipsan" Tudor-Horatiu
|
>
|
======*/

#include <NLN/init.hpp>
#include <NLN/comms.hpp>
#include <NLN/env.hpp>
#include <NLN/render3.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <implot.h>



int main( int argc, char** argv ) {
/* Setup */
    glfwSetErrorCallback( [] ( int err, const char* desc ) -> void {
        NLN::comms( NLN::EchoLevel_Error ) << "GLFW error code ( " << err << " ): " << desc << ".";
    } );

    if( int ret = NLN::begin_runtime( argc, argv, 0, nullptr, nullptr ); ret != 0 ) return ret;
   
/* GLFW window */
    GLFWwindow* window = glfwCreateWindow( NLN::Env::w(.72), NLN::Env::h(.72) , "NLN-ImGui-GLFW Template", nullptr, nullptr);
    NLN_ASSERT( window != nullptr, -1 );
    NLN::Render3 render( window );

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
    ImGui_ImplOpenGL2_Init();

    

    while( !glfwWindowShouldClose( render.handle() ) ) {
        glfwPollEvents();

        if( glfwGetWindowAttrib( render.handle(), GLFW_ICONIFIED ) != 0 ) {
            ImGui_ImplGlfw_Sleep( 10 );
            continue;
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /*CODE HERE*/

        ImGui::Render();
        render.clear( { 0.0, 0.0, 0.0, 1.0 } );

        //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //glUseProgram(0);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        //glUseProgram(last_program);

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent( render.handle() );

        render.swap();
    }


    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow( window );
    
    return NLN::end_runtime( argc, argv, 0, nullptr, nullptr );
}
