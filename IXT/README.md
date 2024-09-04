# IXT "Nine-tailed" engine.

Godlike masterwork engine approved by punchcard/asm/C goddess Ahri. <br>
Make it work again. <br>

Quick dictionary: <br>
> - "ml4o" - most likely forever only <br>

`|`<br>
`|`<br>
`|`<br>

## Build [ ╠═Σ ].

[ I ] Add the `IXT` directory in your project's `CMakeLists.txt` ( `add_subdirectory( <path_to_ixt_directory> )` ), with the following set:
> - `IXT_FONDLES_TO_BUILD` - Inside `IXT/Fondle` there are several sources to showcase the engine's components capabilities. From a fondle source `fdl-<name>.cpp`, append `name` to `IXT_FONDLES_TO_BUILD` in order to build the executable `fdl-<name>`. <br> <br> Example: `-DIXT_FONDLES_TO_BUILD=comms;audio;surface`. <br>

> - `IXT_PREPROCESSOR_DEFINITIONS` - Preprocessor defines for the engine's build. Some defines are mandatory, i.e. operating system and graphics library. ( see ***Preprocessor definitions*** section for more. ) <br> <br> Example: `-DIXT_PREPROCESSOR_DEFINITIONS=IXT_OS_WINDOWS;IXT_GL_DIRECT_2D1`. <br> 

<br>

[ II ] After `CMake` ran through the `IXT` directory tree, the requested fondles are prepared for build, `IXT_LIB_NAME` is forced in the `CMake` cache, and `IXTLib` is available with publicly linked include directories.

<br>

[ Quick cmds ]

`cmake .. -DIXT_PREPROCESSOR_DEFINITIONS=IXT_OS_WINDOWS;IXT_GL_DIRECT_2D1 -DIXT_FONDLES_TO_BUILD=` <br>

`|`<br>
`|`<br>
`|`<br>

## Progress [ ■■■■ . . . . . . . . ].

### Godlike:

> - Acquire <span style="color:magenta">sac</span>. <br>

### Working on:

> - Renderer. <br>

### High:

> - OS::SigInterceptor handling for all cases. ( some terminates are not intercepted now ) <br>
> - <span style="color:orange">~~Do no base fondle executables on component names, iterate Fondle directory instead.~~</span> <br>
> - Separate Clust2Hookable. <br>
> - <span style="color:orange">~~STOP VPtr< T > ptr | some = move( ptr ) | ptr-> FOR FUCK'S SAKE.~~</span> <br>
> - <span style="color:orange">~~Operator new unretarded overload.~~</span> <br>
> - SurfaceEventSentry from type. <br>

### Medium:

> - During audio sampling, don't make "non-continous" jump from last_amp to current_amp. <br>
> - <span style="color:orange">~~VolatilePtr array specializations.~~</span> <br>
> - Option to choose relative path for fondle default assets. <br>
> - <span style="color:orange">~~LinearSweep2 global dive from render spec.~~</span> | Made `direct_dive` with both tmxs and vecs. <br>

### Low:

> - <span style="color:orange">~~Surface VL/VG/CL/CG out of bounds when resizing window. ( see fondle )~~</span> <br>
> - <span style="color:orange">~~Surface style change invokes resize sequence, kill yourself MS, kindly.~~</span> | Scrapped the idea of surface resizing, all resources would need reconstruction. <br>

### Perhaps:

> - <span style="color:orange">~~Some smart pointer wrappers, more ops and behavs.~~</span> | Made `VPtr< T >`. <br>

`|`<br>
`|`<br>
`|`<br>

## Wording choice [ αßΓπ ].

### General:

> - Anything meant for the engine's internal use begins with the ***underscore*** symbol.

### Defines:

> - Full upper-case, words separated by '**_**'.<br>
> `#define SOME_DEFINE 9`

### Typedefs:

> - For primitives and short usings, the typedef is full lower-case and followed by '**_t**', like in ***stdint.h***.<br>
> `typedef int sig_t`<br>
> - For structures, the typedef follows the ***Structure*** wording.

### Enumerations:

> - The enum name is written in full upper-case, words separated by '**_**'.
> - Each enum constant is precedented by the enum name.<br>
> `enum SIG : int { SIG_SEGFAULT, SIG_FLOAT, ... };`

### Structures:

> - Each word begins with an upper-case letter. The words are not separated.<br>
> `struct SomeStructure`
> - Abbreviations follow the same rules.<br>
> `struct SigInterceptor`<br>
> `class IPPipe`

### Variables:

> - Local variables have full lower-case letters, words separated by '**_**'.
> - The same is used for function arguments.<br>
> `int some_int = 9;`
> - Any protected/private variables have an extra '**_**' at the beginning.<br>
> `protected: int _some_int = 9;`

### Functions: 

> - Following the same rules as the ***Variables*** wording.

`|`<br>
`|`<br>
`|`<br>

## Preprocessor definitions [ ╔╬══█ ].

### Mandatory:

> - Operating system. Currently(ml4o) supporting: <br>
>   - `IXT_WINDOWS_OS` <br>
> - Graphics library. Currently(ml4o) supporting: <br>
>   - `IXT_GL_NONE` <br>
>   - `IXT_GL_DIRECT_2D1` <br>

### Optional:

> - Wether your project shall use only one Surface, thus enabling quicker event routing and some quicker accesses. <br>
>   - `IXT_UNIQUE_SURFACE` <br>


`|`<br>
`|`<br>
`|`<br>


## In C|CXX dictionary [ ¥ÆÖ-Ü£ ].

### render2:
> - `RenderSpec2tmx` - transform matrix. <br>
> - `Sweep2gc` - gradient chain. <br>
> - `Sweep2gcn_t` - gradient chain node. <br>


