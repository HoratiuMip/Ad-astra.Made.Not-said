# `Barra`CUDA-CTRL
The "How does it hold together?" makeshift joystick controller, with only one question yet to be answered: "Does it feel better or worse than it looks?". Try not to drop the chonky battery on your toes.

> ![barracuda-artistic](https://github.com/user-attachments/assets/0bfca3d2-d5ec-4444-85bd-04d8d6924fcc)

Dedicated to his impossible love, by its founder, the hopeless romantic, https://github.com/nyjucu.

## Quick manual
Power on.
> Power the controller by connecting the micro-USB port to a power supply, such as a computer, battery, or charger socket adapter.

> After the controller power on procedure, the status led shall begin blinking blue. Now search on your device in the bluetooth table for "`BarraCUDA-CTRL`" and pair with the controller.

> When the status led stop blinking and stays blue, the connection is successful.

> All set! Now you may either use the controller for an application, or build you own one! You may write your own driver for the controller's `BARK` protocol, or use the `IXT` engine's one, found [here](https://github.com/HoratiuMip/Ad-astra.Made.Not-said/blob/main/IXT/Include/IXT/SpecMod/barracuda-ctrl-nln-driver.hpp).

Tester.
> The tester program developed with the `IXT` engine provides a full interface with the controller, offering the posibility to see the state, and send commands to the controller.

> Source code: https://github.com/HoratiuMip/Ad-astra.Made.Not-said/tree/main/IXT/Ruptures/BarraCUDA-CTRL-tester

> Release:

Status LED.
> | Blink pattern | Description |
> | :------ | :---------- |
> | `red quick` | The controller has successfully set the I/O pins and will continue the initialization procedure. |
> | `red slow` | The controller has entered an unrecovarable state. A hard reset is required. |
> | `green quick` | THe controller initialized successfully. |
> | `turquoise` | The controller is beginning the testing mode. At the end of the blinks, a switch scan occurs and depending on the pressed ones, a test shall begin. |
> | `turquoise quick` | The controller acknowledged a test, and will being it soon. / The acknowledged test is completed. |

## Depth manual
In-depth manual of the whole controller system, covering both hardware and software components. <br>
> Sketches made using the tool `UMLet`, https://www.umlet.com. 

### Wire layout
Legend.
> | Marking | Description |
> | :------ | :---------- |
> | `Dashed line` | Common supply lines used by multiple components. |
> | `Solid line` | I/O pins connections. The line colors are the same as the wires on the controller. |
> | `Circle` | Connection between wire and suplpy line. |

> ![barracuda-layout](https://github.com/user-attachments/assets/497adbc0-3c28-4d84-82a4-ac1d76aab4ac)

### The `BARK` protocol 
This is an `ISO/OSI PRESENTATION LAYER ( STACK LAYER 6 )` protocol which describes and assures the correct interpretation of the incoming byte stream.

Each transmitted packet begins with the following `12 byte` header.
> ![barracuda-protocol](https://github.com/user-attachments/assets/9f8ac5d4-d58e-4072-bab2-7ab045a06dc5)

> - `PROTO-SIG` - the first `3 bytes` of the head - contains the sequence '`B`' '`A`' '`R`'. This signature is used to assure the alignment of the read data.
> - `OP` - the next `1 byte` - the code of the operation. Each operation, along with its code, is described below.
> - `SEQUENCE` - the next `4 bytes` - sort of an unique identifier of each transmission. 
> - `PCK-SIZE` - the next `2 bytes` - the size of the transmitted packet, NOT including the size of the head.
> - `RESERVED` - last `2 bytes` of the head - reserved for possible future use.

Quick sample - `PING`.
> ![bark-ping](https://github.com/user-attachments/assets/bf0c129d-9641-4045-ab57-d04fac2ef097)

Operations come in two categories.
> - `WAIT ACK` - after a packet is transmitted, the sender expects a returning acknowledgement packet.
> - `YOLO` - the sender does not expect a feedback.

#### Operations
> `PING` - `WAIT ACK` - Proceeds to execute the complex exchange: "Hello there." <-> "General Kenobi". Never transmitted from the controller.

> `SET`/`GET` - `WAIT ACK` - Exchange of parameters between the controller and the endpoint. The first bytes after a `SET`/`GET` header represent a NULL-terminated string identifying the data.
> - `SET` - The bytes following the NULL-terminated identifying string represent the value of the data to be set. The receiver must respond with either an `ACK` or a `NAK`.
> - `GET` - The receiver must respond with either an `ACK`, followed by the value of the requested data, or a `NAK`.

> `DYNAMIC` - `YOLO` - This operation arrives in packets when the endpoint informed the controller to send its state as fast as possible. The endpoint guarantees in this case that it is able to process the packets quick enough.



