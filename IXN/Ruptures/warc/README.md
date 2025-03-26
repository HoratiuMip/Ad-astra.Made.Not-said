# `[ =Z< )) O* ]` _WARC_ - The *Winter-Arc* project. 
Satellite. Satbe... or... satbe not. Are we some sort of... league of orbiters? It's right above me, isn't it?
Earth images captured from the heavens, on demand, via the baddest immersive program and the sickest circuit rig (which I shall not short & burn).
> ![warc-thumb-2](https://github.com/user-attachments/assets/8427430b-c346-4615-a8f0-297e7836227a)
Damn.

> I.  [Description](#Description)
> II. [Manuals](#Manuals)
> III. [Journal](#Journal)
> IV. [Hyper-Links](#Hyper-Links)


## Description
> ![warc-whole-1](https://github.com/user-attachments/assets/2b84fb3c-ce23-48ce-a540-fcab955bc96b)

> Quick client launch tutorial [here](https://www.youtube.com/watch?v=n78CpjPrs64).

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
> > Clone the `IXN` root folder. `cd` inside it and make a `build` folder.
> > ```console
> > mkdir build
> > ```
> > `cd` inside the `build` folder and run the command:
> > > If you're building with `SSL`:
> > > ```console
> > > cmake .. -G "MinGW Makefiles" -DIXN_OS_WINDOWS=ON -DIXN_GL_OPEN_GL=ON -DIXN_BOOST=ON -DIXN_OPEN_SSL=ON -DIXN_RUPTURES_TO_BUILD=warc -DWARC_INET_TLS=ON
> > > ```
> > > If you're building without `SSL`:
> > > ```console
> > > cmake .. -G "MinGW Makefiles" -DIXN_OS_WINDOWS=ON -DIXN_GL_OPEN_GL=ON -DIXN_BOOST=ON -DIXN_RUPTURES_TO_BUILD=warc -DWARC_INET_TLS=OFF
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
> - `C` | toggle - iterate through camera modes.
> - Double `RMB` | toggle - enable/disable cinematic camera mode.
> - Triple `RMB` | toggle - enable/disable free-spin camera mode.
> - `B` | toggle - highlight the countries. | Alternatively, hold `LMB` on Earth and shake the mouse.
> - `SPACE` | toggle - highlight satellites and their ~range. | Alternatively, hold `RMB` and shake the mouse.
> - `>`/`.` | toggle - iterate through render modes.

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
> | `--root-dir <arg1>` | The root directory to be used. `arg1` is the RELATIVE path to the root directory. | `No` | `No` | `Yes` |
> | `--from-config <arg1>` | The configuration file containing the options for the session. `arg1` is the ABSOLUTE path to the configuration file. | `No` | `No` | `Yes` |
> | `--dev-barracuda-ctrl` | Attempt to connect to the Barracuda Controller before launching the program. | `No` | `Yes` | `Yes` |
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
> | `--earth-imm-lens-fov <arg1>` | The Field of view of the camera, in degrees. | `Yes` | `Yes` | `Yes` |
> | `--earth-imm-shake-decay <arg1>` | How fast the mouse shake effect decays over time. | `Yes` | `Yes` | `Yes` |
> | `--earth-imm-shake-cross-count <arg1>` | The number of mouse left-to-right/right-to-left movements required to be considered a shake. | `Yes` | `Yes` | `Yes` |
> | `--astro-ref-vernal-equinox-ts <arg1>` | Timestamp of the most recent vernal equinox, used to compute sun position. | `Yes` | `Yes` | `Yes` |
> | `--astro-ref-first-january-ts <arg1>` | Timestamp of the most recent first of january, used to compute sun position. | `Yes` | `Yes` | `Yes` |


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
#### Performace affinity:
> - Dynamic allocations avoided in between frames | Heavy data locality.
> - Cache-line-sharing-free multi-threaded control system | Fine tuning of atomic read/modify/writes.
> - State-machine-like general behaviour | Each frame execution does strictly what it needs to execute.
> 
#### Control pipework:
> A Petri-Net-like structure that contains so called "tokens", which are "moved" around the net every frame, invoking different procedures to control the program. Right next follows a more detailed explanation on the camera cinematic mode control ( `CCMC` ), as of 14 JAN 2025.
> 
> ![warc-control-pipework](https://github.com/user-attachments/assets/1d6a6bfa-4f64-4a62-92b4-0d702e2197f1)
> 
> - At program launch the pipework is created by pushing sinks and drains, each with their own procedures and parameters. In the `CCMC`, the sinks have no procedures. The procedures are scattered along the drains.
> - Next, the sinks are connected between one another via the drains, thus creating the pipework. Finally, a token is inserted into the `cin_reset` sink and the pipework is now ready to flow.
> - In the `CCMC`, pressing `C` iterates through all the cinematic modes, thus, this event triggers the `cin_r2r` ( cinematic reset-to-reset ) drain, which invokes to procedure of iteration.
> - For a "SMUS" `CCMC`, double or triple clicking `RMB` sets the cinematic mode to "demo" or "free-spin", respectively. In the pipework, when `RMB` is down, the token moves to the `cin_combo` sink. During this state, two drains may have their condition true: `cin_c2c` on `RMB` down, which counts the number of quick clicks, and `cin_c2r` on time-between-clicks expiration, which decides in which cinematic mode to jump.
> - Creating new control sources is a piece of cake. The `BARRACUDA-Controller` specific module support adds a new control device, without interfering with the already existing control code. For example, pressing the blue switch on the controller triggers the cinematic modes iteration.
> - The big picture is, after the pipework flow has begun, drains' conditions and triggers define the control flow.


### The-Rig
> Moment for upgrades, cooling and structural reinforcement.
>
> ![warc-rig](https://github.com/user-attachments/assets/be6a9801-f109-4cf2-bbb6-505f48604592)
>
#### Upgrades:
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
