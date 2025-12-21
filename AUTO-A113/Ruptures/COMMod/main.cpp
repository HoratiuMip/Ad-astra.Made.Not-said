#include <a113/clkwrk/immersive.hpp>
#include <a113/osp/IO_utils.hpp>

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
    } common_data;

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
    }   _ui_data;

    struct _mb_data_t {
        std::unique_ptr< ModbusClientPort >   port       = nullptr;
        Modbus::SerialSettings                settings   = {
            .dataBits         = 8,
            .parity           = Modbus::NoParity,
            .stopBits         = Modbus::OneStop,
            .timeoutFirstByte = 3000,
            .timeoutInterByte = 1000
        };
    }   _mb_data;

public:
    a113::status_t ui_frame( double dt_, void* arg_ ) override {
        ImGui::SeparatorText( "Found COM ports" );

        a113::dispenser_acquire ports{ G_Launcher.common_data.com_ports };
      
        for( int idx = 0x0; idx < ports->size(); ++idx ) {
            const bool selected = idx == _ui_data.selected_com_port;

            ImGui::Separator();
            if( ImGui::Selectable( (*ports)[ idx ].friendly.c_str(), selected, selected ? ImGuiSelectableFlags_Highlight : ImGuiSelectableFlags_None ) ) {
                _ui_data.selected_com_port = idx;
            }
            ImGui::Separator();
        }

        a113::io::COM_port_t* port = nullptr;
        if( _ui_data.selected_com_port >= 0x0 && _ui_data.selected_com_port < ports->size() ) port = &( *ports )[ _ui_data.selected_com_port ];

        ImGui::SeparatorText( "Connection settings" );
        ImGui::LabelText( "COM port", port ? port->id.c_str() : "?" );
        ImGui::InputInt( "Baud rate", &_mb_data.settings.baudRate, 0, 0 );

        ImGui::Separator();
        if( _mb_data.port ) {
            ImGui::TextColored( ImVec4{ 0, 1, 0, 1 }, "Connected on %s", _mb_data.settings.portName );
        } else {
            ImGui::TextColored( ImVec4{ 1, 0, 0, 1 }, "Disconnected" );
        }
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( not port );
        if( ImGui::Button( "Connect" ) ) {
            _mb_data.settings.portName = port->id.c_str();
            _mb_data.port.reset( Modbus::createClientPort( Modbus::RTU, &_mb_data.settings, true ) );
            A113_ASSERT_OR( _mb_data.port ) spdlog::error( "Failed to create modbus client port." );
        }
        ImGui::EndDisabled();
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( not _mb_data.port );
        if( ImGui::Button( "Disconnect" ) ) {
            _mb_data.settings.portName = "";
            _mb_data.port.reset();
        }
        ImGui::EndDisabled();
        ImGui::Separator();

         
    // if (!port) {
    //     std::cerr << "Failed to create RTU client port\n";
    //     return 1;
    // }

    // const uint8_t unitId   = 3;     // slave ID
    // const uint16_t offset  = 0;     // starting register offset
    // const uint16_t count   = 2;    // number of registers to read
    // std::vector<uint16_t> regs(count);

    // Modbus::StatusCode status = port->readInputRegisters(unitId, offset, count, regs.data());
    // if (Modbus::StatusIsGood(status)) {
    //     std::cout << "Read registers:\n";
    //     for (int i = 0; i < count; i++) {
    //         std::cout << "  Reg[" << (offset + i) << "] = " << regs[i] << "\n";
    //     }
    // } else {
    //     std::cerr << "Modbus error: " << port->lastErrorText() << "\n";
    // }

        return 0x0;
    }

};


a113::status_t Launcher::ui_frame( double dt_, void* arg_ ) {
    ImGui::Begin( "COMMod", nullptr, ImGuiWindowFlags_None );
    ImGui::BeginChild( "Launcher", ImVec2{ 300, 0 }, ImGuiChildFlags_Border );

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