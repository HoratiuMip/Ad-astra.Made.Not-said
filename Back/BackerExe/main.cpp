#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <iostream>

#define TARGET_PATH "target.txt"

int main() {
    std::string target{};
    std::ifstream file{ TARGET_PATH };

    if( !file ) {
        std::cout << '\a';
        std::ofstream{ TARGET_PATH } << "*TARGET PATH HERE*";

        return 0;
    }

    std::getline( file, target );

    std::stringstream sstream{};
    sstream << time( nullptr ) << '_' << target.substr( target.find_last_of( '\\' ) + 1 )  ;

    std::filesystem::copy_file( target, sstream.view() );


    return 0;
}
