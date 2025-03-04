#pragma once
/*
*/

#include <NLN/descriptor.hpp>



namespace _ENGINE_NAMESPACE {



enum FILE_FEIDM_RESULT : DWORD {
    FILE_FEIDM_RESULT_DONE,
    FILE_FEIDM_RESULT_ABORTED,

    FILE_FEIDM_RESULT_ITR_CONTINUE,
    FILE_FEIDM_RESULT_ITR_ABORT,

    _FILE_FEIDM_OP_RESULTFORCE_DWORD = 0x7F'FF'FF'FF
};

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
    static DWORD for_each_in_dir_matching( const char* dir, const char* rgx_c_str, std::function< DWORD( std::string_view ) > op ) {
        std::regex rgx{ rgx_c_str };

        for( auto entry : std::filesystem::directory_iterator{ dir } ) {
            auto file_name    = entry.path().filename().string();
            std::smatch match = {};

            if( !std::regex_search( file_name, match, rgx ) ) continue;

            DWORD result = std::invoke( op, file_name );

            switch( result ) {
                case FILE_FEIDM_RESULT_ITR_CONTINUE: continue;
                case FILE_FEIDM_RESULT_ITR_ABORT:    return FILE_FEIDM_RESULT_ABORTED;
            };
        } 

        return FILE_FEIDM_RESULT_DONE;
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

