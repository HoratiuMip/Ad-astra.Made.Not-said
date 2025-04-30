/*===== WARC Database - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> 
|
======*/
#include <warc-db/collections.hpp>
#include <IXN/Framework/imgui_on_opengl3.hpp>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>


ixN::DWORD gui_loop( double elapsed ) {
	return 0;
}


int main( int argc, char* argv[] ) {
	ixN::Fwk::ImGui_on_OpenGL3 fwk;
    fwk.params.title      = "WARC Database";
    fwk.params.iconify    = true;
    fwk.params.is_running = true;
    fwk.loop              = &gui_loop;
    
	std::thread fwk_th{ [ & ] () -> void { fwk.main( argc, argv ); } };
    fwk.init_complete.wait( false );

    try {
        mongocxx::instance inst{};
        const auto uri = mongocxx::uri{ "mongodb://localhost:27017" };
        
        mongocxx::client client{ uri };
        auto db = client[ "admin" ];

        for( const auto& db : client.list_database_names() ) {
            ixN::comms( ixN::EchoLevel_Info ) << "Database: " << db;
        }

    } catch( const std::exception& exc ) {
		ixN::comms( ixN::EchoLevel_Error ) << exc.what();
    }

	if( fwk_th.joinable() ) fwk_th.join();
    return 0;
}