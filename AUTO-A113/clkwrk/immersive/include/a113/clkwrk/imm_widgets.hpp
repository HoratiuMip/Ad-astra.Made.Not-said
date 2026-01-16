#pragma once
/**
 * @file: clkwrk/imm_widgets.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/clkwrk/immersive.hpp>

namespace a113::clkwrk::imm_widgets {


class DropDownList {
public:
    DropDownList( const char* const strs_[], int size_, int selected_ = -0x1 ) : _strs{ strs_ }, _size{ size_ }, _sel{ selected_ } {}

_A113_PROTECTED:
    int                  _sel     = -0x1;
    const char* const*   _strs    = nullptr;
    int                  _size    = 0x0;

public:
    int imm_frame( const char* const title_, bool* changed_ = nullptr ) {
        const int prev_sel = _sel;

        if( ImGui::BeginCombo( title_, _sel >= 0 && _sel < _size ? _strs[ _sel ] : "N/A" ) ) {
            for( int idx = 0x0; idx < _size; ++idx ) {
                const bool selected = idx == _sel;
                if( ImGui::Selectable( _strs[ idx ], selected ) ) _sel = idx;
            }
            ImGui::EndCombo();
        }

        if( changed_ ) *changed_ |= _sel != prev_sel;
        return _sel;
    }

};

class COM_Ports {
public:
    COM_Ports( HVec< io::COM_Ports > ports_ ) : _ports{ std::move( ports_ ) } {
        /* @note The registered refresh callback lambda function is invoked under a dispenser control, 
            therefore there is no need to worry about races over the container. */
        _ports->register_refresh_callback( A113_TXTUUID_FROM_THIS, [ this ] ( io::COM_Ports::container_t& ports_ ) -> void {
            auto itr = std::ranges::find_if( ports_, [ this ] ( const io::COM_port_t& port_ ) -> bool {
                return port_.id == _sel_id;
            } );
            A113_ASSERT_OR( ports_.end() != itr ) {
                _sel    = -0x1;
                _sel_id = "";
                return;
            }

            _sel = std::distance( ports_.begin(), itr );
        } );
    }

    ~COM_Ports( void ) {
        _ports->unregister_refresh_callback( A113_TXTUUID_FROM_THIS );
    }

_A113_PROTECTED:
    HVec< io::COM_Ports >   _ports    = nullptr;
    int                     _sel      = -0x1;
    std::string             _sel_id   = "";

public:
    io::COM_port_t* imm_frame( io::COM_Ports::watch_t& watch_ ) {
        auto& ports = *watch_;

        for( int idx = 0x0; idx < ports.size(); ++idx ) {
            const bool selected = idx == _sel;

            ImGui::Separator();
            if( ImGui::Selectable( ports[ idx ].friendly.c_str(), selected, selected ? ImGuiSelectableFlags_Highlight : ImGuiSelectableFlags_None ) ) {
                _sel    = idx;
                _sel_id = ports[ _sel ].id;
            }
            ImGui::Separator();
        }

        if( _sel >= 0x0 && _sel < ports.size() ) return &ports[ _sel ]; 
        
        _sel    = -0x1;
        _sel_id = "";
        return nullptr;
    }

};


bool small_X_button( void ) {
    ImGui::PushStyleColor( ImGuiCol_Button,        ImVec4{ 0,0,0,0 } );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0,0,0,0 } );
    ImGui::PushStyleColor( ImGuiCol_ButtonActive,  ImVec4{ 0,0,0,0 } );
    ImGui::PushStyleColor( ImGuiCol_Text,          ImVec4{ 1,0,0,1 } );
    const bool pressed = ImGui::SmallButton( "X" );
    ImGui::PopStyleColor( 4 );
    return pressed;
}


};

