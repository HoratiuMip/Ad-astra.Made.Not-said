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
        Q_vol_down: GPIO_NUM_26,
        Q_fans:     GPIO_NUM_19
    }
};

dwmq::HEAT_SINK HeatSink{ {} };

dwmq::HYPERVISOR Hypervisor{
    dwmq::HYPERVISOR::PIN_MAP{
        I_toggle_fans: GPIO_NUM_4
    },
    &IrRemote,
    &RelayGrid,
    &HeatSink
};


void setup( void ) {
    vTaskDelay( 2000 );
    vTaskPrioritySet( NULL, dwmq::TaskPriority_Current );
    dwmq::Mirun.init();
    Hypervisor.init();
}

void loop( void ) {
    vTaskDelay( 3200 );
}

