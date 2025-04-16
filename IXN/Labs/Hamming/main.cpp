#include <IXN/Framework/imgui_on_opengl3.hpp>


#define PO2( n ) ( (n) == 1 || ( (n) & ( (n) - 1 ) ) == 0 )


typedef std::vector< bool > vec_t;


void imgui_print_vec( const vec_t& vec ) {
    for( bool b : vec ) {
        ImGui::Text( "%d", ( int )b ); ImGui::SameLine(); 
    }
    ImGui::Text( "" );
}


int required_ctrl_bits( const vec_t& vec ) {
    int len = vec.size();
    for( int bit = sizeof( int ) * 8 - 2; bit >= 0; --bit ) {
        if( ( ( len >> bit ) & 1 ) != 1 ) continue;

        return bit + 1;
    }
    return 0;
}

vec_t message_string2vec( const std::string& str ) {
    vec_t res; res.reserve( str.length() );
    for( auto c = str.cbegin(); c != str.cend(); ++c ) res.push_back( *c == '1' );
    return res;
}

vec_t get_ctrl_bits( const vec_t& msg ) {
    int m = msg.size();
    int k = required_ctrl_bits( msg );
    int n = m + k;

    vec_t res; res.assign( k, 0 );

    int H_row[ n ];
    const auto gen_H_row = [ &H_row, &n ] ( int idx ) -> void {
        bool bit  = ( idx == 0 );
        int count = 1;

        for( int c = 0; c < n; ++c ) {
            H_row[ c ] = bit;
            ++count;
            if( count % ( 1 << idx ) == 0 ) bit ^= 1;
        }
    }; 

    for( int r = 0; r < k; ++r ) {
        gen_H_row( r );

        int sum  = 0;
        int idxM = 0;
        for( int idxH = 0; idxH < n; ++idxH ) {
            if( PO2( idxH + 1 ) ) continue;
            sum ^= H_row[ idxH ] * msg[ idxM++ ];
        }

        res[ r ] = sum;
    }

    return res;
}


static struct {
    vec_t         out       = {};
    std::string   _msg_str  = "";

    void loop( void ) {
        ImGui::SetNextWindowSize( { 256, 256 } );
        ImGui::Begin( "Encoder" );

        ImGui::InputTextWithHint( "Message", "bits...", &_msg_str, ImGuiInputTextFlags_None );
            
        vec_t msg = message_string2vec( _msg_str );

        ImGui::SeparatorText( "Info" );
        
        int m = msg.size();
        int k = required_ctrl_bits( msg );
        int n = m + k;

        ImGui::Text( "Message length: (%d)", m );
        ImGui::Text( "Required control bits: (%d)", k );
        ImGui::Text( "Encoded message length: (%d)", n );

        ImGui::SeparatorText( "Encoding" );

        ImGui::Text( "Control bits:" );

        vec_t ctrl_bits = get_ctrl_bits( msg );
        for( auto b : ctrl_bits ) {
            ImGui::SameLine(); ImGui::Text( "[%d]", ( int )b );
        }

        ImGui::SeparatorText( "Encoded message" );

        out.clear(); out.reserve( n );

        int idxC = 0;
        int idxM = 0;
        for( int at = 1; at <= n; ++at ) {
            bool b = out.emplace_back( PO2( at ) ? ctrl_bits[ idxC++ ] : msg[ idxM++ ] );
            ImGui::Text( "%d", ( int )b ); ImGui::SameLine(); 
        }

        ImGui::End();
    }

} encoder;


static struct {
    vec_t*   in;
    vec_t    out;

    void loop( void ) {
        ImGui::SetNextWindowSize( { 256, 256 } );
        ImGui::Begin( "Tamperer" );

        ImGui::SeparatorText( "Intercepted" );
        imgui_print_vec( *in );

        ImGui::SeparatorText( "Tamper" );

        static bool tmps[ 256 ];
        for( int idx = 0; idx < in->size(); ++idx ) {
            ImGui::Checkbox( ( std::string{ "##tmp" } + std::to_string( idx ) ).c_str(), tmps + idx ); ImGui::SameLine();
        }

        ImGui::Text( "" ); ImGui::SeparatorText( "Forwarded" );

        out.clear(); out.assign( in->size(), 0 );
        for( int idx = 0; idx < in->size(); ++idx )
            out[ idx ] = tmps[ idx ] ? ( ( *in )[ idx ] ^ 1 ) : ( *in )[ idx ];

        imgui_print_vec( out );

        ImGui::End();
    }
} tamperer;


int main( int argc, char* argv[] ) {
    ixN::Fwk::ImGui_on_OpenGL3 fwk;

    fwk.params.title = "Hamming";
    fwk.params.iconify = true;

    fwk.loop = [ & ] ( double elapsed ) -> ixN::DWORD {
        encoder.loop();
        tamperer.in = &encoder.out;
        tamperer.loop();
        return 0;
    };
    
    fwk.params.is_running = true;
    return fwk.main( argc, argv );
}