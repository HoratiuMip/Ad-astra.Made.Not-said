# IXT "Nine-tailed" engine.

Godlike masterwork engine approved by punchcard/asm/C goddess Ahri.<br>
Make it work again.<br>

`|`<br>
`|`<br>
`|`<br>


## Build [ ╠═Σ ].

[ I ] Add the `IXT` directory in your project's `CMakeLists.txt` ( `add_subdirectory( <path_to_ixt_directory> )` ), with the following options ( `*` means _mandatory_ ):
> - `IXT_FONDLES_TO_BUILD` - Inside `IXT/Fondle` there are several sources to showcase the engine's components capabilities. From a fondle source `fdl-<name>.cpp`, append `name` to `IXT_FONDLES_TO_BUILD` in order to build the executable `fdl-<name>`.<br><br> Example: `-DIXT_FONDLES_TO_BUILD=comms;audio;surface`.<br>

> - With the same logic as above - `IXT_LABS_TO_BUILD` and `IXT_RUPTURES_TO_BUILD` are present. The labs are experimenting programs. The ruptures are ready to use programs.

> - *`IXT_OS` - The target OS.<br><br> Example:
`-DIXT_OS=Windows`.<br>

> - *`IXT_GLS` - The graphics libraries to use.<br><br> Example:
`-DIXT_GLS=OpenGL`.<br>

> For both `IXT_OS` and `IXT_GLS`, choosing "_None_" is allowed.

> - `IXT_AVX=<level>` - Enables the AVX pipeline on _level_ bits.<br><br> Example:
`-DIXT_AVX=256`.<br>

<br>

[ II ] After `CMake` ran through the `IXT` directory tree, the requested fondles are prepared for build, `IXT_LIB_NAME` is forced in the `CMake` cache, and `IXT` is available with publicly linked include directories.

<br>

[ Quick cmds ]

`cmake .. -DIXT_GLS=Direct2D1;OpenGL -DIXT_OS=Windows -DIXT_FONDLES_TO_BUILD= -DIXT_RUPTURES_TO_BUILD= -DIXT_LABS_TO_BUILD=`<br>


`|`<br>
`|`<br>
`|`<br>


## Progress [ ■■■■ . . . . . . . . ].

### Godlike:

> - Acquire <span style="color:magenta">sac</span>.<br>

### Working on:

> - Audio AVX.<br>

### High:

> - OS::SigInterceptor handling for all cases. ( some terminates are not intercepted now )<br>
> - <span style="color:orange">~~Do no base fondle executables on component names, iterate Fondle directory instead.~~</span><br>
> - Separate Clust2Hookable.<br>
> - <span style="color:orange">~~STOP VPtr< T > ptr | some = move( ptr ) | ptr-> FOR FUCK'S SAKE.~~</span><br>
> - <span style="color:orange">~~Operator new unretarded overload.~~</span><br>
> - SurfaceEventSentry from type.<br>

### Medium:

> - During audio sampling, don't make "non-continous" jump from last_amp to current_amp.<br>
> - <span style="color:orange">~~VolatilePtr array specializations.~~</span><br>
> - <span style="color:orange">~~Option to choose relative path for fondle default assets.~~</span><br>
> - <span style="color:orange">~~LinearSweep2 global dive from render spec.~~</span> | Made `deep_dive` with both tmxs and vecs.<br>
> - RT comms is currently under no locks. Do we need em chat?<br>
> - <span style="color:orange">~~Audio AVX extension.~~</span><br>
> - Sound/Synth AVX extension and filtering.

### Low:

> - <span style="color:orange">~~Surface VL/VG/CL/CG out of bounds when resizing window. ( see fondle )~~</span><br>
> - <span style="color:orange">~~Surface style change invokes resize sequence, kill yourself MS, kindly.~~</span> | Scrapped the idea of surface resizing, all resources would need reconstruction.<br>

### Perhaps:

> - <span style="color:orange">~~Some smart pointer wrappers, more ops and behavs.~~</span> | Made `VPtr< T >`.<br>
> - <span style="color:orange">~~Use transforms instead of direct dives on render2.~~</span><br>


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

### Exceptions:

> - Functions / Variables meant to wrap some another, follow the another's name.<br>


`|`<br>
`|`<br>
`|`<br>


## Preprocessor definitions [ ╔╬══█ ].

> These are meant to be set via CMake @ build. See build above.

### Mandatory:

> - Operating system. Currently supporting:<br>
>   - `IXT_WINDOWS_OS`<br>
> - Graphics library. Currently supporting:<br>
>   - `IXT_GL_NONE`<br>
>   - `IXT_GL_DIRECT_2D1`<br>
>   - `IXT_GL_OPEN_GL`<br>

### Optional:

> - Wether your project shall use only one Surface, thus enabling quicker event routing and some quicker accesses.<br>
>   - `IXT_UNIQUE_SURFACE`<br>


`|`<br>
`|`<br>
`|`<br>


## Components [ ¥ÆÖ-Ü£ ].

For more in-depth examples or an interactive behaviour showcase, read/build the `fdl-<component>.cpp` files.<br>

### `comms`:

The globally available `comms` structure prints the so called `echo`s on their shallow surface destruction. Let:<br>
```cxx
struct X{
    X( int x, IXT_COMMS_ECHO_ARG ) {
        echo( this, ECHO_LEVEL_INTEL ) << "X";
    } 
};

struct Y{
    Y( int y, IXT_COMMS_ECHO_ARG )
    : x{ y, echo }
    {
        echo( this, ECHO_LEVEL_INTEL ) << "Y";
    }

    X x;
};
```
The moment `Y`'s constructor is called, the macro `IXT_COMMS_ECHO_ARG` provides a default-constructed `echo`. This `echo` is the head of the dive chain. `x{ y, echo }` is such a "dive". Since the `echo` of `X`'s constructor is build from another `echo`(`Y`'s), it will be considered to be at depth one. Should we have more dives, the following `echo`s shall be deeper and deeper.<br><br>

The shallow surface destruction is considered the one of the `echo` with depth zero, i.e. the default-constructed one. During this destruction the contents inserted in the `echo`s are printed by the `comms` structure, each line having its insertion depth symbolized by the count of `\>`.<br><br>

Finally, after constructing a `Y` structure, the output shall be:<br>
```
[ INTEL ]   \>[ <unknown>* ][ <address>* ] -> X
[ INTEL ]   [ <unknown>* ][ <address>* ] -> Y
```
*\<unknown\> - the set name of the structure, irrelevant for this example.<br>
*\<address\> - the address of the structure, effectively, `this`.<br>


### `render2`:

The 2D rendering structures.

#### Dictionary:
> - `RenderSpec2tmx` - transform matrix.<br>
> - `Sweep2gc` - gradient chain.<br>
> - `Sweep2gcn_t` - gradient chain node.<br>


`|`<br>
`|`<br>
`|`<br>


