#pragma once

#include <warc-spec-mod/common.hpp>
#include <warc/earth-imm.hpp>
#include <IXT/SpecMod/barracuda_controller.hpp>

namespace warc { namespace spec_mod {


class BARRACUDA_CONTROLLER : virtual public DEVICE, virtual public IXT::SpecMod::BarracudaController {
public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_SPEC_MOD_STR"::BARRACUDA_CONTROLLER" );

public:
    BARRACUDA_CONTROLLER() = default;

    virtual ~BARRACUDA_CONTROLLER() override {}

public:
    virtual int set( warc::MAIN& main, IXT_COMMS_ECHO_ARG ) override {
        int status = 0;

        status = this->IXT::SpecMod::BarracudaController::data_link( L"BARRACUDA", 0 );
        WARC_ASSERT_RT_THIS( status == 0, "Could not connect to the BARRACUDA controller.", status, status );

        status = this->IXT::SpecMod::BarracudaController::write( "X", 2 );
        WARC_ASSERT_RT_THIS( std::exchange( status, 0 ) >= 0, "Could not TX initial byte string.", status, status );

        this->set_soft_params( &main.imm_earth() );

        return status;
    }

    virtual int engage( warc::MAIN& main, IXT_COMMS_ECHO_ARG ) override {
        int status = 0;

        WARC_ASSERT_RT_THIS( this->IXT::SpecMod::BarracudaController::uplinked(), "BARRACUDA controller not set.", -1, -1 );

        imm::EARTH* imm = this->soft_params< imm::EARTH >();
        WARC_ASSERT_RT_THIS( imm != nullptr, "Immersion pointer not flashed to soft parameter.", -1, -1 );

        _imm_idxs.lens_f = imm->ctrl().push_sink( imm::EARTH_CTRL_PARAMS::_SINK{
            name: "BARRACUDA-Controller-lens-f",
            proc: ( WARC_IMM_CTRL_SINK_PROC{
               
                imm->lens_spin( { 
                    abs( desc.rachel.x ) > 0.1 ? desc.rachel.x * elapsed : 0.0,
                    abs( desc.rachel.y ) > 0.1 ? desc.rachel.y * elapsed : 0.0 
                } );

                imm->lens_zoom( abs( desc.samantha.y ) > 0.1 ? desc.samantha.y * 1.6 * elapsed : 0.0 );

                return 0;
            } ),
            sexecc: -1
        } );
        imm->ctrl().insert_tokens( WARC_IMM_CTRL_INSERT_TOKENS_NO_CLEAR, _imm_idxs.lens_f );

        WARC_ECHO_RT_THIS_OK << "Engaged control pipework. Launching communication thread.";

        _imm_control_th = std::thread( [ imm, this ] () -> void { 
            while( this->DEVICE::_engaged.load( std::memory_order_relaxed ) ) {
                int status = this->IXT::SpecMod::BarracudaController::read_state_descriptor( &desc );

                if( status < sizeof( IXT::DWORD ) || status != this->desc._size ) {
                    WARC_ECHO_RT_THIS_WARNING << "Read fault( " << status << " ), retrying in " << read_error_timeout_s << "s.";
                    std::this_thread::sleep_for( std::chrono::seconds( read_error_timeout_s ) );
                    continue;
                }

                auto trigger = [ imm ] ( const auto& sw, const imm::EARTH_CTRL_PARAMS::DRAIN& drain ) -> void {
                    if( sw.prs ) imm->ctrl().trigger( drain );
                };
                trigger( desc.sw_b, imm->ctrl().idxs.cin_r2r );
                trigger( desc.sw_r, imm->ctrl().idxs.cnt_2t );
                trigger( desc.sw_y, imm->ctrl().idxs.sat_high_2t );
            }
        } );
    
        return status;
    }

    virtual int disengage( warc::MAIN& main, IXT_COMMS_ECHO_ARG ) override {
        if( _imm_control_th.joinable() ) _imm_control_th.join();
        WARC_ECHO_RT_THIS_OK << "Disengaged from control pipework. Communication thread shutdown.";
        return 0;
    }

_WARC_PROTECTED:
    std::thread                                             _imm_control_th        = {};
    struct _IMM_IDXS {
        imm::EARTH_CTRL_PARAMS::SINK    lens_f;
    }                                                       _imm_idxs              = {};

public:
    int                                                     read_error_timeout_s   = 5;
    IXT::SpecMod::barracuda_controller_state_descriptor_t   desc                   = {};

};


} };