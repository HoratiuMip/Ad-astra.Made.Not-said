# `Barra`CUDA-CTRL
The "How does it hold together?" makeshift game controller, an elegant mess. Dedicated to his impossible love, by its founder, the hopeless romantic, https://github.com/nyjucu.

> ![barracuda-artistic](https://github.com/user-attachments/assets/83e3d7ba-a70b-49f7-8e5f-e265cb55fb17)

## Manual-quick
> Power the controller by connecting the micro-USB port to a power supply, such as a computer, battery, or charger socket adapter.

> After the controller powered on, search on your computer in the bluetooth table for "`BarraCUDA`" and pair with the controller.

> All set! Now you may either use the controller for an application, or build you own one! You may write your own driver for the controller's protocol, or use the `IXT` engine's one, found [here](https://github.com/HoratiuMip/Ad-astra.Made.Not-said/blob/main/IXT/Include/IXT/SpecMod/barracuda-ctrl-driver.hpp).

## Manual-depth
In-depth manual of the whole controller system, covering both hardware and software components. <br>
> Sketches made using the tool `UMLet`, https://www.umlet.com. 

### Layout
Legend
> | Marking | Description |
> | :------ | :---------- |
> | `Dashed line` | Common supply lines used by multiple actuators/transducers. |
> | `Solid line` | I/O pins connections. The line colors are the same as the wires on the controller. |
> | `Circle` | Connection between wire and suplpy line. |

> ![barracuda-layout](https://github.com/user-attachments/assets/204c6742-14a8-43ac-bdce-a970585e460a)

> The supply lines are modelled for logical connection presentation only, to keep the sketch clean. On the physical controller, the supply lines are wired differently.

> For example, the sketch shows the joysticks' `GND` connected to the bottom `GND` of the `ESP32` ( again, to keep the sketch clean ), although they are physically connected to the top `GND` of the `ESP32`, via the breadboard's power lines.

> The supply lines sketch is shown below.

> *supply lines sketch*

### Protocol
> ![baracuda-proto-2](https://github.com/user-attachments/assets/2c2364b4-d19c-48d4-b626-7523f17a0295)

> - `PROTO-SIG` - the first 3`bytes` of the head - contains the sequence 'B' 'A' 'R'. This signature is used to assure the alignment of the read data. <br>
> - `OP` - the next 1`byte`, the code of the operation. Each operation, along with its code, is described below. <br>
> - `PCK-SIZE` - the next 2`bytes` - the size of the transmitted packet, NOT including the size of the head. <br>
> - `RESERVED` - last 2`bytes` of the head - reserved for future use. <br>
