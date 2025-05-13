/*===== WARC Database - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> 
|
======*/
#include <warc/database.hpp>



namespace warc { namespace database {



class ImmAgent {
public:
    virtual int imm_loop( double elapsed ) = 0;
};

class Imm {
public:
    Imm( int argc, char* argv[] ) 
    : _rtc{ "rtc", 0.0 }
    {
    /* === User interface framework init === */
        _fwk.params.arg        = this;
        _fwk.params.title      = "WARC Database";
        _fwk.params.width      = ixN::Env::w(.9);
        _fwk.params.height     = ixN::Env::h(.9);
        _fwk.params.iconify    = false;
        _fwk.params.is_running = true;
        
        _fwk.loop = &Imm::loop;

        _fwk_th = std::thread{ [ argc, argv, this ] () -> void { _fwk.main( argc, argv ); } };
        _fwk.Wait_init_complete();

    /* === Background rendering === */
        glfwSetCursorPosCallback( _fwk.render.handle(), Imm::_CursorPosCallback );
        glfwSetScrollCallback( _fwk.render.handle(), Imm::_ScrollCallback );

        new ( &_gal_mesh ) ixN::Mesh3{ WARC_IMM_ROOT_DIR/"galaxy/", "galaxy", ixN::MESH3_FLAG_MAKE_PIPES };
        _gal_mesh.pipe->pull( _fwk.uniforms.view, _fwk.uniforms.proj, _rtc );
        _gal_mesh.model = glm::scale( glm::mat4{ 1.0 }, glm::vec3{ 200.0 } ) * _gal_mesh.model.get();
        _gal_mesh.model.uplink_b();

        new ( &_dft_sat_mesh ) ixN::Mesh3{ WARC_IMM_ROOT_DIR/"sat_noaa/", "sat_noaa", ixN::MESH3_FLAG_MAKE_PIPES };
        _dft_sat_mesh.pipe->pull( _fwk.uniforms.view, _fwk.uniforms.proj, _rtc );
        _dft_sat_mesh.model = glm::scale( glm::mat4{ 1.0 }, glm::vec3{ 1.0 / 13.224987 } ) * _dft_sat_mesh.model.get();
        _dft_sat_mesh.model = glm::rotate( glm::mat4{ 1.0 }, glm::radians( 90.0f ), glm::vec3{ 1, 0, 0 } ) * glm::rotate( glm::mat4{ 1.0 }, glm::radians( 180.0f ), glm::vec3{ 1, 1, 0 } ) * _dft_sat_mesh.model.get();
        _dft_sat_mesh.model.uplink_b();
        
        _fwk.uniforms.view.uplink_bv( _fwk.lens.view() );
        _fwk.uniforms.proj.uplink_bv( glm::perspective( glm::radians( 70.0f ), _fwk.render.aspect(), 0.1f, 1000.0f ) );

    /* === Release framework === */
        _fwk.Release_init_hold();
    }

    ~Imm() {
        if( _fwk_th.joinable() ) _fwk_th.join();
    }

_WARC_PROTECTED:
    ixN::Fwk::ImGui_on_OpenGL3   _fwk             = {};
    std::thread                  _fwk_th          = {};

    ixN::Mesh3                   _gal_mesh        = {};
    ixN::Mesh3                   _dft_sat_mesh    = {};
    ixN::Uniform3< glm::f32 >    _rtc;

    ixN::SPtr< ixN::Mesh3 >      _client_mesh     = nullptr;

    std::list< ImmAgent* >       _agents          = {};
    std::recursive_mutex         _agents_mtx      = {};

public:
    bool                         splash_dft_mdl   = false;

public:
    inline bool is_running( void ) {
        return _fwk.params.is_running;
    }

public:
    static ixN::DWORD loop( double elapsed, void* arg ) {
        Imm* that = ( Imm* )arg;

        that->_fwk.render.downlink_face_culling();
        that->_rtc.uplink_bv( that->_rtc.get() + elapsed );
        that->_gal_mesh.splash();
        that->_fwk.render.uplink_face_culling();

        if( that->splash_dft_mdl ) that->_dft_sat_mesh.splash();

        std::unique_lock lock{ that->_agents_mtx };
        for( auto agent = that->_agents.begin(); agent != that->_agents.end(); ) {
            if( ( *agent )->imm_loop( elapsed ) != 0 ) {
                agent = that->_agents.erase( agent );
            } else {
                ++agent;
            }
        }

        return 0;
    }

public:
    void push( ImmAgent* agent ) {
        std::unique_lock lock{ _agents_mtx };

        _agents.push_back( agent );        
    }

    void pop( ImmAgent* agent ) {
        std::unique_lock lock{ _agents_mtx };

        auto entry = _agents.begin();
        for( ; entry != _agents.end(); ++entry ) if( *entry == agent ) break;
        if( entry != _agents.end() ) _agents.erase( entry );
    }

_WARC_PROTECTED:
    static void _CursorPosCallback( GLFWwindow* glfwnd, double x, double y ) {
        ImGui_ImplGlfw_CursorPosCallback( glfwnd, x, y );

        Imm* that = ( Imm* )glfwGetWindowUserPointer( glfwnd );

        static bool        fs     = true;
        static ixN::Vec2   grab   = { 0.0, 0.0 };

        if( glfwGetMouseButton( that->_fwk.render.handle(), GLFW_MOUSE_BUTTON_RIGHT ) != GLFW_PRESS ) { 
            fs = true; 
            while( ShowCursor( true ) < 0 );
            return; 
        }

        if( fs == true ) { 
            grab = ixN::SurfPtr::env_v();
            fs = false; 
            return; 
        } 

        ixN::Vec2 pos = ixN::SurfPtr::env_v();
        glm::vec2 thetas = glm::vec2{ glm::radians( grab.x - pos.x ), glm::radians( grab.y - pos.y ) } * 0.02;
        that->_fwk.uniforms.view.uplink_bv( that->_fwk.lens.spin_ul( thetas * that->_fwk.lens.l2t(), { -82.0, 82.0 } ).view() );

        ixN::SurfPtr::env_to( grab );
        while( ShowCursor( false ) >= 0 );
    }

    static void _ScrollCallback( GLFWwindow* glfwnd, double x, double y ) {
        ImGui_ImplGlfw_ScrollCallback( glfwnd, x, y );

        Imm* that = ( Imm* )glfwGetWindowUserPointer( glfwnd );

        if( glfwGetMouseButton( that->_fwk.render.handle(), GLFW_MOUSE_BUTTON_RIGHT ) != GLFW_PRESS ) return;

        that->_fwk.uniforms.view.uplink_bv( that->_fwk.lens.zoom( ( y > 0 ? 0.03 : -0.03 ) * that->_fwk.lens.l2t(), { 0.6, 7.6 } ).view() );
    }
};



enum HQAgentType_ : int {
    HQAgentType_Collection,
    HQAgentType_Resource,

    HQAgentType_COUNT
};

class HQAgent : public ImmAgent {
public:
    friend class HQ;

_WARC_PROTECTED:
    HQAgent( 
        const std::string& title, 
        const std::string& description, 
        HQAgentType_       type 
    )
    : _title{ title }, _description{ description }
    {
        _register[ ( int )type ].push_back( this );

        _th = std::thread( &HQAgent::proc, this );
    }

    ~HQAgent() {
        if( _th.joinable() ) _th.join();
    }

_WARC_PROTECTED:
    inline static std::list< HQAgent* >   _register[ HQAgentType_COUNT ]   = {};

    std::thread                           _th                              = {};

    std::string                           _title                           = "";
    std::string                           _description                     = "";
    bool                                  _imm_open                        = false;

public:
    virtual int proc( void ) = 0;

};

class HQAgentCollection : public HQAgent {
public:
    HQAgentCollection( 
        const std::string&   title,
        const std::string&   description, 
        mongocxx::collection collection 
    )
    : HQAgent{ title, description, HQAgentType_Collection },
      _collection{ std::move( collection ) }
    {}

_WARC_PROTECTED:
    mongocxx::collection   _collection   = {};

};

class HQAgentResource : public HQAgent {
public:
    HQAgentResource( 
        const std::string&   title,
        const std::string&   description
    )
    : HQAgent{ title, description, HQAgentType_Resource }
    {}

_WARC_PROTECTED:
    

};

class HQ : public ImmAgent {
public:
    inline static HQ*   that   = nullptr;

public:
    HQ( const std::string& address, Imm* imm ) 
    : _uri{ address.c_str() },
      _client{ _uri },
      database{ _client.database( "Rig" ) },
      _imm{ imm }
    {
        that = this;
    }

    ~HQ() {
        that = nullptr;
    }

_WARC_PROTECTED:
    mongocxx::instance   _instance   = {};
    mongocxx::uri        _uri        = {};
    mongocxx::client     _client     = {};

    Imm*                 _imm        = nullptr;

public:
    ImVec4               theme       = { 0.0, 1.0, 0.86, 1.0 };

    mongocxx::database   database    = {};

public:
    inline bool is_running( void ) {
        return _imm->is_running();
    }

public:
    int main( void ) {
        WARC_ASSERT_RT( _imm != nullptr, "NULL immersion pointer.", -1, -1 ); 

        while( _imm->is_running() ) {
            std::this_thread::sleep_for( std::chrono::seconds{ 3 } );
        }
        return 0;
    }

public:
    virtual int imm_loop( double elapsed ) override {
        ImGui::Begin( "HQ", nullptr, ImGuiWindowFlags_None );

        ImGui::SeparatorText( "Details" );
        ImGui::SetWindowFontScale( 2.0 );
        ImGui::TextColored( theme, "- WARC Database - v1 -" );
        ImGui::SetWindowFontScale( 1.0 );
        ImGui::NewLine();
        ImGui::Separator();

        static const char* agents_strs[ ( int )HQAgentType_COUNT ] = {
            "Collection agents", "Resource agents"
        };

        for( int type = 0; type < ( int )HQAgentType_COUNT; ++type ) {
            ImGui::SeparatorText( agents_strs[ type ] );

            for( auto& agent : HQAgent::_register[ type ] ) {
                if( ImGui::Checkbox( agent->_title.c_str(), &agent->_imm_open ) && agent->_imm_open ) {
                    _imm->push( agent );
                }
                ImGui::SetItemTooltip( agent->_description.c_str() );
            }

            ImGui::NewLine();
            ImGui::Separator();
        }

        ImGui::SeparatorText( "Settings" );

        ImGui::Checkbox( "Render default 3D model.", &_imm->splash_dft_mdl );
        ImGui::SetItemTooltip( "Draw NOAA-15's 3D model besides the client mesh, if any. This, if mainly for debugging purposes." );
        ImGui::NewLine();

        ImGui::Separator();
        ImGui::ColorPicker3( "Theme", ( float* )&theme, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha );

        ImGui::End();
        return 0;
    }

};



class HQAgentSatellites : public HQAgentCollection {
public:
    HQAgentSatellites( mongocxx::database& database )
    : HQAgentCollection{ "Satellites", "Satellites which the Rig may listen to. Includes everything about them, from name, to measurements, to 3D models.", database[ "Satellites" ] }
    {}

public:
    typedef   std::deque< bsoncxx::document::value >   DocsContainer;

_WARC_PROTECTED:
    ixN::SPtr< DocsContainer >                               _docs          = nullptr;

    std::atomic< void(*)( HQAgentSatellites*, std::any ) >   _sig_refresh   = nullptr;
    std::any                                                 _sig_arg       = 0;

    std::string                                              _flt_alias     = "";
    std::pair< int, int >                                    _flt_years     = { 1957, 2025 };
    bool                                                     _flt_years_r   = true;

    bool                                                     _read_only     = true;
    ixN::Ticker                                              _drop_tick     = {};
    inline static constexpr float                            _DROP_DELAY    = 3.6;
    bool                                                     _dropping      = false;

public:
    void sig_proc( void( *proc )( HQAgentSatellites*, std::any ), std::any arg = 0 ) {
        _sig_arg = std::move( arg );
        _sig_refresh.store( proc, std::memory_order_release );
        _sig_refresh.notify_one();
    }

    virtual int proc( void ) override {
        for(; HQ::that->is_running() ;) {
            _sig_refresh.wait( nullptr );

            try {
                ( _sig_refresh.exchange( nullptr, std::memory_order_release ) )( this, std::move( _sig_arg ) );
            } catch( std::exception& exc ) {
                WARC_ECHO_RT_ERROR << exc.what();
            }
            
            _sig_refresh.notify_all();
        }

        return 0;
    }

_WARC_PROTECTED:
    static void _proc_query_filters( HQAgentSatellites* that, [[maybe_unused]]std::any ) {
        using bsoncxx::builder::basic::make_document; 
        using bsoncxx::builder::basic::make_array;
        using bsoncxx::builder::basic::kvp;

        ixN::comms( ixN::EchoLevel_Pending, "Refreshing collection entries." );

        ixN::SPtr< DocsContainer > docs{ new DocsContainer{} };

        mongocxx::cursor cursor = that->_collection.find( 
            make_document( 
                kvp( "alias", make_document( kvp( "$regex", that->_flt_alias.c_str() ) ) ),
                kvp( "$expr", make_document( kvp( "$and", make_array(
                    make_document( kvp( "$gte", make_array( make_document( kvp( "$year", "$launch" ) ), that->_flt_years.first ) ) ),
                    make_document( kvp( "$lte", make_array( make_document( kvp( "$year", "$launch" ) ), that->_flt_years_r ? that->_flt_years.second : that->_flt_years.first ) ) )
                ) ) ) )
            ) 
        );

        for( auto&& doc : cursor ) {
            docs->emplace_back( std::move( doc ) );
        }

        ixN::comms( ixN::EchoLevel_Ok, "Refresh collection entries." );

        that->_docs = std::move( docs );
    }

    static void _proc_drop_doc( HQAgentSatellites* that, std::any arg ) {
        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::finalize;

        auto oid = std::any_cast< bsoncxx::oid >( arg );

        ixN::comms( ixN::EchoLevel_Pending, "Dropping [{}].", oid.to_string() );

        that->_collection.delete_one( ( document{} << "_id" << oid << finalize ).view() );

        ixN::comms( ixN::EchoLevel_Pending, "Dropped [{}].", oid.to_string()  );

        that->sig_proc( &_proc_query_filters );
    }

public:
    virtual int imm_loop( double elapsed ) override {
        ImGui::Begin( _title.c_str(), &_imm_open, ImGuiWindowFlags_None );
        if( _imm_open == false ) { ImGui::End(); return -1; }

        ImGui::Separator();
        ImGui::SetWindowFontScale( 1.6 );
        ImGui::TextColored( HQ::that->theme, "- Satellites -" );
        ImGui::SetWindowFontScale( 1.0 );
        ImGui::NewLine(); ImGui::Separator();
        ImGui::TextWrapped( "Registry of all orbiters the Rig may interact with." );
        ImGui::NewLine(); ImGui::Separator();

        ImGui::SeparatorText( "Filters" );
        ImGui::SetNextItemWidth( 300.0 ); 
        ImGui::BulletText( "Alias" ); ImGui::SameLine(); 
        ImGui::InputTextWithHint( "##Alias", "Regex here...", &_flt_alias, ImGuiInputTextFlags_EnterReturnsTrue );
        ImGui::Separator();
        ImGui::BulletText( "Year(s)" ); ImGui::SameLine(); 
        ImGui::SetNextItemWidth( 100.0 ); ImGui::DragInt( "##After", &_flt_years.first, 0.2, 1957, _flt_years_r ? _flt_years.second - 1 : 2024 ); ImGui::SameLine();
        if( _flt_years_r ) {
            ImGui::SetNextItemWidth( 100.0 ); ImGui::DragInt( "##Before", &_flt_years.second, 0.2, _flt_years.first + 1, 2025 ); ImGui::SameLine();
        }
        if( ImGui::Checkbox( "Range", &_flt_years_r ) && _flt_years_r ) {
            _flt_years.second = _flt_years.first + 1;
        }

        ImGui::NewLine(); ImGui::Separator();
        if( ImGui::Button( "Query" ) ) {
            this->sig_proc( &_proc_query_filters );
        }
        if( 
            ( 
                _flt_alias.empty() 
                &&
                ( _flt_years_r && abs( _flt_years.first - _flt_years.second ) >= 10 )
            ) 
            && ImGui::IsItemHovered( ImGuiHoveredFlags_DelayNone ) && ImGui::BeginTooltip() 
        ) {
            ImGui::TextColored( ImVec4{ 1.0, 0.0, 0.0, 1.0 }, "CAUTION" );
            ImGui::Separator();
            ImGui::Text( "Querying these filters may retrieve a lot of the documents in the collection. Proceed?" );
            ImGui::EndTooltip();
        }
        
        ImGui::SameLine();
        if( ImGui::Button( "+" ) ) {

        }

        ImGui::SameLine(); ImGui::Checkbox( "Read only", &_read_only );

        ImGui::NewLine(); ImGui::Separator();

        ixN::SPtr< DocsContainer > docs = _docs; if( !docs || docs->empty() ) goto l_not_ready; 
    {   
        ImGui::NewLine(); ImGui::Separator();

        if( ImGui::BeginTable( "Satellites", 6, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY ) ) {
            ImGui::TableSetupColumn( "NORAD id", ImGuiTableColumnFlags_AngledHeader, 0.0, 0 );
            ImGui::TableSetupColumn( "Alias",    ImGuiTableColumnFlags_AngledHeader, 0.0, 1 );
            ImGui::TableSetupColumn( "Downlink", ImGuiTableColumnFlags_AngledHeader, 0.0, 2 );
            ImGui::TableSetupColumn( "Status",   ImGuiTableColumnFlags_AngledHeader, 0.0, 3 );
            ImGui::TableSetupColumn( "Launch",   ImGuiTableColumnFlags_AngledHeader, 0.0, 4 );
            ImGui::TableSetupColumn( "Action",   ImGuiTableColumnFlags_AngledHeader, 0.0, 5 );

            ImGui::TableSetupScrollFreeze( 0, 1 );

            ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, IM_COL32( 56.0, 56.0, 56.0, 255 ) );
            ImGui::TableAngledHeadersRow();
           
            static double t = 0.0; t += elapsed;

            int sat_count = 0;

            for( auto& doc : *docs ) {
                ++sat_count;

                ImGui::TableNextRow( ImGuiTableRowFlags_None );

                int gs = ( sin( 2.0*t + PI / 4.0 * sat_count ) + 1.0 ) / 2.0 * 56.0;
                ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, IM_COL32( gs, gs, gs, 255 ) );               

                ImGui::PushID( sat_count );

                ImGui::TableSetColumnIndex( 0 );
                ImGui::Text( " %d ", ( int )doc[ "norad_id"].get_int32() );
                        
                ImGui::TableSetColumnIndex( 1 );
                ImGui::Text( " %s ", doc[ "alias" ].get_string().value.data() );

                ImGui::TableSetColumnIndex( 2 );
                ImGui::Text( " %.4f [MHz] ", ( float )doc[ "downlink" ].get_double() );

                ImGui::TableSetColumnIndex( 3 ); {
                    switch( doc[ "status" ].get_int32() ) {
                        case sat::STATUS_UNKNOWN:  ImGui::TextColored( ImVec4{ 1.0, 0.0, 0.0, 1.0 }, "UNKNOWN" ); break;
                        case sat::STATUS_IN_ORBIT: ImGui::TextColored( ImVec4{ 0.0, 1.0, 0.0, 1.0 }, "IN ORBIT" ); break;
                        case sat::STATUS_DECAYED:  ImGui::TextColored( ImVec4{ 1.0, 0.5, 0.0, 1.0 }, "DECAYED" ); break;
                    }
                }

                ImGui::TableSetColumnIndex( 4 );
                std::chrono::system_clock::time_point launch_tp{ std::chrono::milliseconds{ doc[ "launch" ].get_date().to_int64() } };
                std::time_t launch_tt = std::chrono::system_clock::to_time_t( launch_tp );
                std::tm* launch_date = std::gmtime( &launch_tt );
                ImGui::Text( " %d ", 1900 + launch_date->tm_year );
                
                ImGui::TableSetColumnIndex( 5 ); 
                if( !_read_only && ImGui::Button( "Edit" ) ) {

                }

                ImGui::PushStyleColor( ImGuiCol_Button,        ImVec4{ 0.2, 0.2, 0.2, 1.0 } );
                ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.4, 0.4, 0.4, 1.0 } );
                ImGui::PushStyleColor( ImGuiCol_ButtonActive,  ImVec4{ 0.0, 0.0, 0.0, 1.0 } );
                ImGui::SameLine();
                if( ImGui::Button( "3D" ) ) {

                }
                ImGui::PopStyleColor( 3 );

                ImGui::PushStyleColor( ImGuiCol_Button,        ImVec4{ 0.5, 0.0, 0.0, 1.0 } );
                ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.8, 0.0, 0.0, 1.0 } );
                ImGui::PushStyleColor( ImGuiCol_ButtonActive,  ImVec4{ 0.3, 0.0, 0.0, 1.0 } );
                ImGui::SameLine();
                if( !_read_only ) {
                    if( ImGui::Button( "Drop" ) ) { _dropping = false; }
                    if( !_dropping && ImGui::IsItemClicked() ) { _dropping = true; _drop_tick.lap(); }
                    
                    if( _dropping && ImGui::IsItemHovered( ImGuiHoveredFlags_None ) ) {
                        float elapsed = _drop_tick.peek_lap();

                        ImGui::BeginTooltip();
                        ImGui::ProgressBar( elapsed / _DROP_DELAY, ImVec2{ 86, 0 }, "Dropping..." );
                        ImGui::EndTooltip();

                        if( elapsed >= _DROP_DELAY ) {
                            _dropping = false;
                            this->sig_proc( &_proc_drop_doc, doc[ "_id" ].get_oid().value );
                        }
                    }
                }
                ImGui::PopStyleColor( 3 );

                ImGui::PopID();
            }

            ImGui::EndTable();

            ImGui::Text( "Item count: %d", sat_count );
            ImGui::NewLine(); ImGui::Separator();
        }

        goto l_end;
    
    } 
    l_not_ready:
        ImGui::TextColored( ImVec4{ 1.0, 0.5, 0.0, 1.0 }, "No items ready or no match for current filters." );

    l_end:
        ImGui::End();
        return 0;
    }

};

class HQAgentImages : public HQAgentCollection {
public:
    HQAgentImages( mongocxx::database& database )
    : HQAgentCollection{ "Images", "Compilation of all the images ever taken by the Rig.", database[ "Images" ] }
    {}

public:
    virtual int proc( void ) override {
        return 0;
    }

public:
    virtual int imm_loop( double elapsed ) override {
        return 0;
    }

};

class HQAgentEvents : public HQAgentCollection {
public:
    HQAgentEvents( mongocxx::database& database )
    : HQAgentCollection{ "Events", "Thing that happened while the Rig was running, cause either from inside or outside.", database[ "Events" ] }
    {}

public:
    virtual int proc( void ) override {
        return 0;
    }

public:
    virtual int imm_loop( double elapsed ) override {
        return 0;
    }

};

class HQAgentCloudheads : public HQAgentCollection {
public:
    HQAgentCloudheads( mongocxx::database& database )
    : HQAgentCollection{ "Cloudheads", "The persons with their head in the clouds. Here is logged every client.", database[ "Cloudheads" ] }
    {}

public:
    virtual int proc( void ) override {
        return 0;
    }

public:
    virtual int imm_loop( double elapsed ) override {
        return 0;
    }

};

class HQAgentNotes : public HQAgentCollection {
public:
    HQAgentNotes( mongocxx::database& database )
    : HQAgentCollection{ "Notes", "Notes left by cloudheads regarding absolutely anything within the Rig: images, satellites, performances.", database[ "Notes" ] }
    {}

public:
    virtual int proc( void ) override {
        return 0;
    }

public:
    virtual int imm_loop( double elapsed ) override {
        return 0;
    }

};



class HQAgentGridFS : public HQAgentResource {
public:
    HQAgentGridFS( mongocxx::database& database )
    : HQAgentResource{ "GridFS", "Manipulate binary files stored in the database." },
      _bucket{ database.gridfs_bucket() }
    {}

_WARC_PROTECTED:
    mongocxx::gridfs::bucket   _bucket   = {};

public:
    virtual int proc( void ) override {
        return 0;
    }

public:
    virtual int imm_loop( double elapsed ) override {
        ImGui::Begin( _title.c_str(), &_imm_open, ImGuiWindowFlags_None );
        if( _imm_open == false ) { ImGui::End(); return -1; }

        ImGui::Separator();
        ImGui::SetWindowFontScale( 1.6 );
        ImGui::TextColored( HQ::that->theme, "- Grid file system -" );
        ImGui::SetWindowFontScale( 1.0 );
        ImGui::NewLine(); ImGui::Separator();
        ImGui::TextWrapped( "Upload/download big binary files to/from the database, such as heatmaps or 3D models." );
        ImGui::NewLine(); ImGui::Separator();

        if( ImGui::BeginTabBar( "##", ImGuiTabBarFlags_Reorderable ) ) {
            if( ImGui::BeginTabItem( "Upload" ) ) {
                
                ImGui::EndTabItem();
            }
            if( ImGui::BeginTabItem( "Download" ) )
            {
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
        return 0;
    }

};



int main( int argc, char* argv[] ) {  
    Imm imm{ argc, argv };
    HQ hq{ "mongodb://localhost:27017", &imm };

    imm.push( &hq );

    HQAgentSatellites hqa_sat{ hq.database };
    HQAgentImages     hqa_img{ hq.database };
    HQAgentEvents     hqa_evt{ hq.database };
    HQAgentCloudheads hqa_cdh{ hq.database };
    HQAgentNotes      hqa_not{ hq.database };

    HQAgentGridFS     hqa_gfs{ hq.database };

    return hq.main();
}

}; };