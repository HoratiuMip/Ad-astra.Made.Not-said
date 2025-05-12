/*===== IXN Framework - Threads follow - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> 
|
======*/
#include <IXN/Framework/imgui_on_opengl3.hpp>


namespace _ENGINE_NAMESPACE { namespace _ENGINE_FRAMEWORK_NAMESPACE {


class Threads_follow {
public:
    typedef   std::function< void( void* ) >   func_t;

_ENGINE_PROTECTED:
    struct _thread_info_t {
        _thread_info_t() = default;

        _thread_info_t( const char* who, int p_count )
        : who{ who }, p_count{ p_count }
        {}

        std::thread        handle     = {};
        const char*        who        = "?Unknwn";
        int                p_count    = 0;
        std::atomic_int    p_crt      = 0;
        std::atomic_bool   returned   = { false };
    };

_ENGINE_PROTECTED:
    std::list< _thread_info_t >                    _infos;
    std::mutex                                     _infos_mtx;
    std::map< std::thread::id, _thread_info_t* >   _threads;
    std::mutex                                     _threads_mtx;

public:
    void launch( const char* name, int p_count, func_t func, void* arg ) {
        std::unique_lock lock{ _infos_mtx };

        auto* info = &_infos.emplace_back( name, p_count );

        info->handle = std::thread{ [ =, this ] () -> void {
            {
            std::unique_lock lock{ _threads_mtx };
            _threads[ std::this_thread::get_id() ] = info;
            }
            func( arg );
            info->returned.store( true, std::memory_order_seq_cst );
        } };
    }

    void im_at( int at ) {
        std::unique_lock lock{ _threads_mtx };
        _threads[ std::this_thread::get_id() ]->p_crt.store( at );
    }

    void im_next( void ) {
        std::unique_lock lock{ _threads_mtx };
        _threads[ std::this_thread::get_id() ]->p_crt.fetch_add( 1 );
    }

_ENGINE_PROTECTED:
    void _make_info_ui( _thread_info_t* info, float x ) {
        ImGui::SetNextWindowPos( ImVec2{ x, 0 } );
        ImGui::SetNextWindowSize( ImVec2{ 0, 0 } );
        ImGui::Begin( info->who );

        int crt = info->p_crt.load( std::memory_order_relaxed );
        ImGui::Text( "At: %d", crt );

        ImGui::Separator();

        crt *= -1;
        ImGui::VSliderInt( "##", ImVec2{ 100, Env::h(.9) }, &crt, -info->p_count + 1, 0, "" );

        ImGui::Separator();

        if( info->returned.load( std::memory_order_relaxed ) ) 
            ImGui::TextColored( ImVec4{ 1, 0.36, 0, 1 }, "RETURNED" );
        else
            ImGui::TextColored( ImVec4{ 0, 0.86, 1, 1 }, "RUNNING" );

        ImGui::End();
    }

public:
    int main( int argc, char* argv[] ) {
        ImGui_on_OpenGL3 ui;
        ui.params.title = "ixN::Fwk::Thread_follow";
        ui.params.iconify = true;
        ui.params.is_running = true;
        ui.init_hold = false;

        ui.loop = [ & ] ( double elapsed, void* ) -> DWORD {
            std::unique_lock lock{ _infos_mtx };

            float x = -100.0;
            for( auto& info : _infos ) this->_make_info_ui( &info, x += 100.0 );

            return 0;
        };

        return ui.main( argc, argv );
    }

};


}; };