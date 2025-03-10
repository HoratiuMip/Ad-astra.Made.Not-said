#pragma once

#include <warc-spec-mod/common.hpp>
#include <warc/earth-imm.hpp>
#include <NLN/SpecMod/barracuda-ctrl-nln-driver.hpp>

namespace warc { namespace spec_mod {


class BARRACUDA_CONTROLLER : virtual public DEVICE, virtual public NLN::SpecMod::BarracudaController {
public:
    NLN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_SPEC_MOD_STR"::BARRACUDA_CONTROLLER" );

public:
    BARRACUDA_CONTROLLER() = default;

    virtual ~BARRACUDA_CONTROLLER() override {}

public:
    virtual int set( warc::MAIN& main, NLN_COMMS_ECHO_RT_ARG ) override {
        int status = 0;

        status = this->NLN::SpecMod::BarracudaController::data_link( NLN::SpecMod::BARRACUDA_CTRL_FLAG_TRUST );
        WARC_ASSERT_RT_THIS( status == 0, "Could not connect to BarraCUDA-CTRL.", status, status );

        this->set_soft_params( &main.imm_earth() );

        return status;
    }

    virtual int engage( warc::MAIN& main, NLN_COMMS_ECHO_RT_ARG ) override {
        int status = 0;

        WARC_ASSERT_RT_THIS( this->NLN::SpecMod::BarracudaController::connected(), "BarraCUDA-CTRL not connected.", -1, -1 );

        imm::EARTH* imm = this->soft_params< imm::EARTH >();
        WARC_ASSERT_RT_THIS( imm != nullptr, "Immersion pointer not flashed to soft parameter.", -1, -1 );

        _imm_idxs.lens_f = imm->ctrl().push_sink( imm::EARTH_CTRL_PARAMS::_SINK{
            name: "BARRACUDA-Controller-lens-f",
            proc: ( WARC_IMM_CTRL_SINK_PROC{
                if( !_eligible.load( std::memory_order_relaxed ) ) return 0;

                imm->lens_spin( { 
                    abs( dy_st.samantha.x ) > 0.1 ? dy_st.samantha.x * elapsed : 0.0,
                    abs( dy_st.samantha.y ) > 0.1 ? dy_st.samantha.y * elapsed : 0.0 
                } );

                imm->lens_zoom( abs( dy_st.rachel.y ) > 0.1 ? dy_st.rachel.y * 1.6 * elapsed : 0.0 );

                if( dy_st.gran.acc.z >= 0.9 && abs( dy_st.gran.gyr.z ) >= 10 ) {
                    imm->lens_spin( { dy_st.gran.gyr.z / 250.0f * elapsed, 0.0 } );
                }

                return 0;
            } ),
            sexecc: -1
        } );
        imm->ctrl().insert_tokens( WARC_IMM_CTRL_INSERT_TOKENS_NO_CLEAR, _imm_idxs.lens_f );

        WARC_ECHO_RT_THIS_OK << "Engaged control pipework. Launching communication thread.";

        _imm_control_th = std::thread( [ imm, this ] () -> void { 
            while( this->DEVICE::_engaged.load( std::memory_order_relaxed ) ) {
                int status = this->NLN::SpecMod::BarracudaController::listen_trust( &dy_st );

                if( status != 0 ) {
                    _eligible.store( false, std::memory_order_release );
                    WARC_ECHO_RT_THIS_WARNING << "Read fault ( " << status << " ), retrying in " << read_error_timeout_s << "s.";
                    std::this_thread::sleep_for( std::chrono::seconds( read_error_timeout_s ) );
                    continue;
                }
                _eligible.store( true, std::memory_order_release );

                auto trigger = [ imm ] ( const auto& sw, const imm::EARTH_CTRL_PARAMS::DRAIN& drain ) -> void {
                    if( sw.prs ) imm->ctrl().trigger( drain );
                };
                trigger( dy_st.giselle, imm->ctrl().idxs.cin_r2r );
                trigger( dy_st.karina, imm->ctrl().idxs.cnt_2t );
                trigger( dy_st.ningning, imm->ctrl().idxs.sat_high_2t );
            }
        } );
    
        return status;
    }

    virtual int disengage( warc::MAIN& main, NLN_COMMS_ECHO_RT_ARG ) override {
        if( _imm_control_th.joinable() ) _imm_control_th.join();
        WARC_ECHO_RT_THIS_OK << "Disengaged from control pipework. Communication thread shutdown.";
        return 0;
    }

_WARC_PROTECTED:
    std::thread                    _imm_control_th           = {};
    std::atomic_bool               _eligible                 = false;
    struct _IMM_IDXS {
        imm::EARTH_CTRL_PARAMS::SINK    lens_f;
    }                                                       _imm_idxs              = {};

public:
    int                               read_error_timeout_s   = 5;
    barcud_ctrl::dynamic_state_t   dy_st                  = {};

};


} };