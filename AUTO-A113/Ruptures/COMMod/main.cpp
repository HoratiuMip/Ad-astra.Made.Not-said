/* 
| RELEASE NOTES:
|   v1.1.0 - Added the serial monitor pannel.
|   v1.0.0 - Initial release, packed with the ModbusRTU over serial port panel.
*/

#include <a113/clkwrk/immersive.hpp>
#include <a113/osp/osp.hpp>

#include <a113/clkwrk/imm_widgets.hpp>

#define COMMOD_VERSION_STR "v1.1.0"

class Component  {
public:
    friend class Launcher;

public:
    using id_t = std::string;

public:
    virtual a113::status_t ui_frame( double dt_, void* arg_ ) = 0;
};

class Launcher {
public:
    struct _common_data_t {
        a113::io::COM_Ports   com_ports{ a113::DispenserMode_Lock, a113::io::COM_Ports_init_args_t{
            .refresh = true,
            .listen  = true
        } };
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
    #define _MAKE_RULE_DEF( m, f1, f2 ) (((m)<<3)|((f1)<<2)|(f2))
    #define _MAKE_RULE_FUNC( f1, f2 )   (((f1)<<2)|(f2))

protected:
    enum _RuleMode_  { _RuleMode_Once, _RuleMode_Repeat };
    enum _RuleFunc1_ { _RuleFunc1_Read, _RuleFunc1_Write };
    enum _RuleFunc2_ { _RuleFunc2_DiscreteInputs, _RuleFunc2_Coils, _RuleFunc2_InputRegisters, _RuleFunc2_HoldingRegisters };

    inline static constexpr int   _RULE_MODE_MSK    = 0b1000;
    inline static constexpr int   _RULE_FUNC1_MSK   = 0b0100;
    inline static constexpr int   _RULE_FUNC2_MSK   = 0b0011;
    inline static constexpr int   _RULE_FUNC_MSK    = _RULE_FUNC1_MSK | _RULE_FUNC2_MSK;

protected:
    inline static const char* const _STATUS_STRS[] = {
        "OK", "???",

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
            .timeoutFirstByte = 1000,
            .timeoutInterByte = 10
        };
    } _mb;

    struct _rule_t {
        _rule_t( std::shared_ptr< ModbusClientPort > port_, std::shared_ptr< std::mutex > port_mtx_ ) 
        : _bridge{ new _bridge_t{} }
        {
            std::thread( &_rule_t::main, this, std::move( port_ ), std::move( port_mtx_ ), _bridge ).detach();
        }

        ~_rule_t() {
            _bridge->control.store( -0x1, std::memory_order_release );
            _bridge->control.notify_one();
        }

        struct _bridge_t {
            std::atomic_int                                 control     = { 0x0 };

            std::atomic_int                                 rule_def    = { 0x0 };
            struct _config_t {
                std::atomic_uint8_t    unit;
                std::atomic_uint16_t   address;
                std::atomic_uint16_t   count;
                std::atomic_int        interval;
            }                                               config;

            std::atomic_int                                 status      = { 0x1 };
            std::atomic_int                                 completed   = { 0x0 };

            a113::Dispenser< std::array< uint8_t, 256 > >   data        = { a113::DispenserMode_Swap };
        } *_bridge;

        struct _ui_t {
            struct _mode_t : public a113::clkwrk::imm_widgets::DropDownList {
                _mode_t() : DropDownList{ ( const char* const[] ){ "once", "repeat" }, 2, 0x0 } {}
            } mode;
            struct _func1_t : public a113::clkwrk::imm_widgets::DropDownList {
                _func1_t() : DropDownList{ ( const char* const[] ){ "read", "write" }, 2, 0x0 } {}
            } func1;
            struct _func2_t : public std::array< a113::clkwrk::imm_widgets::DropDownList, 2 > {
                _func2_t() : std::array< a113::clkwrk::imm_widgets::DropDownList, 2 >{ {
                    { ( const char* const[] ){ "discrete inputs", "coils", "input registers", "holding registers" }, 4, 0x0 },
                    { ( const char* const[] ){ "coils", "holding registers" }, 2, 0x0 }
                } } {}
            } func2;
            struct _data_type_t : public a113::clkwrk::imm_widgets::DropDownList {
                _data_type_t() : DropDownList{ ( const char* const[] ){
                    "None", "Binary", "ASCII", "Int16", "Uint16", "Int32", "Uint32", "Float32", "Float64" 
                }, 9, 0x0 } {}

                bool          reverse   = false;
                std::string   input     = "";
            };

            struct _config_t {
                uint8_t   unit       = 0x0;
                int       address    = 0x0;
                int       count      = 0;
                int       interval   = 1'000;
            } config;

            std::vector< _data_type_t >   data_types   = {};
            float                         gs_red       = 1.0;
        } _ui;

        void main( std::shared_ptr< ModbusClientPort > port_, std::shared_ptr< std::mutex > port_mtx_, _bridge_t* bridge_ ) { 
            for( int control = bridge_->control; control != -0x1; control = bridge_->control ) {
                auto rule_def = bridge_->rule_def.load();

                if( 0x0 == control || 0x0 == ( rule_def & _RULE_MODE_MSK ) ) { 
                    bridge_->control.wait( 0x0 ); 

                    control  = bridge_->control; if( control == -0x1 ) break;
                    rule_def = bridge_->rule_def;

                    if( 0x0 == ( rule_def & _RULE_MODE_MSK ) ) control = ( bridge_->control = 0x0 );
                }

                int                   status = 0x1;
                _bridge_t::_config_t& config = bridge_->config;   
            {
                std::lock_guard lock{ *port_mtx_ };
                switch( rule_def & _RULE_FUNC_MSK ) {
                    case _MAKE_RULE_FUNC( _RuleFunc1_Read, _RuleFunc2_DiscreteInputs ): {
                        auto data = bridge_->data.control();
                        status = port_->readDiscreteInputsAsBoolArray( config.unit.load(), config.address.load(), config.count.load(), ( bool* )data->data() );
                    break; }
                    case _MAKE_RULE_FUNC( _RuleFunc1_Read, _RuleFunc2_Coils ): {
                        auto data = bridge_->data.control();
                        status = port_->readCoilsAsBoolArray( config.unit.load(), config.address.load(), config.count.load(), ( bool* )data->data() );
                    break; }
                    case _MAKE_RULE_FUNC( _RuleFunc1_Read, _RuleFunc2_InputRegisters ): {
                        auto data = bridge_->data.control();
                        status = port_->readInputRegisters( config.unit.load(), config.address.load(), config.count.load(), ( uint16_t* )data->data() );
                    break; }
                    case _MAKE_RULE_FUNC( _RuleFunc1_Read, _RuleFunc2_HoldingRegisters ): {
                        auto data = bridge_->data.control();
                        status = port_->readHoldingRegisters( config.unit.load(), config.address.load(), config.count.load(), ( uint16_t* )data->data() );
                    break; }
                    
                    case _MAKE_RULE_FUNC( _RuleFunc1_Write, _RuleFunc2_Coils ): {
                        auto data = bridge_->data.watch();
                        status = port_->writeMultipleCoilsAsBoolArray( config.unit.load(), config.address.load(), config.count.load(), ( bool* )data->data() );
                    break; }
                    case _MAKE_RULE_FUNC( _RuleFunc1_Write, _RuleFunc2_HoldingRegisters ): {
                        auto data = bridge_->data.watch();
                        status = port_->writeMultipleRegisters( config.unit.load(), config.address.load(), config.count.load(), ( uint16_t* )data->data() );
                    break; }
                }
            }

                switch( status ) {
                    case Modbus::Status_Uncertain: bridge_->status = 0x1; break;
                    case Modbus::Status_Good:      bridge_->status = 0x0; break;
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
                ++bridge_->completed;
                
                std::this_thread::sleep_for( std::chrono::milliseconds{ ( int )( ( rule_def & _RULE_MODE_MSK ) ? config.interval.load() : 0 ) } );
            } 
            delete bridge_;
        }

        a113::status_t ui_frame( double dt_, void* arg_ ) {
            bool rule_changed = false;
            bool config_changed  = false;

            ImGui::Text( "On unit" );

            ImGui::SameLine(); ImGui::SetNextItemWidth( 50 ); 
            config_changed |= ImGui::InputScalar( "##unit", ImGuiDataType_U8, &_ui.config.unit, 0, 0, nullptr, ImGuiInputTextFlags_CharsDecimal );

            ImGui::SameLine(); ImGui::Text( "," ); 
            ImGui::SameLine(); ImGui::SetNextItemWidth( 100 ); 
            const auto selected_mode = _ui.mode.imm_frame( "##mode", &rule_changed );

            switch( selected_mode ) {
                case _RuleMode_Once: {
                break; }
                case _RuleMode_Repeat: {
                    ImGui::SameLine(); ImGui::Text( ", every" );
                    ImGui::SameLine(); ImGui::SetNextItemWidth( 50 ); 
                    config_changed |= ImGui::InputInt( "##interval", &_ui.config.interval, 0, 0, ImGuiInputTextFlags_CharsDecimal ); 
                    _ui.config.interval = std::clamp( _ui.config.interval, 1, 100'000 );
                    ImGui::SameLine(); ImGui::Text( "ms" );
                break; }
            }
            ImGui::SameLine(); ImGui::Text( "," );
            ImGui::SameLine(); ImGui::SetNextItemWidth( 100 ); 
            const auto selected_func1 = _ui.func1.imm_frame( "##func1", &rule_changed );

            ImGui::SameLine(); ImGui::SetNextItemWidth( 50 ); 
            config_changed |= ImGui::InputInt( "##Count", &_ui.config.count, 0, 0, ImGuiInputTextFlags_CharsDecimal ); 
            _ui.config.count = std::clamp( _ui.config.count, 0x0, 0xFF );

            ImGui::SameLine(); ImGui::SetNextItemWidth( 200 ); 
            const bool func1_is_write = selected_func1 == _RuleFunc1_Write;
            const auto selected_func2 = ( _ui.func2[ selected_func1 ].imm_frame( "##func2", &rule_changed ) << func1_is_write ) + func1_is_write;
            
            ImGui::SameLine(); ImGui::Text( "from address" );
            ImGui::SameLine(); ImGui::SetNextItemWidth( 50 ); 
            const bool address_changed = ImGui::InputInt( "##Address", &_ui.config.address, 0, 0, ImGuiInputTextFlags_CharsDecimal ); 
            _ui.config.address = std::clamp( _ui.config.address, 0x0, 0xFFFF );

            if( address_changed ) _ui.data_types.clear();
            config_changed |= address_changed;

            ImGui::SameLine(); ImGui::Text( "." );

            const auto rule_def = _MAKE_RULE_DEF( selected_mode, selected_func1, selected_func2 );
            if( rule_changed ) {
                _bridge->control  = 0x0;
                _bridge->rule_def = rule_def;
                _bridge->data.switch_swap_mode( func1_is_write ? a113::DispenserMode_ReverseSwap : a113::DispenserMode_Swap, [ func1_is_write ] ( a113::dispenser_config_t& config ) -> void {
                    _FBV( config.flags, func1_is_write, a113::DispenserFlags_SwapMode_CopyWhenReverseWatchAcquire );
                } );
            }
            
            if( config_changed ) {
                _bridge->config.unit     = _ui.config.unit;
                _bridge->config.address  = _ui.config.address;
                _bridge->config.count    = _ui.config.count;
                _bridge->config.interval = _ui.config.interval;
            }

            if( _ui.config.count < _ui.data_types.size() ) {
                _ui.data_types.resize( _ui.config.count );
             } else {
                while( _ui.config.count > _ui.data_types.size() ) _ui.data_types.push_back( {} );
            }

            const auto status = _bridge->status.load();
            _ui.gs_red += ( ( 0x0 == status ? 1.0 : 0.0 ) - _ui.gs_red ) * 2.4*dt_;

            ImGui::BeginDisabled( 0x0 != status && not func1_is_write );
                switch( rule_def & _RULE_FUNC_MSK ) {
                    case _MAKE_RULE_FUNC( _RuleFunc1_Read, _RuleFunc2_DiscreteInputs ): [[fallthrough]];
                    case _MAKE_RULE_FUNC( _RuleFunc1_Read, _RuleFunc2_Coils ): {
                        if( ImGui::BeginTable( "##data", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders ) ) {
                            ImGui::TableSetupColumn( "Address", ImGuiTableColumnFlags_None, 0.0, 0 );
                            ImGui::TableSetupColumn( "Value", ImGuiTableColumnFlags_None, 0.0, 1 );  
        
                            ImGui::TableHeadersRow();

                            auto data = _bridge->data.watch();
                    
                            for( int idx = 0x0; idx < _ui.config.count; ++idx ) {
                                const bool* crt_dat = ( bool* )&data->at( idx );

                                ImGui::PushID( idx );
                                    ImGui::TableNextRow( ImGuiTableRowFlags_None, 26.0 );

                                    int gs = ( sin( 2.0*ImGui::GetTime() + 3.1415 / 4.0 * idx ) + 1.0 ) / 2.0 * 56.0;
                                    ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, IM_COL32( gs, gs*_ui.gs_red, gs*_ui.gs_red, 255 ) );  

                                    ImGui::TableSetColumnIndex( 0 );
                                        ImGui::Text( std::format( "0x{:X}", _ui.config.address + idx ).c_str() );

                                    ImGui::TableSetColumnIndex( 1 );
                                        const bool value = *crt_dat;
                                        ImGui::PushStyleColor( ImGuiCol_Text, value ? ImVec4{ 0,.36,1,1 } : ImVec4{ 1,.36,0,1 } );
                                            ImGui::Text( "%s", value ? "On" : "Off" );
                                        ImGui::PopStyleColor();
                                ImGui::PopID();
                            }

                            ImGui::EndTable();
                        }
                    break; }

                    case _MAKE_RULE_FUNC( _RuleFunc1_Read, _RuleFunc2_InputRegisters ): [[fallthrough]];
                    case _MAKE_RULE_FUNC( _RuleFunc1_Read, _RuleFunc2_HoldingRegisters ): {
                        if( ImGui::BeginTable( "##data", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders ) ) {
                            ImGui::TableSetupColumn( "Address", ImGuiTableColumnFlags_None, 0.0, 0 );
                            ImGui::TableSetupColumn( "Raw value", ImGuiTableColumnFlags_None, 0.0, 1 );  
                            ImGui::TableSetupColumn( "Type", ImGuiTableColumnFlags_None, 0.0, 2 );  
                            ImGui::TableSetupColumn( "Reverse", ImGuiTableColumnFlags_None, 0.0, 3 );  
                            ImGui::TableSetupColumn( "Interpreted value", ImGuiTableColumnFlags_None, 0.0, 4 );  

                            ImGui::TableHeadersRow();

                            auto data      = _bridge->data.watch();
                            int  type_skip = 0;

                            for( int idx = 0x0; idx < _ui.config.count; ++idx ) {
                                const uint16_t* crt_dat = ( uint16_t* )&data->at( idx << 1 );

                                ImGui::PushID( idx );
                                    ImGui::TableNextRow( ImGuiTableRowFlags_None, 26.0 );

                                    int gs = ( sin( 2.0*ImGui::GetTime() + 3.1415 / 4.0 * idx ) + 1.0 ) / 2.0 * 56.0;
                                    ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, IM_COL32( gs, gs*_ui.gs_red, gs*_ui.gs_red, 255 ) );  

                                    ImGui::TableSetColumnIndex( 0 );
                                        ImGui::Text( std::format( "0x{:X}", _ui.config.address + idx ).c_str() );

                                    ImGui::TableSetColumnIndex( 1 );
                                        ImGui::Text( std::format( "{:016b}", *crt_dat ).c_str() );
                                    
                                    if( 0 == type_skip ) {
                                        ImGui::TableSetColumnIndex( 2 );
                                            ImGui::SetNextItemWidth( 120 );
                                            const auto selected = _ui.data_types[ idx ].imm_frame( "" );

                                        ImGui::TableSetColumnIndex( 3 );
                                            if( ImGui::RadioButton( "##reverse", _ui.data_types[ idx ].reverse ) ) _ui.data_types[ idx ].reverse ^= true;

                                            const bool reverse = _ui.data_types[ idx ].reverse;

                                        ImGui::TableSetColumnIndex( 4 );

                                        #define _EXTRACT( t ) (reverse ? std::byteswap(*(t*)(crt_dat)) : *(t*)(crt_dat))
                                            switch( selected ) {
                                                case 0x0: break;
                                                case 0x1: {
                                                    ImGui::Text( std::format( "{:016b}", _EXTRACT( uint16_t ) ).c_str() );
                                                break; }
                                                case 0x2: {
                                                    char c1 = ( *crt_dat ) >> 8;
                                                    char c2 = ( *crt_dat ) & 0xFF;
                                                    if( reverse ) std::swap( c1, c2 );
                                                    ImGui::Text( "%c%c", c1, c2 );
                                                break; }
                                                case 0x3: {
                                                    ImGui::Text( "%d", ( int )_EXTRACT( int16_t ) );
                                                break; }
                                                case 0x4: {
                                                    ImGui::Text( "%u", ( unsigned int )_EXTRACT( uint16_t ) );
                                                break; }
                                                case 0x5: {
                                                    ImGui::Text( "%d", _EXTRACT( int32_t ) ); type_skip = 1;
                                                break; }
                                                case 0x6: {
                                                    ImGui::Text( "%u", _EXTRACT( uint32_t ) ); type_skip = 1;
                                                break; }
                                                case 0x7: {
                                                    ImGui::Text( "%f", *( float* )crt_dat ); type_skip = 1;
                                                break; }
                                                case 0x8: {
                                                    ImGui::Text( "%lf", *( double* )crt_dat ); type_skip = 3;
                                                break; }
                                            }
                                        #undef _EXTRACT
                                    } else {
                                        --type_skip;
                                    }

                                ImGui::PopID();
                            }

                            ImGui::EndTable();
                        }
                    break; }

                    case _MAKE_RULE_FUNC( _RuleFunc1_Write, _RuleFunc2_Coils ): {
                        if( ImGui::BeginTable( "##data", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders ) ) {
                            ImGui::TableSetupColumn( "Address", ImGuiTableColumnFlags_None, 0.0, 0 );
                            ImGui::TableSetupColumn( "Value", ImGuiTableColumnFlags_None, 0.0, 1 );  
        
                            ImGui::TableHeadersRow();

                            auto data = _bridge->data.control();
                    
                            for( int idx = 0x0; idx < _ui.config.count; ++idx ) {
                                bool* crt_dat = ( bool* )&data->at( idx );

                                ImGui::PushID( idx );
                                    ImGui::TableNextRow( ImGuiTableRowFlags_None, 26.0 );

                                    int gs = ( sin( 2.0*ImGui::GetTime() + 3.1415 / 4.0 * idx ) + 1.0 ) / 2.0 * 56.0;
                                    ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, IM_COL32( gs, gs*_ui.gs_red, gs*_ui.gs_red, 255 ) );  

                                    ImGui::TableSetColumnIndex( 0 );
                                        ImGui::Text( std::format( "0x{:X}", _ui.config.address + idx ).c_str() );

                                    ImGui::TableSetColumnIndex( 1 );
                                        if( ImGui::RadioButton( "##value", *crt_dat ) ) *crt_dat ^= 0x1;
                                ImGui::PopID();
                            }

                            ImGui::EndTable();
                        }
                    break; }
                    case _MAKE_RULE_FUNC( _RuleFunc1_Write, _RuleFunc2_HoldingRegisters ): {
                        if( ImGui::BeginTable( "##data", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders ) ) {
                            ImGui::TableSetupColumn( "Address", ImGuiTableColumnFlags_None, 0.0, 0 );
                            ImGui::TableSetupColumn( "Raw value", ImGuiTableColumnFlags_None, 0.0, 1 );  
                            ImGui::TableSetupColumn( "Type", ImGuiTableColumnFlags_None, 0.0, 2 );  
                            ImGui::TableSetupColumn( "Reverse", ImGuiTableColumnFlags_None, 0.0, 3 );  
                            ImGui::TableSetupColumn( "Value", ImGuiTableColumnFlags_None, 0.0, 4 );  

                            ImGui::TableHeadersRow();

                            auto data      = _bridge->data.control();
                            int  type_skip = 0;

                            for( int idx = 0x0; idx < _ui.config.count; ++idx ) {
                                const uint16_t* crt_dat = ( uint16_t* )&data->at( idx << 1 );

                                ImGui::PushID( idx );
                                    ImGui::TableNextRow( ImGuiTableRowFlags_None, 26.0 );

                                    int gs = ( sin( 2.0*ImGui::GetTime() + 3.1415 / 4.0 * idx ) + 1.0 ) / 2.0 * 56.0;
                                    ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, IM_COL32( gs, gs*_ui.gs_red, gs*_ui.gs_red, 255 ) );  

                                    ImGui::TableSetColumnIndex( 0 );
                                        ImGui::Text( std::format( "0x{:X}", _ui.config.address + idx ).c_str() );

                                    ImGui::TableSetColumnIndex( 1 );
                                        ImGui::Text( std::format( "{:016b}", *crt_dat ).c_str() );
                                    
                                    if( 0 == type_skip ) {
                                        ImGui::TableSetColumnIndex( 2 );
                                            ImGui::SetNextItemWidth( 120 );
                                            const auto selected = _ui.data_types[ idx ].imm_frame( "" );

                                        ImGui::TableSetColumnIndex( 3 );
                                            if( ImGui::RadioButton( "##reverse", _ui.data_types[ idx ].reverse ) ) _ui.data_types[ idx ].reverse ^= true;

                                            const bool reverse = _ui.data_types[ idx ].reverse;

                                        ImGui::TableSetColumnIndex( 4 );
                                            bool value_changed = false;
                                            if( 0x0 != selected ) {
                                                ImGui::SetNextItemWidth( 200.0 );
                                                ImGui::InputText( "##data_field", &_ui.data_types[ idx ].input, ImGuiInputTextFlags_CharsNoBlank );
                                            }

                                        #define _INJECT( t, x ) (*(t*)crt_dat = (t)(reverse ? std::byteswap((t)(x)) : (t)(x)))
                                        #define _WHAT {ImGui::PushStyleColor( ImGuiCol_Text, ImVec4{ 1,0,0,1 } ); ImGui::SameLine(); ImGui::Text( "???" ); ImGui::PopStyleColor( 1 );}
                                            switch( selected ) {
                                                case 0x0: break;
                                                case 0x1: {
                                                    try{ _INJECT( uint16_t, std::stoi( _ui.data_types[ idx ].input, nullptr, 2 ) ); } 
                                                    catch(...) { _WHAT }
                                                break; }
                                                case 0x2: {
                                                    const auto sz = _ui.data_types[ idx ].input.size();
                                                    if( sz == 0 ) _WHAT
                                                    else {
                                                        memcpy( ( void* )crt_dat, ( void* )_ui.data_types[ idx ].input.data(), ( sz&1 ) ? sz+1 : sz );
                                                        type_skip = ( sz-1 ) / 2;
                                                    }
                                                break; }
                                                case 0x3: {
                                                    try{ _INJECT( int16_t, std::stoi( _ui.data_types[ idx ].input, nullptr, 10 ) ); } 
                                                    catch(...) { _WHAT }
                                                break; }
                                                case 0x4: {
                                                    try{ _INJECT( uint16_t, std::stoi( _ui.data_types[ idx ].input, nullptr, 10 ) ); } 
                                                    catch(...) { _WHAT }
                                                break; }
                                                case 0x5: {
                                                    try{ _INJECT( int32_t, std::stoi( _ui.data_types[ idx ].input, nullptr, 10 ) ); } 
                                                    catch(...) { _WHAT }
                                                    type_skip = 1;
                                                break; }
                                                case 0x6: {
                                                    try{ _INJECT( uint32_t, std::stoi( _ui.data_types[ idx ].input, nullptr, 10 ) ); } 
                                                    catch(...) { _WHAT }
                                                    type_skip = 1;
                                                break; }
                                                case 0x7: {
                                                    try{ *( float* )crt_dat = std::stof( _ui.data_types[ idx ].input ); } 
                                                    catch(...) { _WHAT }
                                                    type_skip = 1;
                                                break; }
                                                case 0x8: {
                                                    try{ *( double* )crt_dat = std::stod( _ui.data_types[ idx ].input ); } 
                                                    catch(...) { _WHAT }
                                                    type_skip = 3;
                                                break; }
                                            }
                                        #undef _WHAT
                                        #undef _INJECT
                                    } else {
                                        --type_skip;
                                    }

                                ImGui::PopID();
                            }

                            ImGui::EndTable();
                        }
                    break; }
                }
            ImGui::EndDisabled();

            return 0x0;
        }

        std::pair< bool, const char* > status_str( void ) {
            const auto status = _bridge->status.load();
            return { status == 0x0, _STATUS_STRS[ status ] };
        }

    };
    struct _rules_data_t {
        std::list< _rule_t >   list;
    } _rules;

public:
    a113::status_t ui_frame( double dt_, void* arg_ ) override {
        ImGui::SeparatorText( "Found COM ports" );

        const bool port_conn   = ( bool )_mb.port;
        auto       ports_watch = G_Launcher.common.com_ports.watch();

        ImGui::BeginDisabled( port_conn );
            auto port = _ui.ports.imm_frame( ports_watch );
        ImGui::EndDisabled();

        const bool port_not_sel_or_conn = not port || port_conn;
    
        ImGui::SeparatorText( "Connection settings" );
        ImGui::BeginDisabled( port_not_sel_or_conn );
            ImGui::LabelText( "COM port", port ? port->id.c_str() : "N/A" ); ImGui::SetItemTooltip( "Use the above panel to select a COM port." );
            ImGui::InputInt( "Baud rate", &_mb.settings.baudRate, 0, 0 );
            ImGui::InputScalar( "Data bits", ImGuiDataType_S8, &_mb.settings.dataBits, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );

            const auto selected_parity   = ( Modbus::Parity )_ui.parity.imm_frame( "Parity" );
            const auto selected_stopbit  = ( Modbus::StopBits )_ui.stopbit.imm_frame( "Stop bit" );
            const auto selected_flow_ctl = ( Modbus::FlowControl )_ui.flow.imm_frame( "Flow control" );

            ImGui::InputScalar( "First-byte timeout", ImGuiDataType_U32, &_mb.settings.timeoutFirstByte, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );
            ImGui::SetItemTooltip( "Maximum time to receive the first response byte, in milliseconds." );
            ImGui::InputScalar( "Inter-byte timeout", ImGuiDataType_U32, &_mb.settings.timeoutInterByte, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );
            if( ImGui::BeginItemTooltip() ) {
                ImGui::Text( "Maximum time to receive the next response byte, in milliseconds." );
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4{ 1,.36,0,1 } );
                    ImGui::TextWrapped( "WARNING: The underlying modbus library executes a single read of modbus' maximum packet length bytes."\
                                        " Therefore, the read stops when this timeout expires, i.e. %ums after the packet has been recieved.", _mb.settings.timeoutInterByte );
                ImGui::PopStyleColor( 1 );
                ImGui::EndTooltip();
            }
        ImGui::EndDisabled();

        ImGui::Separator();
        if( port_conn ) {
            ImGui::TextColored( ImVec4{ 0,1,0,1 }, "Connected on %s", _mb.settings.portName );
        } else {
            ImGui::TextColored( ImVec4{ 1,0,0,1 }, "Disconnected" );
        }
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( port_not_sel_or_conn );
            if( ImGui::Button( "Connect" ) ) {
                _mb.settings.portName    = port->id.c_str();
                _mb.settings.parity      = selected_parity;
                _mb.settings.stopBits    = selected_stopbit;
                _mb.settings.flowControl = selected_flow_ctl;

                _mb.port.reset( Modbus::createClientPort( Modbus::RTU, &_mb.settings, true ), [] ( ModbusClientPort* port_ ) -> void {
                    port_->close();
                    spdlog::info( "Closed a ModbusRTU client port." );
                    delete port_;
                } );
                if( _mb.port ) spdlog::info( "Created a new ModbusRTU client port." );
                else spdlog::error( "Failed to create a new ModbusRTU client port." );
            }
        
            const bool port_available = ( bool )port;
            ports_watch.release();

            ImGui::SetItemTooltip( "Attempt a connection using the above configured settings." );
        ImGui::EndDisabled();
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( not port_conn );
            if( ImGui::Button( "Disconnect" ) || not port_available ) {
                _rules.list.clear();
                _mb.settings.portName = "";
                _mb.port.reset();
            }
            ImGui::SetItemTooltip( "Terminate the current connection." );
        ImGui::EndDisabled();
        ImGui::Separator();

        ImGui::BeginDisabled( not port_conn );
            if( ImGui::Button( "+" ) ) {
                _rules.list.emplace_back( _mb.port, _mb.port_mtx );
            }
            ImGui::SetItemTooltip( port_conn ? "Add new rule." : "Connect before adding rules." );
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::SeparatorText( "Rules" );
        ImGui::Separator();

        ImGui::BeginChild( "Rules" );
            int ui_ID = 0x0;
            for( auto itr = _rules.list.begin(); itr != _rules.list.end(); ++ui_ID ) {
                bool remove = false;
                
                ImGui::PushID( ui_ID ); 
                    remove |= a113::clkwrk::imm_widgets::small_X_button();

                    ImGui::SameLine(); ImGui::Text( "%d", itr->_bridge->completed.load() );
                    ImGui::SameLine();

                    const auto rule_def = itr->_bridge->rule_def.load();
                    if( rule_def & _RULE_MODE_MSK ) {
                        if( ImGui::RadioButton( "##active", itr->_bridge->control.load() ) ) {
                            itr->_bridge->control ^= 0x1; itr->_bridge->control.notify_one();
                        }
                    } else {
                        if( ImGui::ArrowButton( "##exec", ImGuiDir_Right ) ) {
                            itr->_bridge->control = 0x1; itr->_bridge->control.notify_one();
                        }
                    }

                    auto [ status_ok, status_str ] = itr->status_str();
                    ImGui::PushStyleColor( ImGuiCol_Text, status_ok ? ImVec4{ 0,1,0,1 } : ImVec4{ 1,0,0,1 } );
                        ImGui::SameLine(); ImGui::SeparatorText( status_str );
                    ImGui::PopStyleColor( 1 );

                    remove |= itr->ui_frame( dt_, arg_ ) != 0x0;
                    ImGui::Separator();
                ImGui::PopID();

                if( not remove ) goto l_keep;
            l_remove:
                itr = _rules.list.erase( itr );
                continue;
            l_keep:
                ++itr;
            }
        ImGui::EndChild();

        return 0x0;
    }

protected:
    #undef _MAKE_RULE_FUNC
    #undef _MAKE_RULE_DEF

};


class SerialMonitor : public Component {
public:
    SerialMonitor( void ) {
        _write_th = std::thread{ &SerialMonitor::_main_write_th, this, _ser }; _write_th.detach();
        _read_th = std::thread{ &SerialMonitor::_main_read_th, this, _ser }; _read_th.detach();
    }

protected:
    struct _ui_data_t {
        a113::clkwrk::imm_widgets::COM_Ports   ports{ G_Launcher.common.com_ports };

        struct _parity_t : public a113::clkwrk::imm_widgets::DropDownList {
            _parity_t() : DropDownList{ ( const char* const[] ){ "No parity", "Odd parity", "Even parity", "Mark parity", "Space parity" }, 5, 0x0 } {}
        } parity;
        struct _stopbit_t : public a113::clkwrk::imm_widgets::DropDownList {
            _stopbit_t() : DropDownList{ ( const char* const[] ){ "One", "One & 1/2", "Two" }, 3, 0x0 } {}
        } stopbit;
        struct _line_feed_t : public a113::clkwrk::imm_widgets::DropDownList {
            _line_feed_t() : DropDownList{ ( const char* const[] ){ "None", "New line", "Carriage return", "Both" }, 4, 0x0 } {}
        } line_feed;

        std::string   tx_str   = "";
    } _ui;

    struct _ser_data_t {
        a113::io::Serial                 port           = {};
        std::mutex                       port_mtx       = {};
        a113::io::serial_config_t        config         = { .baud_rate = 115200 };
        std::atomic_int                  dmp_file_ver   = 0x0;
        std::string                      dmp_file_str   = "";
        std::atomic_int                  tx_ver         = 0x0;
        std::string                      tx_str         = "";
        a113::Dispenser< std::string >   acc            = { a113::DispenserMode_Lock };
    };
    a113::HVec< _ser_data_t >   _ser        = a113::HVec< _ser_data_t >::make();
    std::thread                 _write_th   = {};
    std::thread                 _read_th    = {};

protected:
    void _main_write_th( a113::HVec< _ser_data_t > ser_ ) { 
        int last_tx_ver = 0x0;
        
    for(; ser_.use_count() > 2;) {
        if( not ser_->port.is_connected() ) { std::this_thread::sleep_for( std::chrono::milliseconds{ 100 } ); continue; }

        const int crt_tx_ver = ser_->tx_ver.load( std::memory_order_acquire );
        if( last_tx_ver == crt_tx_ver ) { std::this_thread::sleep_for( std::chrono::milliseconds{ 100 } ); continue; }

        ser_->port.write( a113::io::port_RW_desc_t{
            .ptr_SoD = ser_->tx_str.data(),
            .n_SoD   = ser_->tx_str.length()
        } );
        last_tx_ver = ser_->tx_ver.fetch_add( 0x1, std::memory_order_release ) + 0x1;
    } }

    void _main_read_th( a113::HVec< _ser_data_t > ser_ ) {
        std::ofstream dmp_file;
        int           last_dmp_file_ver = 0x0;

    for(; ser_.use_count() > 2;) {
        if( not ser_->port.is_connected() ) { std::this_thread::sleep_for( std::chrono::milliseconds{ 100 } ); continue; }

        char   buffer[ 512 ];
        size_t read_bytes      = 0;

        A113_ASSERT_OR( 0x0 == ser_->port.read( a113::io::port_RW_desc_t{
            ptr_SoD:    buffer,
            n_SoD:      sizeof( buffer ),
            byte_count: &read_bytes
        } ) ) continue;
        A113_ASSERT_OR( read_bytes > 0 ) continue;

        auto acc = ser_->acc.control();
        if( acc->append( buffer, read_bytes ).length() > 10*512 ) {
            acc->erase( 0, 512 );
        }
        acc.commit();

        if( int crt_dmp_file_ver = ser_->dmp_file_ver.load( std::memory_order_acquire ); last_dmp_file_ver != crt_dmp_file_ver ) {
            if( not ser_->dmp_file_str.empty() )
                dmp_file.open( ser_->dmp_file_str );
            else
                dmp_file.close();
            last_dmp_file_ver = crt_dmp_file_ver;
        }
        if( dmp_file ) dmp_file.write( buffer, read_bytes );
    } }

public:
    a113::status_t ui_frame( double dt_, void* arg_ ) override {
        ImGui::SeparatorText( "Found COM ports" );

        const bool port_conn   = _ser->port.is_connected();
        auto       ports_watch = G_Launcher.common.com_ports.watch();

        ImGui::BeginDisabled( port_conn );
            auto port = _ui.ports.imm_frame( ports_watch );
        ImGui::EndDisabled();

        const bool port_not_sel_or_conn = not port || port_conn;
    
        ImGui::SeparatorText( "Connection settings" );
        ImGui::BeginDisabled( port_not_sel_or_conn );
            ImGui::LabelText( "COM port", port ? port->id.c_str() : "N/A" ); ImGui::SetItemTooltip( "Use the above panel to select a COM port." );
            ImGui::InputScalar( "Baud rate", ImGuiDataType_U32, &_ser->config.baud_rate, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );
            ImGui::InputScalar( "Data bits", ImGuiDataType_S8, &_ser->config.byte_size, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );

            const auto selected_parity   = _ui.parity.imm_frame( "Parity" );
            const auto selected_stopbit  = _ui.stopbit.imm_frame( "Stop bit" );

            ImGui::InputScalar( "Update timeout", ImGuiDataType_U32, &_ser->config.rx_ib_timeout, nullptr, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal );
            ImGui::SetItemTooltip( "The time in milliseconds needed to elapse since the last byte was received, in order to update the view buffer.\nNote that an update is triggered when the reception buffer is full." );
        ImGui::EndDisabled();

        ImGui::Separator();
        if( port_conn ) {
            ImGui::TextColored( ImVec4{ 0,1,0,1 }, "Connected on %s", _ser->port.device().data() );
        } else {
            ImGui::TextColored( ImVec4{ 1,0,0,1 }, "Disconnected" );
        }
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( port_not_sel_or_conn );
            if( ImGui::Button( "Connect" ) ) {
                _ser->config.parity  = selected_parity;
                _ser->config.stopbit = selected_stopbit;

                _ser->config.rx_fb_timeout = 200;
                
                _ser->port.open( std::format( "\\\\.\\{}", port->id.c_str() ).c_str(), _ser->config );
            }
        
            const bool port_available = ( bool )port;
            ports_watch.release();

            ImGui::SetItemTooltip( "Attempt a connection using the above configured settings." );
        ImGui::EndDisabled();
        ImGui::SameLine(); ImGui::Bullet();
        ImGui::BeginDisabled( not port_conn );
            if( ImGui::Button( "Disconnect" ) || not port_available ) {
                _ser->port.close();
            }
            ImGui::SetItemTooltip( "Terminate the current connection." );
        ImGui::EndDisabled();
        ImGui::Separator();

        ImGui::SeparatorText( "Dump to file" );

        if( ImGui::Button( "Choose file" ) ) {
            ImGuiFileDialog::Instance()->OpenDialog( "File_explorer", "Choose serial port dump file", ".txt,.*", IGFD::FileDialogConfig{
                .path  = ".",
                .flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_DisableCreateDirectoryButton
            } );
        }
        if( ImGuiFileDialog::Instance()->Display( "File_explorer" ) ) {
            if( ImGuiFileDialog::Instance()->IsOk() ) {
                _ser->dmp_file_str = ImGuiFileDialog::Instance()->GetFilePathName();
                _ser->dmp_file_ver.fetch_add( 0x1, std::memory_order_release );
            }
            
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SameLine(); ImGui::Bullet();
        if( not _ser->dmp_file_str.empty() ) {
            ImGui::Text( _ser->dmp_file_str.c_str() );
            ImGui::SameLine();
            if( a113::clkwrk::imm_widgets::small_X_button() ) {
                _ser->dmp_file_str.clear();
                _ser->dmp_file_ver.fetch_add( 0x1, std::memory_order_release );
            }
        } else {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4{ 1,.36,0,1 } );
                ImGui::Text( "No dump file selected." );
            ImGui::PopStyleColor( 1 );
        }

        ImGui::SeparatorText( "Transmit" );
        ImGui::BeginDisabled( not port_conn || _ser->tx_ver.load( std::memory_order_acquire ) & 0x1 );
            bool signal_tx = false;

            signal_tx |= ImGui::InputText( "##TX_input", &_ui.tx_str, ImGuiInputTextFlags_EnterReturnsTrue );
            ImGui::SameLine();
            ImGui::SetNextItemWidth( 160 );
            const auto selected_line_feed = _ui.line_feed.imm_frame( "##line_feed" );
            ImGui::SameLine();
            signal_tx |= ImGui::ArrowButton( "##TX_button", ImGuiDir_Right );
            
            if( signal_tx ) {
                _ser->tx_str = _ui.tx_str;
                switch( selected_line_feed ) {
                    case 0x0: break;
                    case 0x1: _ser->tx_str += '\n'; break;
                    case 0x2: _ser->tx_str += '\r'; break;
                    case 0x3: _ser->tx_str += "\r\n"; break;
                }
                _ser->tx_ver.fetch_add( 0x1, std::memory_order_release );
            }
        ImGui::EndDisabled();

        ImGui::SeparatorText( "Reception dump" );
        if( a113::clkwrk::imm_widgets::small_X_button() ) {
            auto acc = _ser->acc.control();
            acc->clear();
        }
        ImGui::SetItemTooltip( "Clear accumulated reception buffer." );
        ImGui::Separator();
        ImGui::BeginChild( "Reception dump" );
            auto acc = _ser->acc.watch();
            ImGui::Text( acc->c_str() );
            acc.release();
            ImGui::SetScrollHereY( 1 );
        ImGui::EndChild();

        return 0x0;
    }
};


a113::status_t Launcher::ui_frame( double dt_, void* arg_ ) {
    bool main_window_open = true;
    ImGui::Begin( "COMMod", &main_window_open, ImGuiWindowFlags_None );
        ImGui::BeginChild( "Launcher", ImVec2{ 200, 0 }, ImGuiChildFlags_Border );
            ImGui::SeparatorText( "COMMod "COMMOD_VERSION_STR );
            ImGui::Bullet(); ImGui::TextLinkOpenURL( "GitHub", "https://github.com/HoratiuMip/Ad-astra.Made.Not-said/tree/main/AUTO-A113/Ruptures/COMMod" );
            ImGui::Bullet(); ImGui::TextLinkOpenURL( "YouTube", "https://www.youtube.com/@horatiumip" );
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4{ 1,.36,0,1 } );
                ImGui::Bullet(); ImGui::Text( "Still alpha! :)" );
            ImGui::PopStyleColor( 1 );
            ImGui::Separator();

            static struct _component_t {
                const char* const                                  name;
                std::function< a113::HVec< Component >( void ) >   builder; 
                int                                                count      = 0x0;
            } components[] = {
                { .name = "ModbusRTU", .builder = [] ( void ) -> a113::HVec< Component > { return a113::HVec< ModbusRTU >::make(); } },
                { .name = "SerialMonitor", .builder = [] ( void ) -> a113::HVec< Component > { return a113::HVec< SerialMonitor >::make(); } }
            };

            ImGui::NewLine();
            ImGui::SeparatorText( "Available panels" );
            for( auto& comp : components ) {
                ImGui::Separator();
                ImGui::BulletText( comp.name ); ImGui::SameLine();
                if( ImGui::ArrowButton( comp.name, ImGuiDir_Right ) ) {
                    this->register_component( std::format( "{}-{}", comp.name, ++comp.count ), comp.builder() );
                }
                ImGui::SetItemTooltip( std::format( "Launch a new {} tab.", comp.name ).c_str() );
                ImGui::Separator();
            }

            ImGui::NewLine();
            ImGui::SeparatorText( "Options" );

            static bool active_theme = 0x0;
            if( ImGui::Button( active_theme ? "Make it night." : "Make it day." ) ) {
                if( active_theme ^= 0x1 ) ImGui::StyleColorsLight();
                else ImGui::StyleColorsDark();
            }

        ImGui::EndChild(); 
        ImGui::SameLine(); 
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
    return main_window_open ? 0x0 : -0x1;
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