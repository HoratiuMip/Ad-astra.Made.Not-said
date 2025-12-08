#include <a113/osp/osp.hpp>

#include <ModbusClientPort.h>
#include <iostream>

int main( int argc, char* argv[] ) {
    // Serial / RTU settings
    Modbus::SerialSettings settings;
    settings.portName = "COM7";    // or COMx on Windows
    settings.baudRate = 115200;
    settings.dataBits = 8;
    settings.parity = Modbus::NoParity;               // 'N' (none), 'E', or 'O'
    settings.stopBits = Modbus::OneStop;
    settings.timeoutFirstByte = 3000;             // optional: in milliseconds
    settings.timeoutInterByte = 1000;

    // Create Modbus client port for RTU
    ModbusClientPort *port = Modbus::createClientPort(Modbus::RTU, &settings, true);
    if (!port) {
        std::cerr << "Failed to create RTU client port\n";
        return 1;
    }

    const uint8_t unitId   = 3;     // slave ID
    const uint16_t offset  = 0;     // starting register offset
    const uint16_t count   = 2;    // number of registers to read
    std::vector<uint16_t> regs(count);

    Modbus::StatusCode status = port->readInputRegisters(unitId, offset, count, regs.data());
    if (Modbus::StatusIsGood(status)) {
        std::cout << "Read registers:\n";
        for (int i = 0; i < count; i++) {
            std::cout << "  Reg[" << (offset + i) << "] = " << regs[i] << "\n";
        }
    } else {
        std::cerr << "Modbus error: " << port->lastErrorText() << "\n";
    }

    delete port;

    return 0x0;
}