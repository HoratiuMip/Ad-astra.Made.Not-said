/*===== BarraCUDA-CTRL Tester V2 - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> A simple program to test the controller. This has been build upon the ImGui's OpenGL example.
|
======*/

#include <IXN/init.hpp>
#include <IXN/comms.hpp>
#include <IXN/env.hpp>
#include <IXN/render3.hpp>
#include <IXN/tempo.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <IXN/Device/barracuda-ctrl-nln-driver.hpp>



std::function< bool() >   G_is_running; 
std::filesystem::path     G_root_dir;



ixN::Dev::BarracudaCTRL   BARCUD;
struct {
    ixN::Ticker                                     tkr;
    std::pair< std::atomic_int, std::atomic_int >   tot_wb_sent;
    std::pair< std::atomic_int, std::atomic_int >   tot_wb_recv;
    std::pair< std::atomic_int, std::atomic_int >   tot_brst_sent;
    std::pair< std::atomic_int, std::atomic_int >   tot_brst_recv;

    void reset_session() {
        tkr.lap();
        tot_wb_sent.first.store( 0, std::memory_order_relaxed );
        tot_wb_recv.first.store( 0, std::memory_order_relaxed );
        tot_brst_sent.first.store( 0, std::memory_order_relaxed );
        tot_brst_recv.first.store( 0, std::memory_order_relaxed );
    }

    int add_wb_sent( int val ) {
        tot_wb_sent.first.fetch_add( val, std::memory_order_relaxed );
        tot_wb_sent.second.fetch_add( val, std::memory_order_relaxed );
        return val;
    }
    int add_wb_recv( int val ) {
        tot_wb_recv.first.fetch_add( val, std::memory_order_relaxed );
        tot_wb_recv.second.fetch_add( val, std::memory_order_relaxed );
        return val;
    }
    int add_brst_sent( int val ) {
        tot_brst_sent.first.fetch_add( val, std::memory_order_relaxed );
        tot_brst_sent.second.fetch_add( val, std::memory_order_relaxed );
        return val;
    }
    int add_brst_recv( int val ) {
        tot_brst_recv.first.fetch_add( val, std::memory_order_relaxed );
        tot_brst_recv.second.fetch_add( val, std::memory_order_relaxed );
        return val;
    }

} STATS;



class _BOARD : public ixN::Descriptor {
public:
    _BOARD( const std::string& name )
    : name{ name }
    {}

public:
    std::string   name;

public:
    virtual void frame( void ) = 0;

};

class _DASHBOARD : public ixN::Descriptor, public std::deque< std::unique_ptr< _BOARD > > {
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

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_None );

        auto pad = ImGui::GetStyle().WindowPadding;
        if( !BARCUD.connected() ) {
            ImGui::TextColored( { 1, 0, 0, 1 }, "Disconnected." );
            ImGui::SeparatorText( "This controller session." );
            ImGui::ProgressBar( -1.0f * ( float )ImGui::GetTime(), { 400 - pad.x*2, 60 - pad.y*2 }, "Attempting connection..." );
            goto l_tester_session;
        }
        
        ImGui::TextColored( { 0, 1, 0, 1 }, "Connected." );
        
        ImGui::Separator();

        static int              ping_thread_count = 1;
        static std::atomic_int  last_ping_count   = { 1 };
        static std::atomic_int  last_ping_target  = { 1 };
        if( last_ping_target == last_ping_count ) { 
            if( ImGui::Button( "PING" ) ) {
                last_ping_count = 0;
                last_ping_target = ping_thread_count;
                for( int idx = 0; idx < ping_thread_count; ++idx ) {
                    std::thread{ [] () -> void {
                        if( int ret = BARCUD.ping(); ret > 0 ) {
                            STATS.add_wb_sent( ret );
                            ++last_ping_count;
                        }
                    } }.detach();
                }
            } 
            ImGui::SameLine();
        }
        ImGui::SetNextItemWidth( 100 );
        ImGui::DragInt( "Threads | ", &ping_thread_count, 0.1f, 1, 50 );

        ImGui::SameLine();
        if( last_ping_target == last_ping_count )
            ImGui::TextColored( { 0, 1, 0, 1 }, "Responses: (%d).", last_ping_count.load() );
        else 
            ImGui::TextColored( { 1, 0.5f, 0, 1 }, "Responses: (%d).", last_ping_count.load() );
        
        ImGui::Separator();
        if( ImGui::Button( "RESET" ) ) {
            BARCUD.disconnect( 0 );
        }

        ImGui::SameLine( 0, 30 );
        if( ImGui::Button( "BREACH PROTOCOL" ) ) {
            BARCUD.breach();
        }

        ImGui::SeparatorText( "This controller session." );

        ImGui::BulletText( "Up time: %.1fs.", STATS.tkr.peek_lap() );
        ImGui::BulletText( "Wait-back sent: %d bytes.", STATS.tot_wb_sent.first.load( std::memory_order_relaxed ) );
        ImGui::BulletText( "Wait-back received: %d bytes.", STATS.tot_wb_recv.first.load( std::memory_order_relaxed ) );
        ImGui::BulletText( "Burst sent: %.1f kBytes.", STATS.tot_brst_sent.first.load( std::memory_order_relaxed ) / 1000.0f );
        ImGui::BulletText( "Burst received: %.1f kBytes.", STATS.tot_brst_recv.first.load( std::memory_order_relaxed ) / 1000.0f );
    
    l_tester_session:
        ImGui::SeparatorText( "This tester session." );

        ImGui::BulletText( "Up time: %.1fs.", STATS.tkr.up_time() );
        ImGui::BulletText( "Wait-back sent: %d bytes.", STATS.tot_wb_sent.second.load( std::memory_order_relaxed ) );
        ImGui::BulletText( "Wait-back received: %d bytes.", STATS.tot_wb_recv.second.load( std::memory_order_relaxed ) );
        ImGui::BulletText( "Burst sent: %.1f kBytes.", STATS.tot_brst_sent.second.load( std::memory_order_relaxed ) / 1000.0f );
        ImGui::BulletText( "Burst received: %.1f kBytes.", STATS.tot_brst_recv.second.load( std::memory_order_relaxed ) / 1000.0f );
        
        ImGui::End();
    }

};

class JOYSTICK_BOARD : public _BOARD {
public:
    JOYSTICK_BOARD( const std::string& name, barra::joystick_t* js )
    : _BOARD{ name }, _js{ js }
    {}

protected:
    barra::joystick_t*   _js;

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
        float norm = ixN::Vec2::norm( rx, ry );
        if( norm > 1.0 ) { rx /= norm; ry /= norm; }
        imdraw->AddLine( org, { org.x + rx * 140, org.y - ry * 140 }, IM_COL32( 0, 255, 255, 255 ), 4.0 ); 
        
        imdraw->AddCircle( org, 140, _js->sw.dwn ? IM_COL32( 255, 255, 255, 255 ) : IM_COL32( 80, 80, 80, 255 ), 120, 4.0 );
        
        ImGui::End();
    }

};

class SWITCH_BOARD : public _BOARD {
public:
    SWITCH_BOARD( const std::string& name, ImVec4 col, barra::switch_t* sw )
    : _BOARD{ name }, _col{ IM_COL32( col.x, col.y, col.z, col.w ) }, _sw{ sw }
    {
        memset( event_arr.data(), 0, sizeof( float ) * event_arr_size );
    }

protected:
    barra::switch_t*   _sw;
    ImU32                    _col;

    static constexpr int                  event_arr_size   = 2'000;
    std::array< float, event_arr_size >   event_arr;
    int                                   event_arr_at     = 0;

protected:
    void inc_event_at() {
        ++event_arr_at; if( event_arr_at >= event_arr_size ) event_arr_at = 0;
    }

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 160, 140 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_None );

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

        
        event_arr[ event_arr_at ] = 0;
        if( _sw->prs ) event_arr[ event_arr_at ] = 1.0f;
        if( _sw->rls ) {
            if( _sw->prs ) this->inc_event_at();
            event_arr[ event_arr_at ] = -1.0f;
        }

        int next_at = event_arr_at + 1; if( next_at >= event_arr_size ) next_at = 0;

        ImGui::SetCursorScreenPos( { org.x + 60, org.y - 32 } );
        ImGui::PlotLines( "Events", event_arr.data(), event_arr.size(), next_at, "", -1.0f, 1.0f, { 360, 64 } );

        this->inc_event_at();
     
        ImGui::End();
    }

};

class LIGHT_SENSOR_BOARD : public _BOARD {
public:
    LIGHT_SENSOR_BOARD( const std::string& name, barra::light_sensor_t* ls )
    : _BOARD{ name }, _ls{ ls }
    {}

protected:
    barra::light_sensor_t*   _ls;

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_NoResize );

        ImGui::VSliderFloat( "##1", { 100, 300 }, &_ls->lvl, 0.0, 1.0, "%.3f", ImGuiSliderFlags_NoInput );
     
        ImGui::End();
    }

};

class ACCEL_GYRO_BOARD : public _BOARD {
public:
    ACCEL_GYRO_BOARD( const std::string& name, barra::gyro_t* gr )
    : _BOARD{ name }, _gr{ gr }
    {}

protected:
    barra::gyro_t*   _gr;

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 540, 100 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_None );

        static constexpr int arr_size = 2'000;
        static std::array< float, arr_size > values[ 6 ];
        static int at = 0;

        int next_at = at + 1; if( next_at >= arr_size ) next_at = 0;
        auto update = [ &next_at, this ] ( decltype( values[ 0 ] )& arr, float& v, const char* title, float lower, float upper ) -> void {
            arr[ at ] =  v;
            ImGui::PlotLines( title, arr.data(), arr.size(), next_at, std::to_string( arr[ at ] ).c_str(), lower, upper, { -60, 80 } );
        };

        ImGui::SeparatorText( "Linear acceleration" );
        update( values[ 0 ], BARCUD.dynamic.gran.acc.x, "X[g]", -2.0f, 2.0f );
        update( values[ 1 ], BARCUD.dynamic.gran.acc.y, "Y[g]", -2.0f, 2.0f );
        update( values[ 2 ], BARCUD.dynamic.gran.acc.z, "Z[g]", -2.0f, 2.0f );
        ImGui::SeparatorText( "Angular velocity" );
        update( values[ 3 ], BARCUD.dynamic.gran.gyr.x, "X[rad/s]", -250.0f, 250.0f );
        update( values[ 4 ], BARCUD.dynamic.gran.gyr.y, "Y[rad/s]", -250.0f, 250.0f );
        update( values[ 5 ], BARCUD.dynamic.gran.gyr.z, "Z[rad/s]", -250.0f, 250.0f );

        ++at; if( at >= arr_size ) at = 0;
     
        ImGui::End();
    }

};

class POTENTIOMETER_BOARD : public _BOARD {
public:
    POTENTIOMETER_BOARD( const std::string& name, barra::potentiometer_t* pm )
    : _BOARD{ name }, _pm{ pm }
    {}

protected:
    barra::potentiometer_t*   _pm;

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_NoResize );

        ImGui::VSliderFloat( "##1", { 100, 300 }, &_pm->lvl, 0.0, 1.0, "%.3f", ImGuiSliderFlags_NoInput );
     
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
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_None );

        static int   op_sel      = 0;
        const char*  ops[]       = { "GET", "SET" };
        const WJPOp_ ops_codes[] = { WJPOp_QGet, WJPOp_QSet };

        static int  str_id_sel = 0; 
        const char* str_ids[]  = { "BITNA_CRT", "GRAN_ACC_RANGE", "GRAN_GYR_RANGE" };

        static char hex_args[ 64 ] = { '\0' }; 

        static std::deque< std::pair< bool, std::string > > prev_cmds;

        ImGui::SeparatorText( "Operation." );
        if( ImGui::BeginCombo( "##1.", ops[ op_sel ], ImGuiComboFlags_None ) ) {
            for( int n = 0; n < IM_ARRAYSIZE( ops ); ++n ) {
                const bool is_sel = ( op_sel == n );
                if( ImGui::Selectable( ops[ n ], is_sel, ImGuiSelectableFlags_None ) )
                    op_sel = n;
            }

            ImGui::EndCombo();
        }

        ImGui::SeparatorText( "String Id." );
        if( ImGui::BeginCombo( "##2", str_ids[ str_id_sel ], ImGuiComboFlags_None ) ) {
            for( int n = 0; n < IM_ARRAYSIZE( str_ids ); ++n ) {
                const bool is_sel = ( str_id_sel == n );
                if( ImGui::Selectable( str_ids[ n ], is_sel, ImGuiSelectableFlags_None ) )
                    str_id_sel = n;
            }

            ImGui::EndCombo();
        }

        ImGui::SeparatorText( "Arguments." );
        ImGui::InputText( "##3", hex_args, IM_ARRAYSIZE( hex_args ), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase );

        ImGui::Separator();
        if( ImGui::Button( "SEND" ) ) {
            char buffer[ 128 ];
            int offset = strlen( strcpy( buffer, str_ids[ str_id_sel ] ) ) + 1;

            WJP_WAIT_BACK_INFO info;
            int ret = BARCUD.wait_back( 
                &info, 
                ops_codes[ op_sel ], 
                { buffer, 
                offset + ixN::Bit::hex_chars2bytes( ( ixN::UBYTE* )buffer + offset, hex_args ) }, 
                { buffer, 
                sizeof( buffer ) }, 
                WJPSendMethod_Direct 
            );

            STATS.add_wb_sent( ret );
            info.resolved.wait( false );
            STATS.add_wb_recv( info.recv_count );
            
            if( !info.ackd() ) {
                prev_cmds.emplace_front( false, info.nakr );
            } else {
                prev_cmds.emplace_front( true, std::format( "{} {} {} - {}", 
                    ops[ op_sel ], 
                    str_ids[ str_id_sel ], 
                    hex_args, 
                    info.recv_count > sizeof( WJP_HEAD ) ? ixN::Bit::bytes2hex_chars( ( ixN::UBYTE* )buffer, info.recv_count - sizeof( WJP_HEAD ), true ) : std::string{ "N/A" } 
                ) );
            }
        }

        ImGui::SeparatorText( "Previous commands." );
        for( auto& cmd : prev_cmds ) {
            ImGui::TextColored( cmd.first ? ImVec4{ 0, 1, 0, 1 } : ImVec4{ 1, 0, 0, 1 }, cmd.first ? "ACK'd:" : "NAK'd:" );
            ImGui::SameLine();
            ImGui::Text( "%s.", cmd.second.c_str() );
        }
     
        ImGui::End();
    }

};

class RENDER_BOARD : public _BOARD {
public:
    RENDER_BOARD( const std::string& name, ixN::Render3* rnd )
    : _BOARD{ name }, 
      _rnd{ rnd },
      lens{ { 0, 0, 20 }, { 0, 0, 0 }, { 0, 1, 0 } },
      mesh{ G_root_dir / "Mesh/", "BarraCUDA", ixN::MESH3_FLAG_MAKE_PIPES },
      view{ "view", lens.view() },
      proj{ "proj", glm::perspective( glm::radians( 72.0f ), _rnd->aspect(), 0.1f, 1000.0f ) },
      lens_pos{ "lens_pos", lens.pos }
    {
        mesh.model.uplink_bv( glm::mat4{ 1 } );
        mesh.pipe->pull( view, proj, lens_pos );
        view.uplink_b();
        proj.uplink_b();
        lens_pos.uplink_b();
    }

protected:
    ixN::Render3*                _rnd   = nullptr;

public:
    ixN::Lens3                   lens;
    ixN::Mesh3                   mesh;

    ixN::Uniform3< glm::mat4 >   view;
    ixN::Uniform3< glm::mat4 >   proj;
    ixN::Uniform3< glm::vec3 >   lens_pos;

public:
    virtual void frame( void ) override {
        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiCond_Once );
        ImGui::Begin( _BOARD::name.c_str(), nullptr, ImGuiWindowFlags_None );

        ImGui::SeparatorText( "Render mode" );

        static int render_mode = 0;
        bool render_mode_changed = false;
        render_mode_changed |= ImGui::RadioButton( "Fill", &render_mode, 0 ); 
        ImGui::SameLine( 0, 30 );
        render_mode_changed |= ImGui::RadioButton( "Wireframe", &render_mode, 1 ); 
        ImGui::SameLine( 0, 30 );
        render_mode_changed |= ImGui::RadioButton( "Pointe", &render_mode, 2 );
        
        if( render_mode_changed ) {
            switch( render_mode ) {
                case 0: _rnd->uplink_fill(); break;
                case 1: _rnd->uplink_wireframe(); break;
                case 2: _rnd->uplink_points(); break;
            }
        }

        ImGui::SeparatorText( "Motion mode" );

        static int motion_mode = 0;
        render_mode_changed |= ImGui::RadioButton( "Feedback", &motion_mode, 0 ); 
        ImGui::SameLine( 0, 30 );
        render_mode_changed |= ImGui::RadioButton( "Auto", &motion_mode, 1 ); 

        _rnd->clear( { 0.0, 0.0, 0.0, 1.0 } );
        
        if( motion_mode == 1 ) {
            mesh.model.uplink_bv( glm::rotate( glm::mat4{ 1.0 }, glm::radians( 18.0f * ( float )ImGui::GetTime() ), glm::vec3{ 1, 1, 0 } ) );
        } else {
            glm::mat4 mr{ 1.0 };
            mr = glm::rotate( mr, glm::radians( BARCUD.dynamic.gran.gyr.x ) / 12.0f, glm::vec3{ 1, 0, 0 } );
            mr = glm::rotate( mr, glm::radians( BARCUD.dynamic.gran.gyr.y ) / 12.0f, glm::vec3{ 0, 1, 0 } );
            mr = glm::rotate( mr, glm::radians( BARCUD.dynamic.gran.gyr.z ) / 12.0f, glm::vec3{ 0, 0, 1 } );
            mesh.model.uplink_bv( mr * glm::translate( 
                glm::mat4{ 1.0 }, { BARCUD.dynamic.gran.acc.x * 3.0, BARCUD.dynamic.gran.acc.y * 3.0, BARCUD.dynamic.gran.acc.z * 3.0 }
            ) );
        }

        mesh.splash();
     
        ImGui::End();
    }

};



int main( int argc, char** argv ) {
    G_root_dir = std::filesystem::path{ ( argc >= 2 ) ?  argv[ 1 ] : "." };

/* Setup */
    glfwSetErrorCallback( [] ( int err, const char* desc ) -> void {
        ixN::comms( ixN::EchoLevel_Error ) << "GLFW error code ( " << err << " ): " << desc << ".";
    } );

    if( int ret = ixN::begin_runtime( argc, argv, ixN::BEGIN_RUNTIME_FLAG_INIT_NETWORK, nullptr, nullptr ); ret != 0 ) return ret;
   
/* GLFW window */
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    GLFWwindow* window = glfwCreateWindow( ixN::Env::w(.86), ixN::Env::h(.86) , "BarraCUDA-CTRL Tester V2", nullptr, nullptr);
    glfwSetWindowPos( window, 50, 50 );
    IXN_ASSERT( window != nullptr, -1 );
    glfwMakeContextCurrent( window );
    ixN::Render3 render( window );

    G_is_running = [ &render ] () -> bool { return !glfwWindowShouldClose( render.handle() ); };

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
        "Stats"
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
    DASHBOARD.emplace_back( new POTENTIOMETER_BOARD{
        "Kazuha - Potentiometer", &BARCUD.dynamic.kazuha
    } );
    DASHBOARD.emplace_back( new ACCEL_GYRO_BOARD{
        "Gran - Accel & Gyro", &BARCUD.dynamic.gran
    } );
    DASHBOARD.emplace_back( new COMMAND_BOARD{
        "Command"
    } );
    DASHBOARD.emplace_back( new RENDER_BOARD{
        "Render", &render
    } );

    IXN_ASSERT( BARCUD.bind().open( 0 ) == 0, -1 );
    
    std::thread burst_th{ [] () -> void {
    l_attempt_connect: {
        if( !G_is_running() )
            goto l_burst_th_end;
        ixN::comms( ixN::EchoLevel_Pending ) << "Attempting connection to the controller...";
        if( BARCUD.connect( ixN::Dev::BARRACUDA_CTRL_FLAG_TRUST_INVOKER ) != 0 ) {
            ixN::comms() << "Could not connect to the controller. Retrying in 3s...\n";
            BARCUD.force_waiting_resolvers();
            std::this_thread::sleep_for( std::chrono::seconds{ 3 } );
            goto l_attempt_connect;
        }
        STATS.reset_session();
        ixN::comms() << "Connected to the controller.\n";

        WJP_RESOLVE_RECV_INFO info;
        while( G_is_running() ) {
            int ret = BARCUD.trust_resolve_recv( &info );
            if( ret <= 0 ) break;
            if( info.recv_head._dw0.op == WJPOp_IBurst ) {
                STATS.add_brst_recv( ret );
            } else {
                STATS.add_wb_recv( ret );
                STATS.add_wb_sent( info.sent_count );
            }
        }

        if( BARCUD.connected() ) BARCUD.disconnect( 0 );
        BARCUD.force_waiting_resolvers();
        if( G_is_running() )
            goto l_attempt_connect;
    } 
    l_burst_th_end:
        return;
    } };
    

    while( G_is_running() ) {
        glfwPollEvents();

        if( glfwGetWindowAttrib( render.handle(), GLFW_ICONIFIED ) != 0 ) {
            ImGui_ImplGlfw_Sleep( 10 );
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DASHBOARD.frame();

        //ImGui::ShowDemoWindow();

        ImGui::Render();

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
    
    return ixN::end_runtime( argc, argv, 0, nullptr, nullptr );
}
