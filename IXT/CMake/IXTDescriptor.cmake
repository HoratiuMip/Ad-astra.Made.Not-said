cmake_minimum_required( VERSION 3.30.0 )


# Name of IXT library.
set( IXT_LIBRARY_NAME "IXT" )


# IXT components. Every component has a .hpp and, possibly, a .cpp file.
set(
    IXT_COMPONENTS
    "bit_manip" "comms" "concepts" "descriptor" "endec"
    "file_manip" "os"
)

# IXTDescriptor has been included.
set( IXT_INCLUDED TRUE )
