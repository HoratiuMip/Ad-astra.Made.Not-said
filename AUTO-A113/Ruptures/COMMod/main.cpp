#include <a113/clkwrk/immersive.hpp>
#include <a113/osp/osp.hpp>

class Component {
public:
    using id_t = std::string;

public:
    virtual a113::status_t ui_frame( double dt_, void* arg_ ) = 0;
};

class Launcher {
public:
    struct _common_data_t {
        a113::io::COM_Ports   com_ports{ a113::DispenserMode_Swap, true, true };
    } common;

protected:
    std::map< Component::id_t, a113::HVec< Component > >   _components_map;
    std::recursive_mutex                                   _components_mtx;

public:
    void register_component( const Component::id_t& id_, a113::HVec< Component > comp_ ) {
        std::unique_lock lock{ _components_mtx };
        _components_map[ id_ ] = std::move( comp_ );
    }

    void unregister_component( const Component::id_t& id_ ) {
        std::unique_lock lock{ _components_mtx };
        _components_map.erase( id_ );
    }

public:
    a113::status_t ui_frame( double dt_, void* arg_ );

};
inline Launcher G_Launcher;


#include <ModbusClientPort.h>
class ModbusRTU : public Component {
protected:
    struct _ui_data_t {
        int   selected_com_port   = -0x1;
        struct _parity_t {
            const char* const   types[ 5 ]    = { "No parity", "Even parity", "Odd parity", "Space parity", "Mark parity" };
            int                 selected      = 0x0;
        }    parity;
        struct _stopbit_t {
            const char* const   types[ 3 ]    = { "One", "One & 1/2", "Two" };
            int                 selected      = 0x0;
        }    stopbit;
        struct _flow_t {
            const char* const   types[ 3 ]    = { "None", "Hardware", "Software" };
            int                 selected      = 0x0;
        }    flow;

    }   _ui;

    struct _mb_data_t {
        std::shared_ptr< ModbusClientPort >   port       = nullptr;
        Modbus::SerialSettings                settings   = {
            .baudRate         = 115200,
            .dataBits         = 8,
            .parity           = Modbus::NoParity,
            .stopBits         = Modbus::OneStop,
            .flowControl      = Modbus::NoFlowControl,
            .timeoutFirstByte = 3000,
            .timeoutInterByte = 1000
        };
    }   _mb;

    struct _task_t {
        _task_t( std::shared_ptr< ModbusClientPort > port_ ) 
        : _ctl{ new std::atomic_int{ 1'000 } }
        {
            std::thread( &_task_t::main, this, std::move( port_ ), _ctl ).detach();
        }

        ~_task_t() {
            _ctl->store( -0x1, std::memory_order_release );
        }

        std::atomic_int*   _ctl   = nullptr;

        void main( std::shared_ptr< ModbusClientPort > port_, std::atomic_int* ctl_ ) { 
            for(; ctl_->load( std::memory_order_relaxed ) != -0x1;) {
                spdlog::warn( "gello" );
            } 
            delete ctl_;
        }

        a113::status_t ui_frame( double dt_, void* arg_ ) {
            ImGui::Text( "gello" );
            return 0x0;
        }

        std::string status_str( void ) {
            return "STATUS";
        }
    };
    struct _task_data_t {
        std::list< _task_t >   list;
    }   _tsk;

public:
    a113::status_t ui_frame( double dt_, void* arg_ ) override {
        a113::io::COM_port_t* port = nullptr;

        ImGui::SeparatorText( "Found COM ports" );
    {
        a113::dispenser_acquire ports{ G_Launcher.common.com_ports };
      
        for( int idx = 0x0; idx < ports->size(); ++idx ) {
            const bool selected = idx == _ui.selected_com_port;

            ImGui::Separator();
            if( ImGui::Selectable( (*ports)[ idx ].friendly.c_str(), selected, selected ? ImGuiSelectableFlags_Highlight : ImGuiSelectableFlags_None ) ) {
                _ui.selected_com_port = idx;
            }
            ImGui::Separator();
        }

        if( _ui.selected_com_port >= 0x0 && _ui.selected_com_port < ports->size() ) port = &( *ports )[ _ui.selected_com_port ];
    }
        ImGui::SeparatorText( "Connection settings" );
        ImGui::BeginDisabled( not port );
        ImGui::LabelText( "COM port", port ? port->id.c_str() : "N/A" ); ImGui::SetItemTooltip( "Use the above panel to select a COM port." );
        ImGui::InputInt( "Baud rate", &_mb.settings.baudRate, 0, 0 );
        ImGui::InputScalar( "Data bits", ImGuiDataType_S8, &_mb.settings.dataBits, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );
        if( ImGui::BeginCombo( "Parity", _ui.parity.types[ _ui.parity.selected ] ) ) {
            for( int idx = 0x0; idx < IM_ARRAYSIZE( _ui.parity.types ); ++idx ) {
                if (ImGui::Selectable( _ui.parity.types[ idx ], idx == _ui.parity.selected ) ) _ui.parity.selected = idx;
            }
            ImGui::EndCombo();
        }
        if( ImGui::BeginCombo( "Stop bit", _ui.stopbit.types[ _ui.stopbit.selected ] ) ) {
            for( int idx = 0x0; idx < IM_ARRAYSIZE( _ui.stopbit.types ); ++idx ) {
                if (ImGui::Selectable( _ui.stopbit.types[ idx ], idx == _ui.stopbit.selected ) ) _ui.stopbit.selected = idx;
            }
            ImGui::EndCombo();
        }
        if( ImGui::BeginCombo( "Flow control", _ui.flow.types[ _ui.flow.selected ] ) ) {
            for( int idx = 0x0; idx < IM_ARRAYSIZE( _ui.flow.types ); ++idx ) {
                if (ImGui::Selectable( _ui.flow.types[ idx ], idx == _ui.flow.selected ) ) _ui.flow.selected = idx;
            }
            ImGui::EndCombo();
        }
        ImGui::InputScalar( "First TO", ImGuiDataType_U32, &_mb.settings.timeoutFirstByte, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );
        ImGui::SetItemTooltip( "Maximum time to receive the first response byte, in milliseconds." );
        ImGui::InputScalar( "Inter TO", ImGuiDataType_U32, &_mb.settings.timeoutInterByte, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );
        ImGui::SetItemTooltip( "Maximum time to receive the next response byte, in milliseconds." );
        ImGui::EndDisabled();

        ImGui::Separator();
        if( _mb.port ) {
            ImGui::TextColored( ImVec4{ 0,1,0,1 }, "Connected on %s", _mb.settings.portName );
        } else {
            ImGui::TextColored( ImVec4{ 1,0,0,1 }, "Disconnected" );
        }
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( not port );
        if( ImGui::Button( "Connect" ) ) {
            _mb.settings.portName    = port->id.c_str();
            _mb.settings.parity      = ( Modbus::Parity )_ui.parity.selected;
            _mb.settings.stopBits    = ( Modbus::StopBits )_ui.stopbit.selected;
            _mb.settings.flowControl = ( Modbus::FlowControl )_ui.flow.selected;

            _mb.port.reset( Modbus::createClientPort( Modbus::RTU, &_mb.settings, true ) );
            A113_ASSERT_OR( _mb.port ) spdlog::error( "Failed to create modbus client port." );
        }
        ImGui::SetItemTooltip( "Attempt a connection using the above configured settings." );
        ImGui::EndDisabled();
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( not _mb.port );
        if( ImGui::Button( "Disconnect" ) ) {
            _mb.settings.portName = "";
            _mb.port.reset();
        }
        ImGui::SetItemTooltip( "End the current connection." );
        ImGui::EndDisabled();
        ImGui::Separator();

        if( ImGui::Button( "+" ) ) {
            _tsk.list.emplace_back( _mb.port );
        }
        ImGui::SetItemTooltip( "Add new rule." );
        ImGui::SameLine();
        ImGui::SeparatorText( "Rules" );
        ImGui::Separator();

        ImGui::BeginChild( "Rules" );
        int ids_1 = 0;
        for( auto itr = _tsk.list.begin(); itr != _tsk.list.end(); ++ids_1 ) {
            bool remove = false;
            
            ImGui::PushID( ids_1 ); 
            ImGui::PushStyleColor( ImGuiCol_Button,        ImVec4{ 0,0,0,0 } );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0,0,0,0 } );
            ImGui::PushStyleColor( ImGuiCol_ButtonActive,  ImVec4{ 0,0,0,0 } );
            ImGui::PushStyleColor( ImGuiCol_Text,          ImVec4{ 1,0,0,1 } );
            if( ImGui::SmallButton( "X" ) ) remove = true;
            ImGui::PopStyleColor( 4 );
            ImGui::PopID();

            ImGui::SameLine(); ImGui::SeparatorText( itr->status_str().c_str() );
            A113_ASSERT_OR( itr->ui_frame( dt_, arg_ ) == 0x0 ) remove = true;
            ImGui::Separator();

            if( not remove ) goto l_keep;
        l_remove:
            itr = _tsk.list.erase( itr );
            continue;
        l_keep:
            ++itr;
        }
        ImGui::EndChild();

        return 0x0;
    }

};


a113::status_t Launcher::ui_frame( double dt_, void* arg_ ) {
    ImGui::Begin( "COMMod", nullptr, ImGuiWindowFlags_None );
    ImGui::BeginChild( "Launcher", ImVec2{ 200, 0 }, ImGuiChildFlags_Border );

    static struct _component_t {
        const char* const                                  name;
        std::function< a113::HVec< Component >( void ) >   builder; 
        int                                                count      = 0x0;
    } components[] = {
        { .name = "ModbusRTU", .builder = [] ( void ) -> a113::HVec< Component > { return a113::HVec< ModbusRTU >::make(); } }
    };

    ImGui::SeparatorText( "New panel" );
    for( auto& comp : components ) {
        ImGui::Separator();
        ImGui::BulletText( comp.name ); ImGui::SameLine();
        if( ImGui::ArrowButton( comp.name, ImGuiDir_Right ) ) {
            this->register_component( std::format( "{}-{}", comp.name, ++comp.count ), comp.builder() );
        }
        ImGui::SetItemTooltip( std::format( "Launch a new {} tab.", comp.name ).c_str() );
        ImGui::Separator();
    }

    ImGui::EndChild(); ImGui::SameLine(); 
    ImGui::BeginChild( "Panels", ImVec2{ 0, 0 }, ImGuiChildFlags_Border );
    
    ImGui::BeginTabBar( "Panels" );

    std::unique_lock lock{ _components_mtx };
    for( auto itr = _components_map.begin(); itr != _components_map.end(); ) {
        auto& comp = *itr->second;

        bool open = true;
        if( ImGui::BeginTabItem( itr->first.c_str(), &open ) ) {
            A113_ASSERT_OR( comp.ui_frame( dt_, arg_ ) == 0x0 ) open = false;
            ImGui::EndTabItem();
        }

        if( open ) goto l_keep;

    l_remove:
        itr = _components_map.erase( itr );
        continue;

    l_keep:
        ++itr;
    }

    ImGui::EndTabBar();
    ImGui::EndChild();
    ImGui::End();
    return 0x0;
}


int main( int argc, char* argv[] ) {
    a113::init( argc, argv, a113::init_args_t{
        .flags = a113::InitFlags_None
    } );

    a113::clkwrk::Immersive imm; imm.main( argc, argv, a113::clkwrk::Immersive::config_t{
        .arg        = nullptr,
        .title      = "COMMod",
        .srf_bgn_as = a113::clkwrk::Immersive::SrfBeginAs_Iconify,
        .loop       = [] ( double dt_, void* arg_ ) static -> a113::status_t { return G_Launcher.ui_frame( dt_, arg_ ); }
    } );
}