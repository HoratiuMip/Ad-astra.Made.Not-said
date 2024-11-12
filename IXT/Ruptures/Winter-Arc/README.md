# `[ =Z< )) O* ]` *Winter-Arc*  
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

> I. [Stage-I](#Stage-I)

### Stage-`I` - Anyone up there?
> Let us receive even the faintest signal, in order to have a strong starting anchor, using the following pipeline:
![warc-stage-i-pipe](https://github.com/user-attachments/assets/e3677ac9-e98d-4575-b429-7d97479286e7)
> Hardware: 
> - Dipole antenna tuned for the `NOAA-x` satellites | `~52cm` rod | `120 degrees` between rods.
> - Analog filter & amplifier | `nooelec SAWbird NOAA+`.
> - Analog to digital converter & tuner | `RTL-SDR V3`.
> - Computing power | `Laptop`.

> Software:
> - Signal recorder | `SDR++`.
> - Recording decoder | `SatDump`.

> [Iteration-1](#Iteration-1)
> [Iteration-2](#Iteration-3)
> [Iteration-3](#Iteration-3)
> [Iteration-4](#Iteration-4)

#### Iteration-`1`
> a.k.a `The Wooden Pickaxe`. <br>
> Pass of `NOAA-15` | Audio recording. Bypassed the analog amplifier in the pipeline. <br>
> ![avhrr_3_rgb_MSA_(Uncalibrated)](https://github.com/user-attachments/assets/ed0b09d1-7a37-48a5-929b-51cf451e5687)

#### Iteration-`2`
> Pass of `NOAA-15` | Audio recording. Analog amplifier back in the pipeline. <br>
> ![avhrr_3_rgb_MCIR_(Uncalibrated)](https://github.com/user-attachments/assets/1e8bba32-5f01-4c1e-b3cf-be9410e26eca)

#### Iteration-`3`
> Pass of `NOAA-18`| Baseband recording. <br>
> ![avhrr_3_rgb_MSA_(Uncalibrated)](https://github.com/user-attachments/assets/3cb87800-2a46-44d0-82a2-5042a538dfe2)

#### Iteration-`4`
> Pass of `NOAA-18` | Baseband recording. Better antenna positioning, got a stronger signal. <br>
> ![avhrr_3_rgb_MSA_(Uncalibrated)](https://github.com/user-attachments/assets/586c27b8-2e04-4950-ad44-fc6904407af7)


## Manuals
> [Winter-Arc-Software](#Winter-Arc-Software)

### Winter-Arc-Software

#### Command line arguments
> `--n2yo-api-key <arg1> <arg2>` - specifies how to treat the situation regarding n2yo's server api key. Accepted combinations of `arg1` and `arg2`:
> - `use <XXX>` - where `XXX` is the api key. Sets the current session's api key to `XXX`.
> - `burn <XXX>` - makes a copy of the executable file and burns the api key `XXX` inside it.
> - `use ash` - extract the api key from its burnt location inside the executable and set it for the current session.
> - `show ash` - extracts and shows the executable's currently burnt api key.