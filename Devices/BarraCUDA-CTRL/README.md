# `Barra`CUDA-CTRL
The "How does it hold together?" makeshift joystick controller, with only one question yet to be answered: "Does it feel better or worse than it looks?". Try not to drop the chonky battery on your toes.
> ![barracuda-artistic-2](https://github.com/user-attachments/assets/70c06b52-5cce-49a4-8159-9010b21ce0b9)

Dedicated to his impossible love, by its founder, the hopeless romantic, https://github.com/nyjucu.

## Manual
Quick power on.
> Power the controller by connecting the micro-USB port to a power supply, such as a computer, battery, or charger socket adapter.

> After the controller power on procedure, the status led shall begin blinking blue. Now search on your device in the bluetooth table for "`BarraCUDA-CTRL`" and pair with the controller.

> When the status led stop blinking and stays blue, the connection is successful.

> All set! Now you may either use the controller for an application, or build you own one! You may write your own driver for the controller's `BARK` protocol, or use the `IXN` engine's one, found [here](https://github.com/HoratiuMip/Ad-astra.Made.Not-said/blob/main/IXN/Include/IXN/Device/barracuda-ctrl-nln-driver.hpp).

Status LED.
> | Pattern | Description |
> | :------ | :---------- |
> | `blink quick red` | The controller has successfully set the I/O pins and will continue the initialization procedure. |
> | `blink pulse red` | A protocol breach occured. The controller shall reset the bluetooth connection. |
> | `blink red slow` | The controller has entered a dead state. A hard reset is required. |
> | `blink quick green` | The controller initialized successfully. |
> | `blink pulse turquoise` | The controller is in testing mode. |
> | `blink quick turquoise` | The controller acknowledged a test, and will being it. / The acknowledged test is completed. |
> | `blink blue` | The controller is waiting for a bluetooth connection. |
> | `static blue` | The controller is connected to a device. |

Testing. - The controller may be put in testing mode by holding the blue switch while powering on. When the status led begins pulsating turquoise, the controller is in testing mode.
> | Combination | Description |
> | :------ | :---------- |
> | `red switch` | Status LED RGB iteration. |

> To exit testing mode, while the status led is pulsating turquoise, press the blue and green switches.

The tester program developed with the `IXN` engine provides a full interface with the controller, offering the posibility to see the state, and send commands to the controller.

> Source code: https://github.com/HoratiuMip/Ad-astra.Made.Not-said/tree/main/IXN/Ruptures/BarraCUDA-CTRL-tester

> Release:

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

> ![barracuda-layout](https://github.com/user-attachments/assets/c65797ff-83c2-476d-b6dd-b8a89acdfb4b)

### Protocol
The controller uses the `WJP` for data transmission. 




