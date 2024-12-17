# `[ =Z< )) O* ]` _WARC_ - The *Winter-Arc* project. 
Satellite. Satbe... or... satbe not. Are we some sort of... league of orbiters? It's right above me, isn't it?
Earth images captured from the heavens, on demand, via the baddest immersive program and the sickest circuit rig (which I shall not short & burn).
![warc-1](https://github.com/user-attachments/assets/99dba2e3-141e-4eb7-9a9f-0d4a03fc632a)
Damn.

> I.  [Description](#Description)
> II. [Manuals](#Manuals)
> III. [Journal](#Journal)
> IV. [Hyper-Links](#Hyper-Links)


## Description
> Satellites of interest:
> - `NOAA-15` | NORAD id: `25338` | downlink @ 137.620 MHz.
> - `NOAA-18` | NORAD id: `28654` | downlink @ 137.9125 MHz.
> - `NOAA-19` | NORAD id: `33591` | downlink @ 137.100 MHz.

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
> Using `MinGW` w/ `gcc`/`g++` version `11.3.0`:
> > Clone the `IXT` root folder. `cd` inside it and make a `build` folder.
> > ```console
> > mkdir build
> > ```
> > `cd` inside the `build` folder and run the command:
> > > If you're building with `SSL`:
> > > ```console
> > > cmake .. -G "MinGW Makefiles" -DIXT_OS_WINDOWS=ON -DIXT_GL_OPEN_GL=ON -DIXT_BOOST=ON -DIXT_OPEN_SSL=ON -DIXT_RUPTURES_TO_BUILD=warc -DWARC_INET_TLS=ON
> > > ```
> > > If you're building without `SSL`:
> > > ```console
> > > cmake .. -G "MinGW Makefiles" -DIXT_OS_WINDOWS=ON -DIXT_GL_OPEN_GL=ON -DIXT_BOOST=ON -DIXT_RUPTURES_TO_BUILD=warc -DWARC_INET_TLS=OFF
> > > ```
> > Now,
> > ```console
> > mingw32-make
> > ```
> > If everything is $${\color{green}■■■}$$, launch the program for a quick test:
> > ```console
> > .\Ruptures\warc\warc.exe
> > ```
> > This should initialize the program, prompt the chill user wether to continue, and then terminate independently of the user's choice, since no command line arguments were passed. To make a quick launch of the immersive module, run:
> > ```console
> > .\Ruptures\warc\warc.exe --earth-imm --n2yo-mode past
> > ```

#### Immersion
> Control:
> - `RMB` | hold - enable spin mode - move the mouse to spin around the globe.
> - `SCROLL` - zoom in/out.
> - `C` | toggle - enable cinematic camera. | Alternatively, double click `RMB`.
> - `B` | toggle - highlight the countries.
> - `SPACE` | toggle - highlight satellites and their ~range. | Alternatively, while holding `RMB`, flick the mouse left and right quickly for a couple of times.
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

> I. [The-Sound](#The-Sound)
> II. [The-Immersion](#The-Immersion)
> III. [The-Rig](#The-Rig)

### The-Sound
> Anyone up there? Let us search for even the faintest signal, in order to have a strong starting anchor, using the following pipeline:
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

### The-Immersion
> Roam around the earth, being in full control, Godlike.
> 
> ![warc-rah-1](https://github.com/user-attachments/assets/6ad2812d-5f1c-4a44-b5fa-8a85c3ac811f)
>
> Performace affinity:
> - Dynamic allocations avoided in between frames | Heavy data locality.
> - Cache-line-sharing-free multi-threaded control system | Fine tuning of atomic read/modify/writes.
> - State-machine-like general behaviour | Each frame execution does strictly what it needs to execute.

### The-Rig
> Moment for upgrades, cooling and structural reinforcement.
>
> ![warc-rig](https://github.com/user-attachments/assets/be6a9801-f109-4cf2-bbb6-505f48604592)
>
> Upgrades:
> - Heat sinks mounted with adhesive thermal pads | fan blowing air over them.
> - Storage for ports protections caps.
> - General structure reinforced | Pluging cords in/out does not cause stress on the components.


## Hyper-Links

### Hardware
> - `nooelec SAWbird+ NOAA` | https://www.nooelec.com/store/sdr/sdr-addons/sawbird/sawbird-plus-noaa-308.html
> - `RTL-SDR V3` | https://www.rtl-sdr.com/rtl-sdr-blog-v-3-dongles-user-guide

### Bridge
> - `SRD++` | https://www.sdrpp.org
> - `SatDump` | https://www.satdump.org
> - `N2YO` | https://www.n2yo.com | https://www.n2yo.com/?s=25338|28654|33591

### Software
> - `Galaxy texture` | https://svs.gsfc.nasa.gov/4851
> - `Galaxy model` | From the GOAT | https://github.com/nyjucu
> - `Earth texture` | https://shadedrelief.com/natural3/ne3_data/8192/textures/1_earth_8k.jpg
> - `Earth altitude texture` | https://visibleearth.nasa.gov/images/73934/topography
> - `Earth landmask texture` | https://github.com/SatDump/SatDump/blob/master/resources/maps/landmask.jpg
> - `Earth night lights texture` | https://visibleearth.nasa.gov/images/55167/earths-city-lights
> - `Earth model` | From the GOAT | https://github.com/nyjucu
> - `NOAA textures & model` | https://www.cgtrader.com/free-3d-models/space/spaceship/noaa-15-weather-satellite
