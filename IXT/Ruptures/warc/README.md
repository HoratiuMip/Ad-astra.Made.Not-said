# `[ =Z< )) O* ]` _WARC_ - The *Winter-Arc* project. 
Cool picture here.
![warc-1](https://github.com/user-attachments/assets/99dba2e3-141e-4eb7-9a9f-0d4a03fc632a)
Cool description here.
Damn.

> I.  [Description](#Description)
> II. [Manuals](#Manuals)
> III. [Journal](#Journal)


## Description
> Satellites of interest:
> - `NOAA-15` | downlink @ 137.620 MHz.
> - `NOAA-18` | downlink @ 137.9125 MHz.
> - `NOAA-19` | downlink @ 137.100 MHz.

> Following notations as `NOAA-x` refer to the above satellites.


## Manuals
> I. [WARC-Hardware](#WARC-Hardware)
> II. [WARC-Software](#WARC-Software)

### WARC-Hardware

#### Power supply
> Components:
> - Battery | `5V`.

#### Signal acquisition system
> Components:
> - USB hub | `hama`.
> - Analog filter & amplifier | `nooelec SAWbird+ NOAA`.
> - Analog to digital converter & tuner | `RTL-SDR V3`.

> Antenna wiring:
> - Connect the feed from the antenna into either the SMA-f port of the amplifier, or, to bypass the effects of the filter and amplifier, into the SMA-f port of the converter.

> ICE:
> - Push down the lever to cut the power supply to the acquisition system.

> Caution:
> - When bypassing the amplifier, make sure its power supply is disconnected, so it does not induce any floating noise into the signal pipe.
> - `DO NOT` plug any other equipment into the USB hub ports when the amplifier is operational. The current draw margin is `~50mA`.

#### Cooling system
> Components:
> - Fan | `AVC`.
> - Boost converter | home-made | `~10V`.
> - Controller | `Arduino UNO` | maintains the output of the boost converter @ `~10V`.

> Fan RPM tuning:
> - Potentiometer control.

> ICE:
> - The controller will attempt to keep the voltage of the boost converter @ `~10V`. Shall the voltage exceed `15V`, the controller shall disable the boost converter until a system reset, since this would mean a control loop general failure. The disabled boost converter is indicated by a red LED. Disabling the boost converter does not mean a total cut of power to the fan ( it will be supplied @ the battery voltage ).

### WARC-Software

#### Quick build
> Using `MinGW`:
> > Clone the `IXT` root folder. `cd` inside it and make a `build` folder.
> > ```console
> > chill_individual/../IXT> mkdir build
> > ```
> > `cd` inside the `build` folder and run the command:
> > > If you're building with `SSL`:
> > > ```console
> > > chill_individual/../IXT/build> cmake .. -G "MinGW Makefiles" -DIXT_OS_WINDOWS=ON -DIXT_GL_OPEN_GL=ON -DIXT_BOOST=ON -DIXT_OPEN_SSL=ON -DIXT_RUPTURES_TO_BUILD=warc -DWARC_INET_TLS=ON
> > > ```
> > > If you're building without `SSL`:
> > > ```console
> > > chill_individual/../IXT/build> cmake .. -G "MinGW Makefiles" -DIXT_OS_WINDOWS=ON -DIXT_GL_OPEN_GL=ON -DIXT_BOOST=ON -DIXT_RUPTURES_TO_BUILD=warc -DWARC_INET_TLS=OFF
> > > ```
> > Now,
> > ```console
> > chill_individual/../IXT/build> make
> > ```
> > If everything is green, then is okay.

#### Immersion
> Control:
> - `RMB` | hold - enable spin mode - move the mouse to spin around the globe.
> - `RMB` | quick double click, toggle - enable cinematic camera.
> - `SCROLL` - zoom in/out.
> - `SPACE` | toggle - highlight satellites and their ~range.
> - `<`/`,` - enable wireframe rendering.
> - `>`/`.` - enable full rendering.

#### Options
> The good old command line arguments. There are three ways to configure the session options:
> - From the command line arguments @ program launch, `CMDL-L`.
> - From a configuration file, `CFILE`.
> - From the console command line, `CMDL-RT`.

> Caution:
> - Every json field in the configuration file must be a string, since the command line arguments parser is used here as well.
> - Options are parsed sequentially, so every later appearance of an option will overwrite the previous one.

> Example of `CMDL-L`:
> ```console
> .\warc.exe --from-config .\config.json --earth-imm --n2yo-mode past`
> ```

> Example of `CFILE`:
> ```json
> {
>    "--n2yo-ip": "A.B.C.D",
>    "--n2yo-api-key": "use XXX",
>    "--n2yo-bulk-count": "600",
>    "--earth-imm-lens-sens": "0.6"
> }
> ```

> Table:
> | Option | Description | CMDL-RT | CFILE | CMDL-L |
> |:-------|:------------|:-------:|:-----:|:------:|
> | `--from-config <arg1>` | The configuration file containing the options for the session. `arg1` is the absolute path to the configuration file. | `No` | `No` | `Yes` |
> | `--n2yo-mode <arg1>` | How to retrieve satellite data from the `N2YO` server. | `Yes` | `Yes` | `Yes` |
> > Accepted `arg1`, Satellites positions are...:
> > - `rand` - ...random. Server connection `IS NOT` made in this mode.
> > - `past` - ...from a past data file. Server connection `IS NOT` made in this mode.
> > - `real` - ...real-time. Server connection `IS` made in this mode.

> | Option | Description | CMDL-RT | CFILE | CMDL-L |
> |:-------|:------------|:-------:|:-----:|:------:|
> | `--n2yo-api-key <arg1> <arg2>` | How to load the n2yo's server api key. | `Yes` | `Yes` | `Yes` |
> > Accepted combinations of `arg1` and `arg2`:
> > - `use <XXX>` - where `XXX` is the api key. Sets the current session's api key to `XXX`.
> > - `burn <XXX>` - makes a copy of the executable file and burns the api key `XXX` inside it. Current session is terminated.
> > - `use ash` - extract the api key from its burnt location inside the executable and set it for the current session.
> > - `show ash` - extracts and shows the executable's currently burnt api key.

> | Option | Description | CMDL-RT | CFILE | CMDL-L |
> |:-------|:------------|:-------:|:-----:|:------:|
> | `--n2yo-ip <arg1>` | The ip of the `N2YO` server. | `Yes` | `Yes` | `Yes` |
> | `--n2yo-bulk-count <arg1>` | How many orbit positions ( how many seconds of orbit ) to request from the `N2YO` server, per satellite update. | `Yes` | `Yes` | `Yes` |
> | `--earth-imm` | Launch the immersive earth control module. That is, the graphical component. | `No` | `No` | `Yes` |
> | `--earth-imm-lens-sens <arg1>` | The sensitivity of the camera. | `Yes` | `Yes` | `Yes` |


## Journal

> I. [Stage-I](#Stage-I)
> II. [Stage-II](#Stage-II)

### Stage-I - Anyone up there?
> Let us receive even the faintest signal, in order to have a strong starting anchor, using the following pipeline:
> ![warc-stage-i-pipe](https://github.com/user-attachments/assets/e3677ac9-e98d-4575-b429-7d97479286e7)

> Hardware: 
> - Dipole antenna tuned for the `NOAA-x` satellites | `~52cm` rod | `120 degrees` between rods.
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

### Stage-II - Immersive control.
> Let us use the `IXT` engine to create an interactive software to work with `NOAA-x` and the created hardware equipment.

> - This program targets real-time tracking of `NOAA-x` | computing `NOAA-x` future orbits | on-demand, over-the-internet satellite image capturing.
> ![warc-soft-v3-2](https://github.com/user-attachments/assets/2f672418-77de-47bc-9dc7-6b3350f907d3)
> ![warc-soft-v3](https://github.com/user-attachments/assets/9ae7ee73-706b-4aaa-a8e3-ccd0f29db944)

