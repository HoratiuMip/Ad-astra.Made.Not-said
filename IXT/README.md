## Wording choice.

### General:

> - Anything meant for the engine's internal use begins with the ***underscore*** symbol.

### Defines:

> - Full upper-case, words separated by '**_**'.<br>
> `#define SOME_DEFINE 9`

### Typedefs:

> - For primitives, the typedef is full lower-case and followed by '**_t**', like in ***stdint.h***.<br>
> `typedef int sig_t`<br>
> - For structures, the typedef follows the structure wording( see below ).

### Enumerations:

> - The enum name is written in full upper-case. Words separated by '**_**'.
> - Each enum constant is precedented by the enum name.<br>
> `enum SIG {`<br>
> `SIG_MEMORY`<br>
> `...`<br>
> `}`

### Structures:

> - Each word begins with an upper-case letter. The words are not separated.<br>
> `struct SomeStructure`
> - Abbreviations follow the same rules.<br>
> `struct SigInterceptor`<br>
> `class IPPipe`