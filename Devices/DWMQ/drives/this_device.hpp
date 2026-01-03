#pragma once

namespace rnk {


class SelectableDev {
public:
    struct init_args_t {
        int   select_expire_ms   = 10000;
    };

public:
    SelectableDev( void ) = default;

    SelectableDev( const init_args_t& args_ ) {
        this->init( args_ );
    }

protected:
    TimerHandle_t      _H_select_expire   = NULL;
    std::atomic_bool   _selected          = { false };

public:
    void init( const init_args_t& args_ ) {
        _H_select_expire = xTimerCreate(
            "rnk::SelectableDev::_H_select_expire",
            pdMS_TO_TICKS( args_.select_expire_ms ), 
            false,
            ( void* )this,  
            &SelectableDev::_SISR_select_expire
        );
    }

protected:
    static void _SISR_select_expire( TimerHandle_t timer_ ) {
        ( ( SelectableDev* )pvTimerGetTimerID( timer_ ) )->unselect();
    }

public:
    void short_select() { _selected.store( true ); xTimerReset( _H_select_expire, 0 ); }

    void long_select( void ) { _selected.store( true ); }

    void unselect( void ) { _selected.store( false ); }

    bool is_selected( void ) const { return _selected.load(); }

    void select_expire_in( int expire_ms_ ) { xTimerChangePeriod( _H_select_expire, expire_ms_, portMAX_DELAY ); } 

};


}