#pragma once
/*
*/

#include <IXN/descriptor.hpp>
#include <IXN/bit_utils.hpp>
#include <IXN/comms.hpp>
#include <IXN/concepts.hpp>

namespace _ENGINE_NAMESPACE {



#pragma region Spatial



class Vec2;
class Crd2;
class Ray2;
class Clust2;



enum HEADING {
    HEADING_NORTH = 0, 
    HEADING_EAST, 
    HEADING_SOUTH, 
    HEADING_WEST
};

enum SYSTEM {
    SYSTEM_LOCAL = 0, 
    SYSTEM_GLOBAL
};



class Deg {
public:
    static ggfloat_t pull( ggfloat_t theta ) {
        return theta * ( 180.0 / PI );
    }

    static void push( ggfloat_t& theta ) {
        theta *= ( 180.0 / PI );
    }
};

class Rad {
public:
    static ggfloat_t pull( ggfloat_t theta ) {
        return theta * ( PI / 180.0 );
    }

    static void push( ggfloat_t& theta ) {
        theta *= ( PI / 180.0 );
    }
};



#pragma region D2



template< typename T > concept is_vec2_base = std::is_base_of_v< Vec2, T >;

class Vec2 {
public:
    static ggfloat_t norm_sq(
        ggfloat_t x, ggfloat_t y
    ) {
        return x*x + y*y;
    }

    static ggfloat_t norm(
        ggfloat_t x, ggfloat_t y
    ) {
        return sqrt( norm_sq( x, y ) );
    }

    static ggfloat_t dot_product(
        ggfloat_t x1, ggfloat_t y1,
        ggfloat_t x2, ggfloat_t y2
    ) {
        return x1*x2 + y1*y2;
    }

    static ggfloat_t cross_product(
        ggfloat_t x1, ggfloat_t y1,
        ggfloat_t x2, ggfloat_t y2
    ) {
        return x1*y2 - x2*y1;
    }

    static void project(
        ggfloat_t x1, ggfloat_t y1,
        ggfloat_t x2, ggfloat_t y2,
        ggfloat_t* xi, ggfloat_t* yi
    ) {
        ggfloat_t dot = dot_product( x1, y1, x2, y2 );
        ggfloat_t n2  = norm( x2, y2 );

        n2  *= n2;
        dot /= n2;

        *xi = x2 * dot;
        *yi = y2 * dot;
    }

public:
    Vec2() = default;

    Vec2( ggfloat_t x, ggfloat_t y )
    : x{ x }, y{ y }
    {}

    Vec2( ggfloat_t x )
    : Vec2{ x, x }
    {}

public:
    ggfloat_t   x   = 0.0;
    ggfloat_t   y   = 0.0;

public:
    ggfloat_t dot( Vec2 other ) const {
        return dot_product( this->x, this->y, other.x, other.y );
    }

public:
    ggfloat_t mag_sq() const {
        return norm_sq( x, y );
    }

    ggfloat_t mag() const {
        return norm( x, y );
    }

    ggfloat_t angel() const {
        return Deg::pull( atan2( y, x ) );
    }

public:
    ggfloat_t dist_sq( Vec2 other ) const {
        return norm_sq( other.x - x, other.y - y );
    }

    ggfloat_t dist( Vec2 other ) const {
        return norm( other.x - x, other.y - y );
    }

public:
    Vec2 respect( Vec2 other ) const {
        return { x - other.x, y - other.y };
    }

    Vec2 operator () ( Vec2 other ) const {
        return this->respect( other );
    }

public:
    Vec2& normalize() {
        return *this /= this->mag();
    }

    Vec2 normalized() const {
        return Vec2{ *this }.normalize();
    }

    Vec2& absolute() {
        return x = abs( x ), y = abs( y ), *this;
    }

    Vec2 absoluted() const {
        return Vec2{ *this }.absolute();
    }

public:
    Vec2& polar( ggfloat_t angel, ggfloat_t dist ) {
        Rad::push( angel );

        x += cos( angel ) * dist;
        y += sin( angel ) * dist;

        return *this;
    }

    Vec2 polared( ggfloat_t angel, ggfloat_t dist ) const {
        return Vec2{ *this }.polar( angel, dist );
    }

public:
    Vec2& approach( const Vec2 other, ggfloat_t dist ) {
        return this->polar( other.respect( *this ).angel(), dist );
    }

    Vec2 approached( const Vec2 other, ggfloat_t dist ) const {
        return Vec2{ *this }.approach( other, dist );
    }

public:
    Vec2& spin( ggfloat_t theta ) {
        Rad::push( theta );

        ggfloat_t nx = x * cos( theta ) - y * sin( theta );
        y = x * sin( theta ) + y * cos( theta );
        x = nx;

        return *this;
    }

    Vec2& spin( ggfloat_t theta, Vec2 other ) {
        *this = this->respect( other ).spin( theta ) + other;

        return *this;
    }

    Vec2 spinned( ggfloat_t theta ) const {
        return Vec2{ *this }.spin( theta );
    }

    Vec2 spinned( ggfloat_t theta, Vec2 other ) const {
        return Vec2{ *this }.spin( theta, other );
    }

public:
    Vec2& project( Vec2 other ) {
        project( x, y, other.x, other.y, &x, &y );
        return *this;
    }

    Vec2 projected( Vec2 other ) {
        return Vec2{ *this }.project( other );
    }

public:
    bool is_further_than( Vec2 other, HEADING heading ) const {
        switch( heading ) {
            case HEADING_NORTH: return y > other.y;
            case HEADING_EAST:  return x > other.x;
            case HEADING_SOUTH: return y < other.y;
            case HEADING_WEST:  return x < other.x;
        }

        return false;
    }

public:
    Vec2& operator = ( Vec2 other ) {
        x = other.x; y = other.y; return *this;
    }

    Vec2& operator = ( ggfloat_t val ) {
        x = y = val; return *this;
    }

    bool operator == ( Vec2 other ) const {
        return x == other.x && y == other.y;
    }

    Vec2 operator + ( Vec2 other ) const {
        return { x + other.x, y + other.y };
    }

    Vec2 operator - ( Vec2 other ) const {
        return { x - other.x, y - other.y };
    }

    Vec2 operator * ( Vec2 other ) const {
        return { x * other.x, y * other.y };
    }

    Vec2 operator / ( Vec2 other ) const {
        return { x / other.x, y / other.y };
    }

    Vec2 operator + ( ggfloat_t delta ) const {
        return { x + delta, y + delta };
    }

    Vec2 operator - ( ggfloat_t delta ) const {
        return { x - delta, y - delta };
    }

    Vec2 operator * ( ggfloat_t delta ) const {
        return { x * delta, y * delta };
    }

    Vec2 operator / ( ggfloat_t delta ) const {
        return { x / delta, y / delta };
    }

    Vec2 operator >> ( ggfloat_t delta ) const {
        return { x + delta, y };
    }

    Vec2 operator ^ ( ggfloat_t delta ) const {
        return { x, y + delta };
    }

    Vec2& operator += ( Vec2 other ) {
        x += other.x;
        y += other.y;

        return *this;
    }

    Vec2& operator -= ( Vec2 other ) {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    Vec2& operator *= ( Vec2 other ) {
        x *= other.x;
        y *= other.y;

        return *this;
    }

    Vec2& operator /= ( Vec2 other ) {
        x /= other.x;
        y /= other.y;

        return *this;
    }

    Vec2& operator *= ( ggfloat_t delta ) {
        x *= delta;
        y *= delta;

        return *this;
    }

    Vec2& operator /= ( ggfloat_t delta ) {
        x /= delta;
        y /= delta;

        return *this;
    }

    Vec2& operator >>= ( ggfloat_t delta ) {
        x += delta;

        return *this;
    }

    Vec2& operator ^= ( ggfloat_t delta ) {
        y += delta;

        return *this;
    }

    Vec2 operator - () const {
        return ( *this ) * -1.0;
    }

public:
    static Vec2 O() {
        return { 0.0, 0.0 };
    }

};

class Crd2 : public Vec2 {
public:
    using Base = Vec2;

public:
    using Base::Base;

    using Base::operator =;

public:
    Crd2( const Vec2& vec )
    : Vec2{ vec }
    {}

public:
#if defined( _ENGINE_GL_DIRECT_2D1 )
    operator const D2D1_POINT_2F& () const {
        return *reinterpret_cast< const D2D1_POINT_2F* >( this );
    }

    operator D2D1_POINT_2F& () {
        return *reinterpret_cast< D2D1_POINT_2F* >( this );
    }
#endif

};

inline Vec2 pull_normal_axis( Crd2 crd ) {
    return { crd.x - .5_ggf, .5_ggf - crd.y };
}

inline Crd2 pull_normal_axis( Vec2 vec ) {
    return { vec.x + .5_ggf, .5_ggf - vec.y };
}

inline void push_normal_axis( Crd2& crd ) {
    crd = pull_normal_axis( crd );
}

inline void push_normal_axis( Vec2& vec ) {
    vec = pull_normal_axis( vec );
}




template< typename T > concept ggX_result = std::is_same_v< bool, T > || std::is_same_v< Vec2, T >;

class Ray2 {
public:
    static bool intersection_assert( 
        ggfloat_t ox1, ggfloat_t oy1, ggfloat_t vx1, ggfloat_t vy1,
        ggfloat_t ox2, ggfloat_t oy2, ggfloat_t vx2, ggfloat_t vy2
    ) {
        ggfloat_t oovx = ox2 - ox1;
        ggfloat_t oovy = oy2 - oy1;
        ggfloat_t ovvx = oovx + vx2;
        ggfloat_t ovvy = oovy + vy2; 

        ggfloat_t s1 = Vec2::cross_product( vx1, vy1, oovx, oovy );
        ggfloat_t s2 = Vec2::cross_product( vx1, vy1, ovvx, ovvy );

        if( std::signbit( s1 ) == std::signbit( s2 ) ) return false;
        
        oovx = ox1 - ox2;
        oovy = oy1 - oy2;
        ovvx = oovx + vx1;
        ovvy = oovy + vy1;

        s1 = Vec2::cross_product( vx2, vy2, oovx, oovy );
        s2 = Vec2::cross_product( vx2, vy2, ovvx, ovvy );

        return std::signbit( s1 ) != std::signbit( s2 );
    }

    static bool intersection_point(
        ggfloat_t ox1, ggfloat_t oy1, ggfloat_t vx1, ggfloat_t vy1,
        ggfloat_t ox2, ggfloat_t oy2, ggfloat_t vx2, ggfloat_t vy2,
        ggfloat_t* ix, ggfloat_t* iy
    ) {
        ggfloat_t det = vy1*-vx2 + vx1*vy2;

        if( det == .0_ggf ) return false;

        ggfloat_t sx = -( -ox1*vy1 + oy1*vx1 );
        ggfloat_t sy = -( -ox2*vy2 + oy2*vx2 );

        ggfloat_t x = ( sx*-vx2 + vx1*sy ) / det;
        ggfloat_t y = ( vy1*sy - sx*vy2 ) / det;
        
        ggfloat_t nvx = x - ox1;
        ggfloat_t nvy = y - oy1;

        if( Vec2::norm_sq( nvx, nvy ) > Vec2::norm_sq( vx1, vy1 ) || Vec2::dot_product( nvx, nvy, vx1, vy1 ) < 0.0 ) return false;
 
        nvx = x - ox2;
        nvy = y - oy2; 

        if( Vec2::norm_sq( nvx, nvy ) > Vec2::norm_sq( vx2, vy2 ) || Vec2::dot_product( nvx, nvy, vx2, vy2 ) < 0.0 ) return false;

        *ix = x;
        *iy = y;

        return true;
    }

public:
    Ray2() = default;

    Ray2( Vec2 org, Vec2 v )
    : origin{ org }, vec{ v }
    {}

public:
    Vec2   origin   = {};
    Vec2   vec      = {};

public:
    Vec2 drop() const {
        return origin + vec;
    }

    Vec2& operator [] ( bool idx ) {
        return idx ? vec : origin;
    }

public:
    ggfloat_t slope() const {
        return ( this->drop().y - origin.y ) / ( this->drop().x - origin.x );
    }

public:
    template< ggX_result X_T >
    auto X( const Ray2& other ) const {
        if constexpr( std::is_same_v< bool, X_T > ) {
            return intersection_assert( 
                origin.x, origin.y, vec.x, vec.y,
                other.origin.x, other.origin.y, other.vec.x, other.vec.y
            );
        } else if constexpr( std::is_same_v< Vec2, X_T > ) {
            using RetType = std::optional< Vec2 >;

            Vec2 result;

            return intersection_point( 
                origin.x, origin.y, vec.x, vec.y,
                other.origin.x, other.origin.y, other.vec.x, other.vec.y,
                &result.x, &result.y
            ) ? RetType{ result } : RetType{};
        }
    }

};



/* Clust2 FILE FORMAT
0: DWORD: ixt file idx
4: BYTE: metadata
    - bit 0-1: mode:
        -- 0b00: text, pairs of x and y
        -- 0b01: bin, floats( 4 bytes )
        -- 0b10: bin, doubles( 8 bytes )
    - bit 2: org:
        -- 0b0: no origin, only vertices
        -- 0b1: treat first vertex as the origin
    - bit 3-8: unused
5: DWORD: vertex count, including the origin if the <org> bit is set
*/
inline const std::string_view   CLUST2_FILE_FMT_DFT_EXT   = ".clst2";
enum CLUST2_FILE_FMT {
    CLUST2_FILE_FMT_MODE_MSK = 0b00000011,
    CLUST2_FILE_FMT_ORG_MSK  = 0b00000100
};


class Clust2 : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Clust2" );

public:
    typedef   std::pair< Vec2, Vec2 >   Vrtx;

public:
    Clust2() = default;

    Clust2( std::forward_iterator auto first, std::forward_iterator auto last ) {
        _vrtx.reserve( std::distance( first, last ) );

        for( ; first != last; ++first )
            _vrtx.emplace_back( *first, *first );
    }

    Clust2( std::forward_iterator auto first, size_t n )
    : Clust2{ first, first + n }
    {}

    template< typename Cunt >
    requires requires{
        std::begin( Cunt{} );
        std::end( Cunt{} );
    }
    Clust2( Cunt&& cunt )
    : Clust2{ std::begin( cunt ), std::end( cunt ) }
    {}

    Clust2( const Vec2& org, std::forward_iterator auto first, std::forward_iterator auto last )
    : Clust2{ first, last }
    {
        _origin = org;
    }

    Clust2( const Vec2& org, std::forward_iterator auto first, size_t n )
    : Clust2{ org, first, first + n }
    {}

    template< typename Cunt >
    Clust2( const Vec2& org, Cunt&& cunt )
    : Clust2{ org, std::begin( cunt ), std::end( cunt ) }
    {}

    Clust2( std::string_view path, _ENGINE_COMMS_ECHO_ARG ) {
        std::ifstream file{ path.data() };

        if( !file ) {
            echo( this, EchoLevel_Error ) << "Could NOT open file: \"" << path.data() << "\".";
            return;
        }

        struct _Meta {
            _Meta( std::ifstream& file, _ENGINE_COMMS_ECHO_ARG ) {
                file.read( ( char* )&xtfdx, sizeof( xtfdx ) );

                ubyte_t src = file.get();

                mode = src & 0b11;
                org = ( src >> 2 ) & 0b1;

                file.read( ( char* )&count, sizeof( count ) );
            }

            XtFdx   xtfdx = 0;
            dword_t count = 0;
            ubyte_t mode:2;
            ubyte_t org:1; 

        } meta{ file, echo };

        if( meta.xtfdx != FDX_CLUST2 ) 
            echo( this, EchoLevel_Warning ) << "XtFdx of file: \"" << path.data() << "\" does not match this structure's XtFdx.";

        _vrtx.reserve( meta.count );

        switch( meta.mode ) {
            case 0b00: {
                dword_t read_count = 0;

                if( meta.org ) {
                    if( file.eof() ) goto l_end;
                    file >> _origin.x; ++read_count;
                    if( file.eof() ) goto l_end;
                    file >> _origin.y; ++read_count;
                }

l_read_vrtx:
                if( ( read_count >> 1 ) == meta.count || file.eof() ) goto l_end;
                file >> _vrtx.emplace_back().second.x; 
                _vrtx.back().first.x = _vrtx.back().second.x;
                ++read_count;

                if( file.eof() ) goto l_end;
                file >> _vrtx.back().second.y; 
                _vrtx.back().first.y = _vrtx.back().second.y;
                ++read_count;

                goto l_read_vrtx;
l_end:
                if( ( read_count >> 1 ) != meta.count )
                    echo( this, EchoLevel_Warning ) << "Read vertex count ( " << ( read_count >> 1 ) << " ) is different from in-file reported vertex count ( " << meta.count << " ).";

                if( read_count & 1 )
                    echo( this, EchoLevel_Warning ) << "Read vertex count is odd, meaning there is a missing Y or an extra X.";

                
            break; }
        }

        file.close();

        echo( this, EchoLevel_Ok ) << "Created from: \"" << path.data() << "\".";
    }

public:
    Clust2( const Clust2& other )
    : _origin{ other._origin },
      _vrtx  { other._vrtx },
      _scaleX{ other._scaleX },     
      _scaleY{ other._scaleY },
      _angel { other._angel }
    {}

    Clust2& operator = ( const Clust2& other ) {
        _origin = other._origin;
        _vrtx   = other._vrtx;
        _scaleX = other._scaleX;
        _scaleY = other._scaleY;
        _angel  = other._angel;

        return *this;
    }

    Clust2( Clust2&& other ) noexcept
    : _origin{ std::move( other._origin ) },
      _vrtx  { std::move( other._vrtx ) },
      _scaleX{ other._scaleX },
      _scaleY{ other._scaleY },
      _angel { other._angel }
    {}

    Clust2& operator = ( Clust2&& other ) noexcept {
        _origin = std::move( other._origin );
        _vrtx   = std::move( other._vrtx );
        _scaleX = other._scaleX;
        _scaleY = other._scaleY;
        _angel  = other._angel;

        return *this;
    }

_ENGINE_PROTECTED:
    Vec2                  _origin   = {};
    std::vector< Vrtx >   _vrtx     = {};

    ggfloat_t             _scaleX   = 1.0;
    ggfloat_t             _scaleY   = 1.0;
    ggfloat_t             _angel    = 0.0;

public:
    Vec2 origin() const {
        return _origin;
    }

    Vec2& origin_ref() {
        return _origin;
    }

    operator Vec2 () const {
        return this->origin();
    }

    Vec2 operator () () const {
        return this->operator Vec2();
    }

public:
    Vec2& base_vrtx( size_t idx ) {
        return _vrtx[ idx ].second;
    }

    Vec2& operator [] ( size_t idx ) {
        return _vrtx[ idx ].first;
    }

    Vec2 operator() ( size_t idx ) const {
        return _vrtx[ idx ].first + this->origin();
    }

    size_t vrtx_count() const {
        return _vrtx.size();
    }

public:
    ggfloat_t angel() const {
        return _angel;
    }

    ggfloat_t scaleX() const {
        return _scaleX;
    }

    ggfloat_t scaleY() const {
        return _scaleY;
    }

    ggfloat_t scale() const {
        return this->scaleX();
    }

public:
    Clust2& push_base() {
        for( auto& vrtx : _vrtx )
            vrtx.second = vrtx.first;
            
        return *this;
    }

public:
    Clust2& relocate_at( const Vec2& vec ) {
        _origin = vec;
        return *this;
    }

    Clust2& operator = ( const Vec2& vec ) {
        return this->relocate_at( vec );
    }

    Clust2& relocate_by( size_t idx, const Vec2& vec ) {
        _origin += vec.respect( this->operator()( idx ) );
        return *this;
    }

public:
    Clust2& spin_with( ggfloat_t theta ) {
        _angel += theta;

        this->_refresh();

        return *this;
    }

    Clust2& spin_at( ggfloat_t theta ) {
        _angel = theta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleX_with( ggfloat_t delta ) {
        _scaleX *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleY_with( ggfloat_t delta ) {
        _scaleY *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scale_with( ggfloat_t delta ) {
        _scaleX *= delta;
        _scaleY *= delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleX_at( ggfloat_t delta ) {
        _scaleX = delta;

        this->_refresh();

        return *this;
    }

    Clust2& scaleY_at( ggfloat_t delta ) {
        _scaleY = delta;

        this->_refresh();

        return *this;
    }

    Clust2& scale_at( ggfloat_t delta ) {
        _scaleX = _scaleY = delta;

        this->_refresh();

        return *this;
    }

public:
    static Clust2 triangle( ggfloat_t edge_length ) {
        Vec2 vrtx = { 0.0_ggf, edge_length * ( ggfloat_t )sqrt( 3.0 ) / 3.0_ggf };

        return std::vector< Vec2 >( {
            vrtx,
            vrtx.spinned( 120.0 ),
            vrtx.spinned( -120.0 )
        } );
    }

    static Clust2 square( ggfloat_t edge_length ) {
        edge_length /= 2.0;

        return std::vector< Vec2 >( {
            { edge_length, edge_length },
            { edge_length, -edge_length },
            { -edge_length, -edge_length },
            { -edge_length, edge_length }
        } );
    }

    static Clust2 circle( ggfloat_t radius, size_t precision ) {
        std::vector< Vec2 > vrtx;
        vrtx.reserve( precision );

        vrtx.emplace_back( 0.0, radius );

        for( size_t n = 1; n < precision; ++n )
            vrtx.push_back( vrtx.front().spinned( 360.0 / precision * n ) );

        return vrtx;
    }

    static Clust2 rect( Vec2 tl, Vec2 br ) {
        return std::vector< Vec2 >( {
            tl, Vec2( br.x, tl.y ), br, Vec2( tl.x, br.y )
        } );
    }

    template< typename T >
    requires std::is_invocable_v< T >
    static Clust2 random(
        ggfloat_t min_dist, ggfloat_t max_dist,
        size_t min_ec, size_t max_ec,
        const T& generator
    ) {
        static auto scalar = [] ( const auto& generator, ggfloat_t min ) -> ggfloat_t {
            return ( ( ggfloat_t )( std::invoke( generator ) % 10001 ) / 10000 )
                    * ( 1.0 - min ) + min;
        };

        size_t edge_count = std::invoke( generator ) % ( max_ec - min_ec + 1 ) + min_ec;

        Vec2 vrtx[ edge_count ];

        vrtx[ 0 ] = { 0.0, max_dist };

        ggfloat_t diff = 360.0 / edge_count;


        for( size_t n = 1; n < edge_count; ++n )
            vrtx[ n ] = vrtx[ 0 ].spinned( diff * n + ( scalar( generator, min_dist / max_dist ) - 0.5 ) * diff )
                        *
                        scalar( generator, min_dist / max_dist );


        return { vrtx, vrtx + edge_count };
    }

public:
    size_t extreme_idx( HEADING heading ) {
        size_t ex_idx = 0;

        for( size_t idx = 0; idx < this->vrtx_count(); ++idx )
            if( _vrtx[ idx ].first.is_further_than( _vrtx[ ex_idx ].first, heading ) )
                ex_idx = idx;

        return ex_idx;
    }

    Vec2& extreme_ref( HEADING heading ) {
        return _vrtx[ this->extreme_idx( heading ) ].first;
    }

    Vec2 extreme( HEADING heading, SYSTEM system = SYSTEM_GLOBAL ) const {
        return const_cast< Clust2* >( this )->extreme_ref( heading )
                +
                ( system == SYSTEM_GLOBAL ? this->origin() : Vec2::O() );
    }

public:
    template< ggX_result X_T >
    auto X( const Ray2& ray ) const {
        if constexpr( std::is_same_v< bool, X_T > )
            return this->_intersection_with_ray_assert( ray );
        else if constexpr( std::is_same_v< Vec2, X_T > )
            return this->_intersection_with_ray_points( ray );
    }

    template< ggX_result X_T >
    auto X( const Clust2& other ) const {
        if constexpr( std::is_same_v< bool, X_T > )
            return this->_intersect_bool( other );
        else if constexpr( std::is_same_v< Vec2, X_T > )
            return this->_intersect_vec( other );
    }

_ENGINE_PROTECTED:
    bool _intersection_with_ray_assert( const Ray2& ray ) const {
        /*for( auto itr = this->outter_ray_begin(); itr != this->outter_ray_end(); ++itr )
            if( itr->X< bool >( ray ) )
                return true;*/

        return false;
    }

    std::vector< Vec2 > _intersection_with_ray_points( const Ray2& ray ) const {
        std::vector< Vec2 > Xs{};

        /*for( auto itr = this->outter_ray_begin(); itr != this->outter_ray_end(); ++itr ) {
            auto point = itr->X< Vec2 >( ray );

            if( point.has_value() )
                Xs.push_back( point.value() );
        }*/

        return Xs;
    }

    bool _intersect_bool( const Clust2& other ) const {
        for( size_t idx = 0; idx < other.vrtx_count(); ++idx )
            if( this->X< bool >( other._mkray( idx ) ) )
                return true;

        return false;
    }

    std::vector< Vec2 > _intersect_vec( const Clust2& other ) const {
        std::vector< Vec2 > Xs{};

        for( size_t idx = 0; idx < other.vrtx_count(); ++idx ) {
            auto vecs = this->X< Vec2 >( other._mkray( idx ) );

            Xs.insert( Xs.end(), vecs.begin(), vecs.end() );
        }

        return Xs;
    }

public:
    bool contains( Vec2 vec ) const {
        Ray2    ref     = { vec, Vec2{ std::numeric_limits< ggfloat_t >::max(), .0_ggf } };
        int32_t x_count = 0;
        Ray2    phase   = { _vrtx.back().first + _origin, _vrtx.front().first - _vrtx.back().first };

        for( auto edge_itr = this->coutter_ray_begin(); edge_itr != this->coutter_ray_end(); ++edge_itr ) {
            Ray2 edge = *edge_itr;

            if( edge.vec.y == .0_ggf ) continue;

            if( !ref.X< bool >( phase ) ) {
                phase = edge;
                continue;
            }

            if( edge.origin.y == ref.origin.y ) {
                x_count += std::signbit( phase.vec.dot( edge.vec ) );
                ++edge_itr;
            }

            phase = edge;
            ++x_count;
        }
        
        return x_count & 0x1;
    }

_ENGINE_PROTECTED:
    void _refresh() {
        for( Vrtx& v : _vrtx )
            v.first = v.second.spinned( _angel ) * Vec2{ _scaleX, _scaleY };
    }

    Ray2 _mkray( size_t idx ) const {
        return { ( *this )( idx ), ( *this )( ( idx + 1 ) % this->vrtx_count() )( ( *this )( idx ) ) };
    }

    Ray2 _mkray( size_t idx1, size_t idx2 ) const {
        return { ( *this )( idx1 ), ( *this )( idx2 )( ( *this )( idx1 ) ) };
    }


_ENGINE_PROTECTED:
    using _vrtx_itr_t = decltype( _vrtx )::iterator;
    using _vrtx_citr_t = decltype( _vrtx )::const_iterator;

_ENGINE_PROTECTED:
    template< bool is_const >
    struct _inner_vrtx_iterator_base {
    _ENGINE_PROTECTED:
        using _uth_itr_t = std::conditional_t< is_const, _vrtx_citr_t, _vrtx_itr_t >;

    public:
        _inner_vrtx_iterator_base( _uth_itr_t itr )
        : _itr{ itr }
        {}

    _ENGINE_PROTECTED:
        _uth_itr_t   _itr   = {};

    public:
        _inner_vrtx_iterator_base& operator ++ () {
            ++_itr;
            return *this;
        }

        _inner_vrtx_iterator_base operator ++ ( [[maybe_unused]] int ) {
            auto last = _itr;
            
            this->operator++();

            return { last };
        }

    public:
        bool operator == ( const _inner_vrtx_iterator_base& other ) const {
            return _itr == other._itr;
        }

    public:
        std::conditional_t< is_const, const Vec2&, Vec2& > operator * () {
            return _itr->first;
        }

        std::conditional_t< is_const, const Vec2*, Vec2* > operator -> () {
            return &_itr->first;
        }

    };

public:
    struct inner_vrtx_iterator : _inner_vrtx_iterator_base< false > {
        using _inner_vrtx_iterator_base::_inner_vrtx_iterator_base;
    };

    inner_vrtx_iterator inner_vrtx_begin() {
        return { _vrtx.begin() };
    }

    inner_vrtx_iterator inner_vrtx_end() {
        return { _vrtx.end() };
    }

    struct cinner_vrtx_iterator : _inner_vrtx_iterator_base< true > {
        using _inner_vrtx_iterator_base::_inner_vrtx_iterator_base;
    };

    cinner_vrtx_iterator cinner_vrtx_begin() const {
        return { _vrtx.cbegin() };
    }

    cinner_vrtx_iterator cinner_vrtx_end() const {
        return { _vrtx.cend() };
    }

_ENGINE_PROTECTED:
    template< bool is_const >
    struct _outter_vrtx_iterator_base {
    _ENGINE_PROTECTED:
        using _uth_itr_t = std::conditional_t< is_const, _vrtx_citr_t, _vrtx_itr_t >;
        using _uth_ref_t = std::conditional_t< is_const, const Clust2*, Clust2* >;

    public:
        _outter_vrtx_iterator_base( _uth_itr_t itr, _uth_ref_t ref )
        : _itr{ itr }, _ref{ ref }
        {}

    _ENGINE_PROTECTED:
        _uth_itr_t   _itr   = {};
        _uth_ref_t   _ref   = { nullptr };

    public:
        _outter_vrtx_iterator_base& operator ++ () {
            ++_itr;
            return *this;
        }

        _outter_vrtx_iterator_base operator ++ ( [[maybe_unused]] int ) {
            auto last = _itr;
            
            this->operator++();

            return { last, _ref };
        }

    public:
        bool operator == ( const _outter_vrtx_iterator_base& other ) const {
            return _itr == other._itr;
        }

    public:
        Vec2 operator * () {
            return _ref->_origin + _itr->first;
        }

    };

public:
    struct outter_vrtx_iterator : _outter_vrtx_iterator_base< false > {
        using _outter_vrtx_iterator_base::_outter_vrtx_iterator_base;
    };

    outter_vrtx_iterator outter_vrtx_begin() {
        return { _vrtx.begin(), this };
    }

    outter_vrtx_iterator outter_vrtx_end() {
        return { _vrtx.end(), nullptr };
    }

    struct coutter_vrtx_iterator : _outter_vrtx_iterator_base< true > {
        using _outter_vrtx_iterator_base::_outter_vrtx_iterator_base;
    };

    coutter_vrtx_iterator coutter_vrtx_begin() const {
        return { _vrtx.cbegin(), this };
    }

    coutter_vrtx_iterator coutter_vrtx_end() const {
        return { _vrtx.cend(), nullptr };
    }


_ENGINE_PROTECTED:
    template< bool is_const >
    struct _inner_ray_iterator_base {
    _ENGINE_PROTECTED:
        using _uth_itr_t = std::conditional_t< is_const, _vrtx_citr_t, _vrtx_itr_t >;
        using _uth_ref_t = std::conditional_t< is_const, const Clust2*, Clust2* >;

    public:
        _inner_ray_iterator_base( _uth_itr_t itr, _uth_ref_t ref )
        : _itr{ itr }, _ref{ ref }
        {}

    _ENGINE_PROTECTED:
        _uth_itr_t   _itr   = {};
        _uth_ref_t   _ref   = { nullptr };

    public:
        _inner_ray_iterator_base& operator ++ () {
            ++_itr;
            return *this;
        }

        _inner_ray_iterator_base operator ++ ( [[maybe_unused]] int ) {
            auto last = _itr;
            
            this->operator++();

            return { last, _ref };
        }

    public:
        bool operator == ( const _inner_ray_iterator_base& other ) const {
            return _itr == other._itr;
        }

    public:
        Ray2 operator * () {
            return { _ref->_origin, _itr->first };
        }

        Vec2* operator -> () {
            return &_itr->first;
        }

    };

public:
    struct inner_ray_iterator : _inner_ray_iterator_base< false > {
        using _inner_ray_iterator_base::_inner_ray_iterator_base;
    };

    inner_ray_iterator inner_ray_begin() {
        return { _vrtx.begin(), this };
    }

    inner_ray_iterator inner_ray_end() {
        return { _vrtx.end(), nullptr };
    }

    struct cinner_ray_iterator : _inner_ray_iterator_base< true > {
        using _inner_ray_iterator_base::_inner_ray_iterator_base;
    };

    cinner_ray_iterator cinner_ray_begin() const {
        return { _vrtx.cbegin(), this };
    }

    cinner_ray_iterator cinner_ray_end() const {
        return { _vrtx.cend(), nullptr };
    }

_ENGINE_PROTECTED:
    template< bool is_const >
    struct _outter_ray_iterator_base {
    _ENGINE_PROTECTED:
        using _uth_itr_t = std::conditional_t< is_const, _vrtx_citr_t, _vrtx_itr_t >;
        using _uth_ref_t = std::conditional_t< is_const, const Clust2*, Clust2* >;

    public:
        _outter_ray_iterator_base( _uth_itr_t org, _uth_itr_t drop, _uth_ref_t ref )
        : _org{ org }, _drop{ drop }, _ref{ ref }
        {}

    _ENGINE_PROTECTED:
        _uth_itr_t   _org    = {};
        _uth_itr_t   _drop   = {};
        _uth_ref_t   _ref    = { nullptr };

    public:
        _outter_ray_iterator_base& operator ++ () {
            if( _drop == _ref->_vrtx.cbegin() ) {
                if constexpr( is_const )
                    _org = _ref->_vrtx.cbegin();
                else
                    _org = _ref->_vrtx.begin();

                return *this;
            }
            
            ++_org;
            ++_drop;

            if( _drop == _ref->_vrtx.cend() ) {
                if constexpr( is_const )
                    _drop = _ref->_vrtx.cbegin();
                else
                    _drop = _ref->_vrtx.begin();
            }

            return *this;
        }

        _outter_ray_iterator_base operator ++ ( [[maybe_unused]] int ) {
            auto last = std::make_pair( _org, _drop );
            
            this->operator++();

            return { last.first, last.second, _ref };
        }

    public:
        bool operator == ( const _outter_ray_iterator_base& other ) const {
            return _org == other._org && _drop == other._drop;
        }

    public:
        Ray2 operator * () {
            Vec2 org = _ref->_origin + _org->first;
            return { org, _ref->_origin + _drop->first - org };
        }

    };

public:
    struct outter_ray_iterator : _outter_ray_iterator_base< false > {
        using _outter_ray_iterator_base::_outter_ray_iterator_base;
    };

    outter_ray_iterator outter_ray_begin() {
        return { _vrtx.begin(), _vrtx.begin() + 1, this };
    }

    outter_ray_iterator outter_ray_end() {
        return { _vrtx.begin(), _vrtx.begin(), nullptr };
    }

    struct coutter_ray_iterator : _outter_ray_iterator_base< true > {
        using _outter_ray_iterator_base::_outter_ray_iterator_base;
    };

    coutter_ray_iterator coutter_ray_begin() const {
        return { _vrtx.cbegin(), _vrtx.cbegin() + 1, this };
    }

    coutter_ray_iterator coutter_ray_end() const {
        return { _vrtx.cbegin(), _vrtx.cbegin(), nullptr };
    }


public:
    static Clust2 from_file( std::string_view path ) {
        std::ifstream file( path.data() );

        if( !file ) return {};

        Vec2 org = {};

        std::vector< Vec2 > vrtx = {};

        {
            bool has_origin = false;

            file >> has_origin;

            if( has_origin )
                file >> org.x >> org.y;
        }

        for( Vec2 vec; file >> vec.x >> vec.y; )
            vrtx.push_back( vec );

        return { org, vrtx };
    }

};



#pragma endregion D2



#pragma region D3

#pragma endregion D3



#pragma endregion Spatial



};
