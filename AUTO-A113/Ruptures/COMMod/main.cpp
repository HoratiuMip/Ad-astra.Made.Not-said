#include <a113/clkwrk/immersive.hpp>
#include <a113/osp/osp.hpp>

#include <a113/clkwrk/imm_widgets.hpp>

class Component {
public:
    using id_t = std::string;

public:
    virtual a113::status_t ui_frame( double dt_, void* arg_ ) = 0;
};

class Launcher {
public:
    struct _common_data_t {
        a113::io::COM_Ports   com_ports{ a113::DispenserMode_Lock, true, true };
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
    #define _SCRAMBLE( a, b ) (((a)<<2)|(b))

protected:
    enum _Mode_ { _Mode_Once, _Mode_Repeat };
    enum _Op_ { _Op_Read, _Op_Write };
    enum _Type_ { _Type_DiscreteInput, _Type_Coil, _Type_InputRegister, _Type_HoldingRegister };

protected:
    inline static const char* const _STATUS_STRS[] = {
        "OK", "?",

        "BAD ILLEGAL FUNCTION",
        "BAD ILLEGAL DATA ADDRESS",
        "BAD ILLEGAL DATA VALUE",
        "BAD SERVER DEVICE FAILURE",
        "BAD ACKNOWLEDGE",
        "BAD SERVER DEVICE BUSY",
        "BAD NEGATIVE ACKNOWLEDGE",
        "BAD MEMORY PARITY ERROR",
        "BAD GATEWAY PATH UNAVAILABLE",
        "BAD GATEWAY TARGET DEVICE FAILED TO RESPOND",

        "BAD EMPTY RESPONSE",
        "BAD NOT CORRECT REQUEST",
        "BAD NOT CORRECT RESPONSE",
        "BAD WRITE BUFFER OVERFLOW",
        "BAD READ BUFFER OVERFLOW",
        "BAD PORT CLOSED",

        "BAD SERIAL OPEN",
        "BAD SERIAL WRITE",
        "BAD SERIAL READ",
        "BAD SERIAL READ TIMEOUT",
        "BAD SERIAL WRITE TIMEOUT",

        "BAD CRC"
    };

protected:
    struct _ui_data_t {
        a113::clkwrk::imm_widgets::COM_Ports   ports{ G_Launcher.common.com_ports };

        struct _parity_t : public a113::clkwrk::imm_widgets::DropDownList {
            _parity_t() : DropDownList{ ( const char* const[] ){ "No parity", "Even parity", "Odd parity", "Space parity", "Mark parity" }, 5, 0x0 } {}
        } parity;
        struct _stopbit_t : public a113::clkwrk::imm_widgets::DropDownList {
            _stopbit_t() : DropDownList{ ( const char* const[] ){ "One", "One & 1/2", "Two" }, 3, 0x0 } {}
        } stopbit;
        struct _flow_t : public a113::clkwrk::imm_widgets::DropDownList {
            _flow_t() : DropDownList{ ( const char* const[] ){ "None", "Hardware", "Software" }, 3, 0x0 } {}
        } flow;

    } _ui;

    struct _mb_data_t {
        std::shared_ptr< ModbusClientPort >   port       = nullptr;
        std::shared_ptr< std::mutex >         port_mtx   = std::make_shared< std::mutex >();
        Modbus::SerialSettings                settings   = {
            .baudRate         = 115200,
            .dataBits         = 8,
            .parity           = Modbus::NoParity,
            .stopBits         = Modbus::OneStop,
            .flowControl      = Modbus::NoFlowControl,
            .timeoutFirstByte = 100,
            .timeoutInterByte = 10
        };
    } _mb;

    struct _task_t {
        _task_t( std::shared_ptr< ModbusClientPort > port_, std::shared_ptr< std::mutex > port_mtx_ ) 
        : _bridge{ new _bridge_t{
            .ctl      = { 0x0 },
            .scramble = { 0x0 },
            .status   = { 0x0 },
            .data     = { a113::DispenserMode_Swap }
        } }
        {
            std::thread( &_task_t::main, this, std::move( port_ ), std::move( port_mtx_ ), _bridge ).detach();
        }

        ~_task_t() {
            _bridge->ctl.store( -0x1, std::memory_order_release );
            _bridge->ctl.notify_one();
        }

        struct _bridge_t {
            std::atomic_int                                   ctl;
            std::atomic_int                                   scramble;
            struct _meta_t {
                std::atomic_uint8_t    unit;
                std::atomic_uint16_t   address;
                std::atomic_uint16_t   count;
            }                                                 meta;
            std::atomic_int                                   status;
            std::atomic_int                                   count;
            a113::Dispenser< std::array< uint16_t, 2048 > >   data;
        } *_bridge;

        struct _config_t {
            struct _mode_t : public a113::clkwrk::imm_widgets::DropDownList {
                _mode_t() : DropDownList{ ( const char* const[] ){ "once", "repeat" }, 2, 0x0 } {}
            }   mode;
            struct _op_t : public a113::clkwrk::imm_widgets::DropDownList {
                _op_t() : DropDownList{ ( const char* const[] ){ "read", "write" }, 2, 0x0 } {}
            }   op;
            struct _type_t : public std::array< a113::clkwrk::imm_widgets::DropDownList, 2 > {
                _type_t() : std::array< a113::clkwrk::imm_widgets::DropDownList, 2 >{ {
                    { ( const char* const[] ){ "discrete inputs", "coils", "input registers", "holding registers" }, 4 },
                    { ( const char* const[] ){ "coils", "holding registers" }, 2 }
                } } {}
            }   type;
            struct _meta_t {
                uint8_t   unit       = 0x0;
                int       address    = 0x0;
                int       count      = 0;
                int       interval   = 1'000;
            }   meta;
        } _config;

        struct _ui_t {
            double   elasped   = 0.0;
        } _ui;

        void main( std::shared_ptr< ModbusClientPort > port_, std::shared_ptr< std::mutex > port_mtx_, _bridge_t* bridge_ ) { 
            for( int ctl = bridge_->ctl; ctl != -0x1; ctl = bridge_->ctl ) {
                if( ctl == 0x0 ) {
                    bridge_->ctl.wait( 0x0 );
                    ctl = bridge_->ctl;
                    if( ctl == -0x1 ) break;
                    if( ctl == -0x2 ) { bridge_->ctl = 0x0; ctl = 0x0; }
                }

                int                 status = 0x1;
                _bridge_t::_meta_t& meta   = bridge_->meta;
            
                switch( bridge_->scramble ) {
                    case _SCRAMBLE( _Op_Read, _Type_InputRegister ): {
                        std::lock_guard lock{ *port_mtx_ };
                        a113::dispenser_control data{ bridge_->data };
                        status = port_->readInputRegisters( meta.unit.load(), meta.address.load(), meta.count.load(), data->data() );
                    break; }
                }

                switch( status ) {
                    case Modbus::Status_Uncertain: bridge_->status = 0x1; break;
                    case Modbus::Status_Good: bridge_->status = 0x0; break;
                    default: {
                        if( not ( status & Modbus::Status_Bad ) ) { status = 0x1; break; }
                        status &= ~Modbus::Status_Bad;

                        if( status & 0x100 ) {
                            status = 0x0B + status & ~0x100;
                        } else if( status & 0x200 ) {
                            status = 0x11 + status & ~0x200;
                        } else if( status & 0x400 ) {
                            status = 0x16 + status & ~0x400;
                        } else {
                            status = 0x01 + status;
                        }
                    break; }
                }
                if( status >= std::size( _STATUS_STRS ) && status < 0x0 ) status = 0x1;
                bridge_->status = status;
                ++bridge_->count;

                std::this_thread::sleep_for( std::chrono::milliseconds{ ctl } );
            } 
            delete bridge_;
        }

        a113::status_t ui_frame( double dt_, void* arg_ ) {
            ImGui::Text( "On unit" );
            ImGui::SameLine(); ImGui::SetNextItemWidth( 50 ); ImGui::InputScalar( "##Unit", ImGuiDataType_U8, &_config.meta.unit, 0, 0, nullptr, ImGuiInputTextFlags_CharsDecimal );
            ImGui::SameLine(); ImGui::Text( "," ); 
            ImGui::SameLine(); ImGui::SetNextItemWidth( 100 ); _config.mode.imm_frame( "##Mode" );
            switch( _config.mode.selected ) {
                case _Mode_Once: {

                break; }
                case _Mode_Repeat: {
                    ImGui::SameLine(); ImGui::Text( ", every" );
                    ImGui::SameLine(); ImGui::SetNextItemWidth( 50 ); ImGui::InputInt( "##Interval", &_config.meta.interval, 0, 0, ImGuiInputTextFlags_CharsDecimal ); if( _config.meta.interval <= 100 ) _config.meta.interval = 100;
                    ImGui::SameLine(); ImGui::Text( "ms" );
                break; }
            }
            ImGui::SameLine(); ImGui::Text( "," );
            ImGui::SameLine(); ImGui::SetNextItemWidth( 100 ); _config.op.imm_frame( "##Op" );
            ImGui::SameLine(); ImGui::SetNextItemWidth( 50 ); ImGui::InputInt( "##Count", &_config.meta.count, 0, 0, ImGuiInputTextFlags_CharsDecimal ); if( _config.meta.count <= 0 ) _config.meta.count = 0;
            ImGui::SameLine(); ImGui::SetNextItemWidth( 200 ); _config.type[ _config.op.selected ].imm_frame( "##Type" );
            ImGui::SameLine(); ImGui::Text( "from address" );
            ImGui::SameLine(); ImGui::SetNextItemWidth( 50 ); ImGui::InputInt( "##Address", &_config.meta.address, 0, 0, ImGuiInputTextFlags_CharsDecimal ); if( _config.meta.address <= 0x0 ) _config.meta.address = 0x0;
            ImGui::SameLine(); ImGui::Text( "." );

            _bridge->scramble     =_SCRAMBLE( _config.op.selected, _config.type[ _config.op.selected ].selected );
            _bridge->meta.unit    =_config.meta.unit;
            _bridge->meta.address =_config.meta.address;
            _bridge->meta.count   =_config.meta.count;

            switch( _bridge->scramble ) {
                case _SCRAMBLE( _Op_Read, _Type_InputRegister ): {
                    if( ImGui::BeginTable( "##Data", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders ) ) {
                        ImGui::TableSetupColumn( "Address", ImGuiTableColumnFlags_None, 0.0, 0 );
                        ImGui::TableSetupColumn( "Value", ImGuiTableColumnFlags_None, 0.0, 1 );  

                        ImGui::TableHeadersRow();

                        _ui.elasped += dt_;

                        a113::dispenser_watch data{ _bridge->data };
                        for( int idx = 0x0; idx < _config.meta.count; ++idx ) {
                            ImGui::TableNextRow( ImGuiTableRowFlags_None );

                            int gs = ( sin( 2.0*_ui.elasped + 3.1415 / 4.0 * idx ) + 1.0 ) / 2.0 * 56.0;
                            ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, IM_COL32( gs, gs, gs, 255 ) );  

                            ImGui::TableSetColumnIndex( 0 );
                            ImGui::Text( std::format( "0x{:X}", idx ).c_str() );

                            ImGui::TableSetColumnIndex( 1 );
                            ImGui::Text( std::format( "{:016b}", data->data()[ idx ] ).c_str() );
                        }

                        ImGui::EndTable();
                    }
                break; }
            }

            return 0x0;
        }

        std::pair< bool, const char* > status_str( void ) {
            return { _bridge->status == 0x0, _STATUS_STRS[ _bridge->status ] };
        }

        int completed( void ) {
            return _bridge->count;
        }

        bool is_active( void ) {
            return _bridge->ctl > 0x0;
        }

        void set_ctl( int ctl_ ) {
            _bridge->ctl = ctl_;
            _bridge->ctl.notify_one();
        }
    };
    struct _task_data_t {
        std::list< _task_t >   list;
    }   _tsk;

public:
    a113::status_t ui_frame( double dt_, void* arg_ ) override {
        ImGui::SeparatorText( "Found COM ports" );
        auto ports_guard = _ui.ports.imm_frame();
        auto port        = _ui.ports.port;
    
        ImGui::SeparatorText( "Connection settings" );
        ImGui::BeginDisabled( not port || _mb.port );
        ImGui::LabelText( "COM port", port ? port->id.c_str() : "N/A" ); ImGui::SetItemTooltip( "Use the above panel to select a COM port." );
        ImGui::InputInt( "Baud rate", &_mb.settings.baudRate, 0, 0 );
        ImGui::InputScalar( "Data bits", ImGuiDataType_S8, &_mb.settings.dataBits, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );
        _ui.parity.imm_frame( "Parity" );
        _ui.stopbit.imm_frame( "Stop bit" );
        _ui.flow.imm_frame( "Flow control" );
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
        ImGui::BeginDisabled( not port || _mb.port );
        if( ImGui::Button( "Connect" ) ) {
            _mb.settings.portName    = port->id.c_str();
            _mb.settings.parity      = ( Modbus::Parity )_ui.parity.selected;
            _mb.settings.stopBits    = ( Modbus::StopBits )_ui.stopbit.selected;
            _mb.settings.flowControl = ( Modbus::FlowControl )_ui.flow.selected;

            _mb.port.reset( Modbus::createClientPort( Modbus::RTU, &_mb.settings, true ), [] ( ModbusClientPort* port_ ) static -> void {
                port_->close();
                delete port_;
            } );
            A113_ASSERT_OR( _mb.port ) spdlog::error( "Failed to create modbus client port." );
        }
    
        ports_guard.release();

        ImGui::SetItemTooltip( "Attempt a connection using the above configured settings." );
        ImGui::EndDisabled();
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( not _mb.port );
        if( ImGui::Button( "Disconnect" ) ) {
            _tsk.list.clear();
            _mb.settings.portName = "";
            _mb.port.reset();
        }
        ImGui::SetItemTooltip( "End the current connection." );
        ImGui::EndDisabled();
        ImGui::Separator();

        ImGui::BeginDisabled( not _mb.port );
        if( ImGui::Button( "+" ) ) {
            _tsk.list.emplace_back( _mb.port, _mb.port_mtx );
        }
        ImGui::SetItemTooltip( _mb.port ? "Add new rule." : "Connect before adding rules." );
        ImGui::EndDisabled();
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

            ImGui::SameLine(); ImGui::Text( "%d", itr->completed() );
            ImGui::SameLine();
            if( itr->_config.mode.selected == 0x1 ) {
                const bool active = itr->is_active();
                if( ImGui::RadioButton( "##Toggle", active ) ) {
                    if( active ) itr->set_ctl( 0x0 ); else { itr->set_ctl( itr->_config.meta.interval ); }
                }
            } else {
                if( ImGui::ArrowButton( "##Exec", ImGuiDir_Right ) ) {
                    itr->set_ctl( -0x2 );
                }
            }

            auto [ status_ok, status_str ] = itr->status_str();
            ImGui::PushStyleColor( ImGuiCol_Text, status_ok ? ImVec4{ 0,1,0,1 } : ImVec4{ 1,0,0,1 } );
            ImGui::SameLine(); ImGui::SeparatorText( status_str );
            ImGui::PopStyleColor( 1 );
            A113_ASSERT_OR( itr->ui_frame( dt_, arg_ ) == 0x0 ) remove = true;
            ImGui::Separator();
            ImGui::PopID();

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

protected:
    #undef _SCRAMBLE

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