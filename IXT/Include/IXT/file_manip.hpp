#pragma once
/*
*/

#include <IXT/descriptor.hpp>



namespace _ENGINE_NAMESPACE {



class File {
public:
    static std::string dir_of( std::string_view path ) {
        return path.substr( 0, path.find_last_of( "/\\" ) ).data();
    }

    static std::string name_of( std::string_view path ) {
        return path.substr( path.find_last_of( "/\\" ) + 1, path.size() - 1 ).data();
    }

public:
    static size_t byte_count( std::ifstream& file ) {
        size_t crt = file.tellg();

        file.seekg( 0, std::ios_base::end );

        size_t bc = file.tellg();

        file.seekg( crt, std::ios_base::beg );

        return bc;
    }

    static size_t byte_count( std::string_view path ) {
        std::ifstream file( path.data(), std::ios_base::binary );

        return byte_count( file );
    }

public:
    static std::string browse( std::string_view title ) {
        char path[ MAX_PATH ];

        OPENFILENAME hf;

        std::fill_n( path, sizeof( path ), 0 );
        std::fill_n( reinterpret_cast< char* >( &hf ), sizeof( hf ), 0 );

        hf.lStructSize = sizeof( hf );
        hf.hwndOwner   = GetFocus();
        hf.lpstrFile   = path;
        hf.nMaxFile    = MAX_PATH;
        hf.lpstrTitle  = title.data();
        hf.Flags       = OFN_EXPLORER | OFN_NOCHANGEDIR;

        GetOpenFileName( &hf );

        return path;
    }

    static std::string save( std::string_view title ) {
        char path[ MAX_PATH ];

        OPENFILENAME hf;

        std::fill_n( path, sizeof( path ), 0 );
        std::fill_n( reinterpret_cast< char* >( &hf ), sizeof( hf ), 0 );

        hf.lStructSize = sizeof( hf );
        hf.hwndOwner   = GetFocus();
        hf.lpstrFile   = path;
        hf.nMaxFile    = MAX_PATH;
        hf.lpstrTitle  = title.data();
        hf.Flags       = OFN_EXPLORER | OFN_NOCHANGEDIR;

        GetSaveFileName( &hf );

        return path;
    }

public:
    template< typename Itr >
    static std::optional< ptrdiff_t > next_idx(
        std::ifstream& file, std::string& str,
        Itr begin, Itr end
    ) {
        if( !( file >> str ) ) return {};

        return std::distance(
            begin,
            std::find_if( begin, end, [ &str ] ( const decltype( *begin )& entry ) -> bool {
                return str == entry;
            } )
        );
    }

    template< typename Itr >
    static void auto_nav(
        std::ifstream& file,
        Itr begin, Itr end,
        std::function< void( ptrdiff_t, std::string& ) > func
    ) {
        std::string str = {};

        for(
            auto idx = next_idx( file, str, begin, end );
            idx.has_value();
            idx = next_idx( file, str, begin, end )
        ) {
            std::invoke( func, idx.value(), str );
        }
    }

};




};

