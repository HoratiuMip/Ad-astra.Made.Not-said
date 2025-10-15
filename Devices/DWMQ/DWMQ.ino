/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */


#include <drives/dwmq.hpp>


dwmq::IR_REMOTE IrRemote{ 
    dwmq::IR_REMOTE::PIN_MAP{
        I_signal: GPIO_NUM_15
    }
};

dwmq::RELAY_GRID RelayGrid{ 
    dwmq::RELAY_GRID::PIN_MAP{
        Q_power:    GPIO_NUM_25,
        Q_blue:     GPIO_NUM_27,
        Q_vol_up:   GPIO_NUM_33,
        Q_vol_down: GPIO_NUM_26
    }
};

dwmq::HYPERVISOR Hypervisor{
    dwmq::HYPERVISOR::PIN_MAP{
    },
    &IrRemote,
    &RelayGrid
};

void setup( void ) {
    dwmq::Miruna.init();
    Hypervisor.init();
}

void loop( void ) {
    vTaskDelay( 1000 );
}

