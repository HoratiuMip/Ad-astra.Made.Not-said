/*===== BarraCUDA-CTRL Tester - Vatca "Mipsan" Tudor-Horatiu
|
> A simple program to test the controller. This has been build upon the ImGui's OpenGL example.
|
======*/

#include <NLN/init.hpp>
#include <NLN/comms.hpp>
#include <NLN/env.hpp>
#include <NLN/render3.hpp>
#include <NLN/tempo.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <NLN/Device/barracuda-ctrl-nln-driver.hpp>



std::function< bool() >   is_running; 



NLN::DEV::BarracudaCTRL   BARCUD;
struct {
    NLN::Ticker                                     tkr;
    std::pair< std::atomic_int, std::atomic_int >   tot_sent;
    std::pair< std::atomic_int, std::atomic_int >   tot_recv;

    void reset_session() {
        tkr.lap();
        tot_sent.first.store( 0, std::memory_order_relaxed );
        tot_recv.first.store( 0, std::memory_order_relaxed );
    }

    int add_sent( int val ) {
        tot_sent.first.fetch_add( val, std::memory_order_relaxed );
        tot_sent.second.fetch_add( val, std::memory_order_relaxed );
        return val;
    }
    int add_recv( int val ) {
        tot_recv.first.fetch_add( val, std::memory_order_relaxed );
        tot_recv.second.fetch_add( val, std::memory_order_relaxed );
        return val;
    }

} STATS;



class _BOARD : public NLN::Descriptor {
public:
    _BOARD( const std::string& name )
    : name{ name }
    {}

public:
    std::string   name;

public:
    virtual void frame( void ) = 0;

};

class _DASHBOARD : public NLN::Descriptor, public std::deque< std::unique_ptr< _BOARD > > {
public:
    void frame( void ) {
        for( auto& board : *this ) {
            board->frame();
        }
    }

} DASHBOARD;



class STATS_BOARD : public _BOARD {
public:
    STATS_BOARD( const std::string& name )
    : _BOARD{ name }
    {}

protected:
    NLN::Ticker   _tkr_ping       = { NLN::ticker_lap_epoch_init_t{} };
    bool          _last_ping_ok   = false;

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar );

        auto pad = ImGui::GetStyle().WindowPadding;
        if( !BARCUD.connected() ) {
            ImGui::TextColored( { 1, 0, 0, 1 }, "Disconnected." );
            ImGui::SeparatorText( "This controller session." );
            ImGui::ProgressBar( -1.0f * ( float )ImGui::GetTime(), { 400 - pad.x*2, 60 - pad.y*2 }, "Attempting connection..." );
            goto l_tester_session;
        }
        
        ImGui::TextColored( { 0, 1, 0, 1 }, "Connected." );
        
        ImGui::SeparatorText( "Quick commands." );

        if( ImGui::Button( "PING" ) ) {
            if( int ret = BARCUD.ping(); ret > 0 ) {
                STATS.add_sent( ret );
                _last_ping_ok = true;
                _tkr_ping.lap();
            } else {
                _last_ping_ok = false;
            }
        }
        if( _tkr_ping.peek_lap() < 3.6 ) {
            ImGui::SameLine();
            if( _last_ping_ok )
                ImGui::TextColored( { 0, 1, 0, 1 }, "ACK'd." );
            else 
                ImGui::TextColored( { 1, 0, 0, 1 }, "ERROR." );
            ImGui::SetItemTooltip( "Will disappear in %.1fs.", 3.6 - _tkr_ping.peek_lap() );
        }

        ImGui::SameLine( 0, 30 ); 
        if( ImGui::Button( "RESET" ) ) {
            BARCUD.disconnect( 0 );
        }

        ImGui::SameLine( 0, 30 );
        if( ImGui::Button( "BREACH PROTOCOL" ) ) {
            BARCUD.breach();
        }

        ImGui::SeparatorText( "This controller session." );

        ImGui::BulletText( "Up time: %.1fs.", STATS.tkr.peek_lap() );
        ImGui::BulletText( "Sent: %.1f kBytes.", STATS.tot_sent.first.load( std::memory_order_relaxed ) / 1000.0f );
        ImGui::BulletText( "Received: %.1f kBytes.", STATS.tot_recv.first.load( std::memory_order_relaxed ) / 1000.0f );
    
    l_tester_session:
        ImGui::SeparatorText( "This tester session." );

        ImGui::BulletText( "Up time: %.1fs.", STATS.tkr.up_time() );
        ImGui::BulletText( "Sent: %.1f kBytes.", STATS.tot_sent.second.load( std::memory_order_relaxed ) / 1000.0f );
        ImGui::BulletText( "Received: %.1f kBytes.", STATS.tot_recv.second.load( std::memory_order_relaxed ) / 1000.0f );
        
        ImGui::End();
    }

};

class JOYSTICK_BOARD : public _BOARD {
public:
    JOYSTICK_BOARD( const std::string& name, barcud_ctrl::joystick_t* js )
    : _BOARD{ name }, _js{ js }
    {}

protected:
    barcud_ctrl::joystick_t*   _js;

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 300, 360 }, ImGuiCond_Always );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_NoResize );

        ImGui::Text( "X: (%+.3f) | Y: (%+.3f) | SW: (%d)", _js->x, _js->y, _js->sw.dwn ); 
        ImGui::Separator();

        ImDrawList* imdraw = ImGui::GetWindowDrawList();
        auto org = ImGui::GetCursorScreenPos();

        org.x += 150 - ImGui::GetStyle().WindowPadding.x; org.y += 150;
        imdraw->AddLine( org, { org.x, org.y - _js->y * 140 }, IM_COL32( 0, 255, 0, 255 ), 4.0 ); 
        imdraw->AddLine( org, { org.x + _js->x * 140, org.y }, IM_COL32( 0, 0, 255, 255 ), 4.0 ); 

        float rx = _js->x; float ry = _js->y;
        float norm = NLN::Vec2::norm( rx, ry );
        if( norm > 1.0 ) { rx /= norm; ry /= norm; }
        imdraw->AddLine( org, { org.x + rx * 140, org.y - ry * 140 }, IM_COL32( 0, 255, 255, 255 ), 4.0 ); 
        
        imdraw->AddCircle( org, 140, _js->sw.dwn ? IM_COL32( 255, 255, 255, 255 ) : IM_COL32( 80, 80, 80, 255 ), 120, 4.0 );
        
        ImGui::End();
    }

};

class SWITCH_BOARD : public _BOARD {
public:
    SWITCH_BOARD( const std::string& name, ImVec4 col, barcud_ctrl::switch_t* sw )
    : _BOARD{ name }, _col{ IM_COL32( col.x, col.y, col.z, col.w ) }, _sw{ sw }
    {}

protected:
    barcud_ctrl::switch_t*   _sw;
    ImU32                    _col;

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 160, 140 }, ImGuiCond_Always );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_NoResize );

        ImGui::Text( "SW: (%d)", _sw->dwn );
        ImGui::Separator();

        ImDrawList* imdraw = ImGui::GetWindowDrawList();
        auto pad = ImGui::GetStyle().WindowPadding;
        auto org = ImGui::GetCursorScreenPos();
        org.x += 80 - pad.x; org.y += 50 - pad.y;
        
        if( _sw->dwn )
            imdraw->AddCircleFilled( org, 32, _col, 120 );
        else
            imdraw->AddCircle( org, 32, _col, 120, 3.0 );
     
        ImGui::End();
    }

};

class LIGHT_SENSOR_BOARD : public _BOARD {
public:
    LIGHT_SENSOR_BOARD( const std::string& name, barcud_ctrl::light_sensor_t* ls )
    : _BOARD{ name }, _ls{ ls }
    {}

protected:
    barcud_ctrl::light_sensor_t*   _ls;

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_NoResize );

        ImGui::VSliderFloat( "##1", { 100, 300 }, &_ls->val, 0.0, 1.0, "%.3f", ImGuiSliderFlags_NoInput );
     
        ImGui::End();
    }

};

class COMMAND_BOARD : public _BOARD {
public:
    COMMAND_BOARD( const std::string& name )
    : _BOARD{ name }
    {}

protected:

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_NoResize );

        
     
        ImGui::End();
    }

};





int main( int argc, char** argv ) {
/* Setup */
    glfwSetErrorCallback( [] ( int err, const char* desc ) -> void {
        NLN::comms( NLN::ECHO_LEVEL_ERROR ) << "GLFW error code ( " << err << " ): " << desc << ".";
    } );

    if( int ret = NLN::begin_runtime( argc, argv, NLN::BEGIN_RUNTIME_FLAG_INIT_NETWORK, nullptr, nullptr ); ret != 0 ) return ret;
   
/* GLFW window */
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    GLFWwindow* window = glfwCreateWindow( NLN::Env::w(.72), NLN::Env::h(.72) , "BarraCUDA-CTRL Tester V2", nullptr, nullptr);
    NLN_ASSERT( window != nullptr, -1 );
    glfwMakeContextCurrent( window );
    NLN::Render3 render( window );

    is_running = [ &render ] () -> bool { return !glfwWindowShouldClose( render.handle() ); };

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


    DASHBOARD.emplace_back( new STATS_BOARD{
        "Stats - General"
    } );
    DASHBOARD.emplace_back( new JOYSTICK_BOARD{
        "Rachel - Joystick", &BARCUD.dynamic.rachel
    } );
    DASHBOARD.emplace_back( new JOYSTICK_BOARD{
        "Samantha - Joystick", &BARCUD.dynamic.samantha
    } );
    DASHBOARD.emplace_back( new SWITCH_BOARD{
        "Giselle - Switch", { 0, 0, 255, 255 }, &BARCUD.dynamic.giselle
    } );
    DASHBOARD.emplace_back( new SWITCH_BOARD{
        "Karina - Switch", { 255, 0, 0, 255 }, &BARCUD.dynamic.karina
    } );
    DASHBOARD.emplace_back( new SWITCH_BOARD{
        "Ningning - Switch", { 255, 255, 0, 255 }, &BARCUD.dynamic.ningning
    } );
    DASHBOARD.emplace_back( new SWITCH_BOARD{
        "Winter - Switch", { 0, 255, 0, 255 }, &BARCUD.dynamic.winter
    } );
    DASHBOARD.emplace_back( new LIGHT_SENSOR_BOARD{
        "Naksu - Light Sensor", &BARCUD.dynamic.naksu
    } );
    DASHBOARD.emplace_back( new COMMAND_BOARD{
        "Command - Selector"
    } );

    float ax = 0.0;
    std::thread burst_th{ [ &ax ] () -> void {
    l_attempt_connect: {
        if( !is_running() )
            goto l_burst_th_end;

        NLN::comms( NLN::ECHO_LEVEL_PENDING ) << "Attempting connection to the controller...";
        if( BARCUD.connect( NLN::DEV::BARRACUDA_CTRL_FLAG_TRUST_INVOKER ) != 0 ) {
            NLN::comms() << "Could not connect to the controller. Retrying in 3s...\n";
            std::this_thread::sleep_for( std::chrono::seconds{ 3 } );
            goto l_attempt_connect;
        }
        STATS.reset_session();
        NLN::comms() << "Connected to the controller.\n";

        NLN::Ticker tkr;
        BAR_PROTO_STREAM_RESOLVE_RECV_INFO info;
        while( is_running() ) {
            int ret = BARCUD.trust_resolve_recv( &info );
            if( ret <= 0 ) break;
            STATS.add_recv( ret );
            STATS.add_sent( info.sent );

            ax += BARCUD.dynamic.gran.gyr.z / std::numeric_limits< int16_t >::max() * 250.0 * tkr.lap();
        }

        BARCUD.disconnect( 0 );
        if( is_running() )
            goto l_attempt_connect;
    } 
    l_burst_th_end:
        return;
    } };
    

    NLN::Lens3 lens{ { 0, 0, 20 }, { 0, 0, 0 }, { 0, 1, 0 } };
    NLN::Mesh3 mesh{ std::filesystem::path{ argv[ 1 ] } / "Mesh/", "BarraCUDA", NLN::MESH3_FLAG_MAKE_PIPES };
    mesh.model.uplink_bv( glm::mat4{ 1 } );

    NLN::Uniform3< glm::mat4 > view{ "view", lens.view() };
    NLN::Uniform3< glm::mat4 > proj{ "proj", glm::perspective( glm::radians( 72.0f ), render.aspect(), 0.1f, 1000.0f ) };

    mesh.pipe->pull( view, proj );
    view.uplink_bv( lens.view() );
    proj.uplink_b();
    render.uplink_wireframe();

    while( is_running() ) { mesh.model.uplink_bv( glm::rotate( glm::mat4{ 1.0 }, ax, glm::vec3( 0, 0, 1 ) ) );
        glfwPollEvents();

        if( glfwGetWindowAttrib( render.handle(), GLFW_ICONIFIED ) != 0 ) {
            ImGui_ImplGlfw_Sleep( 10 );
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DASHBOARD.frame();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        render.clear( { 0.0, 0.0, 0.0, 1.0 } );
        mesh.splash();

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent( render.handle() );

        render.swap();
    }

    if( burst_th.joinable() ) burst_th.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow( window );
    
    return NLN::end_runtime( argc, argv, 0, nullptr, nullptr );
}
