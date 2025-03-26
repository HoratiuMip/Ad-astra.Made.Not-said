/*
*/
#include <IXN/file-manip.hpp>

using namespace ixN;


int main() {
    File::for_each_in_dir_matching( ASSETS_DIR.data(), "[(\\.wav)(\\.bmp)]$", []( std::string_view path ) -> ixN::DWORD {
        std::cout << path << std::endl;
        return 0;
    } );
}