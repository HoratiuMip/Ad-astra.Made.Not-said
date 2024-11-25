# `[ =Z< )) O* ]` _WARC_ - The *Winter-Arc* project. 
Cool description here.

> I.  [Description](#Description)
> II. [Journal](#Journal)
> III. [Manuals](#Manuals)


## Description
> Satellites of interest:
> - `NOAA-15` | downlink @ 137.620 MHz.
> - `NOAA-18` | downlink @ 137.9125 MHz.
> - `NOAA-19` | downlink @ 137.100 MHz.

> Following notations as `NOAA-x` refer to the above satellites.

## Journal

> I. [Stage-I](#Stage-`I`)
> II. [Stage-II](#Stage-`II`)

### Stage-`I` - Anyone up there?
> Let us receive even the faintest signal, in order to have a strong starting anchor, using the following pipeline:
> ![warc-stage-i-pipe](https://github.com/user-attachments/assets/e3677ac9-e98d-4575-b429-7d97479286e7)

> Hardware: 
> - Dipole antenna tuned for the `NOAA-x` satellites | `~52cm` rod | `120 degrees` between rods.
> - USB hub | `hama`.
> - Analog filter & amplifier | `nooelec SAWbird+ NOAA`.
> - Analog to digital converter & tuner | `RTL-SDR V3`.
> - Computing power | `Laptop`.

> Components assembled & caged by the toughest material known to mankind, Lego:
> ![warc-hard](https://github.com/user-attachments/assets/4f6e0a34-ed91-43ee-b529-0ed289bf17c4)

> Software:
> - Signal recorder | `SDR++`.
> - Recording decoder | `SatDump`.

> Results:

> | `Iteration-1` - a.k.a `The Wooden Pickaxe` - Pass of `NOAA-15` - Bypassed the analog amplifier. | `Iteration-2` - Pass of `NOAA-15` - Analog amplifier in pipeline. |
> |-|-|
> |![avhrr_3_rgb_MSA_(Uncalibrated)](https://github.com/user-attachments/assets/ed0b09d1-7a37-48a5-929b-51cf451e5687)|![avhrr_3_rgb_MCIR_(Uncalibrated)](https://github.com/user-attachments/assets/1e8bba32-5f01-4c1e-b3cf-be9410e26eca)|

> | `Iteration-3` - Pass of `NOAA-18` - Higher definition recording. | `Iteration-4` - Pass of `NOAA-18` - Smarter antenna positioning. |
> |-|-|
> |![avhrr_3_rgb_MSA_(Uncalibrated)](https://github.com/user-attachments/assets/3cb87800-2a46-44d0-82a2-5042a538dfe2)|![avhrr_3_rgb_MSA_(Uncalibrated)](https://github.com/user-attachments/assets/586c27b8-2e04-4950-ad44-fc6904407af7)|

### Stage-`II` - Immersive control.
> Let us use the `IXT` engine to create an interactive software to work with `NOAA-x` and the created hardware equipment.

> - This program targets real-time tracking of `NOAA-x` | computing `NOAA-x` future orbits | on-demand, over-the-internet satellite image capturing.
> ![warc-soft](https://github.com/user-attachments/assets/af779d77-e20a-4efb-b664-691f78f4071b)

## Manuals
> I. [Winter-Arc-Hardware](#Winter-Arc-Hardware)
> II. [Winter-Arc-Software](#Winter-Arc-Software)

### Winter-Arc-Hardware
> - `DO NOT` plug any other equipment into the USB hub ports when `nooelec SAWbird+ NOAA` amplifier is operational. The current draw margin is `50mA`.
> - The equipment features the option to bypass the `nooelec SAWbird+ NOAA` amplifier.
> - Push down the lever to cut the power supply to the acquisition devices.

### Winter-Arc-Software

#### Command line arguments
> `--from-config <arg1>` - specifies the configuration file containing different parameters for the session. `arg1` is the absolute path to the configuration file.

> `--n2yo-api-key <arg1> <arg2>` - specifies how to treat the situation regarding n2yo's server api key. Accepted combinations of `arg1` and `arg2`:
> - `use <XXX>` - where `XXX` is the api key. Sets the current session's api key to `XXX`.
> - `burn <XXX>` - makes a copy of the executable file and burns the api key `XXX` inside it. Current session is terminated.
> - `use ash` - extract the api key from its burnt location inside the executable and set it for the current session.
> - `show ash` - extracts and shows the executable's currently burnt api key.

> `--earth-imm` - launch the immersive earth control module. That is, the graphical component.
