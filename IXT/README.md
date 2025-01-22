# IXT "Nine-tailed" engine.
Smooth operator of anything.

> I. [Build](#Build) <br>
> II. [Components-Overview](#Components-Overview) <br>
> III. [Specific-Modules-Overview](#Specific-Modules-Overview) <br>

## Build
> Add the `IXT` directory in your `CMake` script, configuring the following: <br>( `*`/`?` - mandatory / optional )
> - [ `STRING LIST`, `?` ] - `IXT_FONDLES_TO_BUILD` - [ `<name>` from `fld_<name>.cpp` files ] - build the respective demonstration programs.
> - [ `STRING LIST`, `?` ] - `IXT_RUPTURES_TO_BUILD` - [ directory name of `Rupture` ] - include the respective directories in the `IXT` `CMake` script.
> - [ `OPTION`, `*` ] - `IXT_OS_NONE` | `IXT_OS_WINDOWS` - [ `ON` | `OFF` ] - select the target operating system.
> - [ `OPTION`, `*` ] - `IXT_GL_NONE` | `IXT_GL_DIRECT_2D1` | `IXT_GL_OPEN_GL` - [ `ON` | `OFF` ] - select the target(s) graphic library.
> - [ `NUMBER`, `?` ] - `IXT_AVX` - [ `256` | `512` ] - enable `AVX` usage on several engine components.
> - [ `OPTION`, `?` ] - `IXT_OPEN_SSL` - [ `ON` | `OFF` ] - include the `OpenSSL` directory. <br>
> After the engine's `CMake` script is ran, The "`IXT`" library is available for link, with public included directories.

## Components-Overview

### Memory handling
> - `HYPER_VECTOR` - automatic pointer for slick memory usage, supporting both single type and array, reference counting, move semantics, allocations with/without constructing, reallocation, quick pointer reassignment ( through the "`vector`"/"point to" function ), and soft/hard address link.
> - `BLOCK_DIFFUSER` - N/A

### Logger
> - `Echo` - structure used for recording logs.
> - `Comms` - structure used for outputting recorded logs. The engine provides a global instance of a `Comms` structure, `comms`.

### OS
> - `SigInterceptor` - push callbacks to be executed in case the CPU/OS rises exceptions, such as paging faults, interrupts or illegal instructions.

### Audio
> - `WaveMeta`, `Wave` - bases for creating audio sampling structures.
> - `Audio` - audio playback device - native - `AVX` accelerated.
> - `Sound`, `Synth` - audio sampling stuctures, one sampling from discrete arrays, and the other from continous functions, respectively.

### Geometry
> - `Deg`, `Rad` - angle conversions.
> - `Vec2`, `Crd2`, `Ray2`, `Clust2` - vector, coordinate, ray, cluster ( shape ).

### Graphics
> - `Surface` - window handler - native | `GLFW`.
> - `Renderer2`, `Viewport2`, `SldSweep2`, `LnrSweep2`, `RdlSweep2`, `Sprite2` - 2D graphics structures - `Direct2D`.
> - `RenderCluster3`, `Render3`, `Shader3`, `ShaderPipe3`, `Uniform3`, `Lens3`, `Mesh3` - 3D graphics structures - `OpenGL`.

### Tempo
> - `Ticker` - time measuring device.

### Endec
> - `Wav` - encoder/decoder for `WAV` format.
> - `Bmp` - encoder/decoder for `BMP` format.

### Misc
> - `File` - tools for working with files.
> - `Bytes` - structure for manipulating bytes.
> - `Env` - obtain different information about the environment, such as screen dimenstions, current process. 

## Specific-Modules-Overview

### Input
> - `BarracudaController` - support for the `Barracuda` bluetooth controller.
