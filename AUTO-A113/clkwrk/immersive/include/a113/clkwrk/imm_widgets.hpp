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
    DropDownList( const char* const strs_[], int size_, int selected_ = -0x1 ) : _strs{ strs_ }, _size{ size_ }, selected{ selected_ } {}

public:
    int                  selected   = -0x1;

_A113_PROTECTED:
    const char* const*   _strs    = nullptr;
    int                  _size    = 0x0;

public:
    int imm_frame( const char* const title_ ) {
        if( ImGui::BeginCombo( title_, selected >= 0 && selected < _size ? _strs[ selected ] : "N/A" ) ) {
            for( int idx = 0x0; idx < _size; ++idx ) {
                if( ImGui::Selectable( _strs[ idx ], idx == selected ) ) selected = idx;
            }
            ImGui::EndCombo();
        }

        return selected;
    }

};


class COM_Ports {
public:
    COM_Ports( HVec< io::COM_Ports > ports_ ) : _ports{ std::move( ports_ ) } {}

_A113_PROTECTED:
    HVec< io::COM_Ports >   _ports   = nullptr;
    int                     _sel     = -0x1;

public:
    io::COM_port_t*   port   = nullptr;

public:
    dispenser_watch< io::COM_Ports::container_t > imm_frame( void ) {
        dispenser_watch ports{ *_ports };

        for( int idx = 0x0; idx < ports->size(); ++idx ) {
            const bool selected = idx == _sel;

            ImGui::Separator();
            if( ImGui::Selectable( ( *ports )[ idx ].friendly.c_str(), selected, selected ? ImGuiSelectableFlags_Highlight : ImGuiSelectableFlags_None ) ) {
                _sel = idx;
            }
            ImGui::Separator();
        }

        if( _sel >= 0x0 && _sel < ports->size() ) { port = &( *ports )[ _sel ]; } else { _sel = -0x1; port = nullptr; }
        return ports;
    }

};


};

