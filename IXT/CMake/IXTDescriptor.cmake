cmake_minimum_required( VERSION 3.30.0 )


# Name of IXT library.
set( IXT_LIBRARY_NAME "IXT" )


# IXT components. Every component has a .hpp and, possibly, a .cpp file.
set(
    IXT_COMPONENTS
    "bit_manip" "comms" "concepts" "descriptor" "endec"
    "file_manip" "os" "audio"
)


# IXT externals.
set(
    IXT_EXTERNAL_LIBS
    "-lwinmm"
    "-ld2d1"
    "-lole32"
    "-lComdlg32"
    "-lDwrite"
    "-lwindowscodecs"
    "-lopengl32"
    "-lglu32"
    "-lgdi32"
)


# IXTDescriptor has been included.
set( IXT_INCLUDED TRUE )
