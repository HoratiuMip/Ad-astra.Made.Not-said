#pragma once

#include <warc-spec-mod/common.hpp>
#include <warc/earth-imm.hpp>
#include <IXT/SpecMod/barracuda_controller.hpp>

namespace warc { namespace spec_mod {


class BARRACUDA_CONTROLLER : virtual public DEVICE, virtual public IXT::SpecMod::BarracudaController {
public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_SPEC_MOD_STR"::BARRACUDA_CONTROLLER" );

public:
    BARRACUDA_CONTROLLER( IXT_COMMS_ECHO_ARG ) {
        std::atomic_bool data_link_complete{ false };

        _imm_control_th = std::thread( [ &data_link_complete, this ] () mutable -> void {
            struct _RELEASE_EXIT_LEASH {
                ~_RELEASE_EXIT_LEASH() { proc(); } std::function< void() > proc;
            } release_exit_leash{ proc: [ &data_link_complete ] () -> void { data_link_complete.store( true, std::memory_order_release ); data_link_complete.notify_one(); } };

            int status = -1;
            
            status = this->IXT::SpecMod::BarracudaController::data_link( L"BARRACUDA", 0 );
            WARC_ASSERT_RT_THIS( status == 0, "Could not connect to the BARRACUDA controller.", status, ; );

            status = this->IXT::SpecMod::BarracudaController::write( "X", 2 );
            WARC_ASSERT_RT_THIS( status >= 0, "Could not TX initial byte string.", status, ; );

            release_exit_leash.proc();

            static constexpr const int BUFFER_SIZE = 512;
            static constexpr const int READ_FAULT_SLEEP_S = 3;

            //char front_sentinel[] = "BARRA-FRONT-SENTINEL";
            char buffer[ BUFFER_SIZE + 1 ]; buffer[ BUFFER_SIZE ] = 0; // <- Matei. ( he's between BARRA's, did you get it? )
            //char back_sentinel[] = "BACK-SENTINEL-BARRA";

            while( true ) {
                int status = this->IXT::SpecMod::BarracudaController::read( buffer, BUFFER_SIZE );

                if( status < 0 ) {
                    WARC_ECHO_RT_WARNING << "Read fault, retrying in ( " << READ_FAULT_SLEEP_S << " )s.";
                    std::this_thread::sleep_for( std::chrono::seconds( READ_FAULT_SLEEP_S ) );
                    continue;
                }

                buffer[ status ] = 0;
                WARC_ECHO_RT_THIS_INTEL << "RX'd:\n" << buffer;

                if( std::string_view{ buffer, status }.find( "( G, Y, R, B ) = ( 0" ) != std::string_view::npos ) {
                    auto& ctrl = this->soft_params< imm::EARTH >()->ctrl();
                    ctrl.trigger( ctrl.idxs.sat_high_2t );
                }
            }
        } );

        data_link_complete.wait( false, std::memory_order_acquire );
    }

_WARC_PROTECTED:
    std::thread   _imm_control_th   = {};

};


} };