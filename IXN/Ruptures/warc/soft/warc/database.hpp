/*===== WARC Database - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> 
|
======*/
#include <warc/common.hpp>
#include <warc/satellite.hpp>

#include <IXN/Framework/imgui_on_opengl3.hpp>

#include <bsoncxx/json.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/gridfs/bucket.hpp>

#include <any>


namespace warc { namespace database {


#define WARC_DATABASE_STR WARC_STR"::DATABASE"
_WARC_IXN_COMPONENT_DESCRIPTOR( WARC_DATABASE_STR );


struct InstalledAntenna {
    bsoncxx::oid   oid;
    std::string    name;
    std::string    rating;
    int            acquisition_count;
};

enum RigEventLevel_ : ixN::UBYTE {
    RigEventLevel_Info,
    RigEventLevel_Ok,
    RigEventLevel_Warning,
    RigEventLevel_Error,
    RigEventLevel_Fatal,

    _RigEventLevel_FORCE_UBYTE = 0xFF
};
struct RigEvent {
    bsoncxx::oid     oid;
    RigEventLevel_   level;
    std::string      description;
    time_t           trigger_ts;
    bsoncxx::oid     source;
    std::string      state;
};

enum CloudheadType_ : ixN::UBYTE {
    /* Banned from any DB access. */
    CloudheadType_Restricted = 0x00,

    /* Standard DB access. */
    CloudheadType_Guest = 0x01,

    /* Logged-in standard DB access. */
    CloudheadType_Viewer = 0x02,

    /* Extended logged-in DB access. */
    CloudheadType_DeepViewer = 0x10,

    /* Complete DB access. */
    CloudheadType_Admin = 0xA0,

    _CloudheadType_FORCE_UBYTE = 0xFF
};
struct Cloudhead {
    bsoncxx::oid     oid;
    std::string      nickname;
    CloudheadType_   type;
    std::string      salt;
    time_t           register_ts;
    time_t           last_log_ts;
    int              image_count;
};

struct Decoding {
    bsoncxx::oid                   oid;
    std::string                    description;
    bsoncxx::oid                   source;
    time_t                         acquire_ts;
    bsoncxx::oid                   invoker;
    std::unique_ptr< ixN::BYTE >   data;
    std::string                    format;
};

struct Satellite {
    bsoncxx::oid    oid;
    sat::NORAD_ID   norad_id;
    double          downlink;          /* [MHz] */
    double          uplink;            /* [MHz] */
    double          beacon;            /* [MHz] */
    int             acquisiton_count;
    double          avg_speed;         /* [km/s] */
    double          avg_alt;           /* [km] */
};

struct Note {
    bsoncxx::oid   oid;
    bsoncxx::oid   source;
    bsoncxx::oid   target;
    std::string    content;
    time_t         release_ts;
};


int main( int argc, char* argv[] );


}; };
