# `Barraun`CUDA
The "How does it hold together?" makeshift joystick controller, with only one question yet to be answered: "Does it feel better or worse than it looks?". Try not to drop the chonky battery on your toes.
> ![barruncuda](https://github.com/user-attachments/assets/30b66ecb-8f77-4ac2-bbce-c9203b24ec85)

Dedicated to his impossible love, by its founder, the hopeless romantic, https://github.com/nyjucu.

Quick demo [here](https://www.youtube.com/shorts/fO5yGtDo8sk).

## Overview
What started as a simple bluetooth controller, now has become a portable electronic bomb-looking multi-tool, including:
> - `Control` - the original purpose of the device, as your good ol' joystick controller.
> - `Tools` - LASER, distance measure, heartbeat listner, compass, surface level, audio frequency generator, and counting.
> - `Environment` - temperature, humidity, pressure, altitude, light intensity, all that jazz.
> - `Games` - if "te plictisești", relax yourself to a game of SNAKE or challange your friend to a match of PONG.

Immersion is a top priority, therefore all the navigation and usage of the controller is accompanied by the reactive light band.

The "all from scratch" frame brought the birth of the protocol used by the controller, [`WJP`](https://github.com/HoratiuMip/Ad-astra.Made.Not-said/tree/main/WJP), with further intention of implementing standardized protocols so the controller may be recognized by other machines without the need of the additional `WJP` drivers.

The controller has a dedicated [tester](https://github.com/HoratiuMip/Ad-astra.Made.Not-said/tree/main/IXN/Ruptures/BarrunCUDA-tester). Fancy.
><p align="center">
>  <img src="https://github.com/user-attachments/assets/8480758c-47e2-4b20-8903-6dc645778c59" />
></p>

## Familiarize with the inputs
As of now, the available mounted input sources are:
> - 2x joysticks, a.k.a `Samantha` and `Rachel` ( those who know ).
> - 4x buttons, a.k.a, from left to right, `Giselle`, `Karina`, `Ningning`, `Winter`.
> - Potentiometer, a.k.a `Tanya`.
> - Speaker volume selector.
> - The `Barrun Button` (c), a.k.a `Xabara`.
> ![barruncuda_inputs_friendly](https://github.com/user-attachments/assets/0a1279f3-b5f5-41b4-bc4f-9606190ec616)

## Navigation
The controller boots up to the `Home` bridge. A "bridge" is a node in the navigation tree. The navigation is done with `Samantha`, the upper-right joystick:
> - Pull left or right the joystick to move horizontally through the current level.
> - Pull up or down(!CAUTION for selecting) the joystick to move vertically surfacing or diving from the current level.
> - Press the joystick to select the bridge. If the joystick is pulled down and the current bridge is a leaf, i.e. diving is not possible, a select is triggered.
> ![barruncuda_navigation_friendly](https://github.com/user-attachments/assets/6a3ab8bf-0817-4046-b77e-cbd9ebffe88c)

Selecting a bridge may result in the controller changing its mode, e.g. begin bluetooth pairing or launch a game. Some bridges do not change the controller's mode when selected, they change their contents, e.g. the temperature bridge changing the units from celsius to fahrenheit and back when selected.

The `Barrun Button`, `Xabara`, is used to jump back in the bridge navigation mode from another mode, i.e. bluetooth control or a game. This button is lit up when it is eligible to be pressed, i.e. the controller is not already in the bridge navigation mode. 

## Manual

### Hardware 
Schematics for electrical wiring, both front and back sides, are available in the `schematic` folder.
> ![barruncude_front_schematic](https://github.com/user-attachments/assets/432d967e-b9bd-4b42-846d-ca121c0be1be)

The underlying breadboard plates present a total of `6` power supply lines, `3` "+" and `3` "-". The top and bottom supply lines are connected to as follows:
> - The "+" lines to the `3.3V` voltage regulator's output.
> - The "-" lines to the `GND`.

As for the two middle supply lines, they are connected as follows:
> - The "+" line to the `SDA` ( data ) of the `I^2C` bus.
> - The "-" line to the `SCL` ( clock ) of the `I^2C` bus.




