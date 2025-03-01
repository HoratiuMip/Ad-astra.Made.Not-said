# `Barra`CUDA-CTRL
The "How does it hold together?" makeshift game controller, an elegant mess. Dedicated to his impossible love, by its founder, the hopeless romantic, https://github.com/nyjucu.

> ![barracuda-artistic](https://github.com/user-attachments/assets/83e3d7ba-a70b-49f7-8e5f-e265cb55fb17)

## Manual quick
Quick power.
> Power the controller by connecting the micro-USB port to a power supply, such as a computer, battery, or charger socket adapter. <br>
> After the controller power on procedure, the status led shall begin blinking blue. Now search on your device in the bluetooth table for "`BarraCUDA`" and pair with the controller.

> All set! Now you may either use the controller for an application, or build you own one! You may write your own driver for the controller's protocol, or use the `IXT` engine's one, found [here](https://github.com/HoratiuMip/Ad-astra.Made.Not-said/blob/main/IXT/Include/IXT/SpecMod/barracuda-ctrl-driver.hpp).

Status LED.
> | Blink pattern | Description |
> | :------ | :---------- |
> | `red quick` | The controller has successfully set the I/O pins and will continue the initialization procedure. |
> | `red slow` | The controller has entered an unrecovarable state. A hard reset is required. |
> | `green quick` | THe controller initialized successfully. |
> | `turquoise` | The controller is beginning the testing mode. At the end of the blinks, a switch scan occurs and depending on the pressed ones, a test shall begin. |
> | `turquoise quick` | The controller acknowledged a test, and will being it soon. / The acknowledged test is completed. |

## Manual depth
In-depth manual of the whole controller system, covering both hardware and software components. <br>
> Sketches made using the tool `UMLet`, https://www.umlet.com. 

### Wire layout
Legend.
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

### The `BARK` protocol 
This is an `ISO/OSI PRESENTATION LAYER ( STACK LAYER 6 )` protocol which describes and assures the correct interpretation of the incoming byte stream.

Each transmitted packet begins with a header.
> ![baracuda-proto-2](https://github.com/user-attachments/assets/2c2364b4-d19c-48d4-b626-7523f17a0295)
> - `PROTO-SIG` - the first `3 bytes` of the head - contains the sequence '`B`' '`A`' '`R`'. This signature is used to assure the alignment of the read data. <br>
> - `OP` - the next `1 byte`, the code of the operation. Each operation, along with its code, is described below. <br>
> - `PCK-SIZE` - the next `2 bytes` - the size of the transmitted packet, NOT including the size of the head. <br>
> - `RESERVED` - last `2 bytes` of the head - reserved for future use. <br>

Operations come in two categories.
> - `EXPECT BACK` - after a packet is transmitted, the sender expects a returning acknowledgement packet.
> - `YOLO` - the sender does not expect a feedback.

#### Operations
`PING` - `EXPECT BACK` - Proceeds to execute the complex exchange: "Hello there." <-> "General Kenobi".



