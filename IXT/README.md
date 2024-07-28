# IXT "Nine-tailed" engine.

`|`<br>
`|`<br>
`|`<br>

## To do.

### Godlike:

> - Get sac. <br>

### High:

> - OS::SigInterceptor handling for all cases. ( some terminates are not intercepted now ) <br>
> - Do no base fondle executables on component names, iterate Fondle directory instead. <br>
> - Separate CLust2Hookable. <br>

### Medium:

> - PID audio sampling, don't make "non-continous" jump from last_amp to current_amp. <br>

### Low:

> - <br>

### Perhaps:

> - Some smart pointer wrappers, more ops and behavs.<br>

`|`<br>
`|`<br>
`|`<br>

## Wording choice.

### General:

> - Anything meant for the engine's internal use begins with the ***underscore*** symbol.

### Defines:

> - Full upper-case, words separated by '**_**'.<br>
> `#define SOME_DEFINE 9`

### Typedefs:

> - For primitives, the typedef is full lower-case and followed by '**_t**', like in ***stdint.h***.<br>
> `typedef int sig_t`<br>
> - For structures, the typedef follows the ***Structure*** wording.

### Enumerations:

> - The enum name is written in full upper-case, words separated by '**_**'.
> - Each enum constant is precedented by the enum name.<br>
> `enum SIG : int { SIG_MEMORY, SIG_FLOAT, ... };`

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
