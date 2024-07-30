#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/concepts.hpp>

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



class Vec2 {
public:
    static ggfloat_t norm(
        ggfloat_t x, ggfloat_t y
    ) {
        return sqrt( x*x + y*y );
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
        return x1*y2 + x2*y1;
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
    ggfloat_t dot( const Vec2& other ) const {
        return dot_product( this->x, this->y, other.x, other.y );
    }

public:
    ggfloat_t mag_sq() const {
        return x * x + y * y;
    }

    ggfloat_t mag() const {
        return sqrt( this->mag_sq() );
    }

    ggfloat_t angel() const {
        return Deg::pull( atan2( y, x ) );
    }

public:
    ggfloat_t dist_sq_to( const Vec2& other ) const {
        return ( other.x - x ) * ( other.x - x ) + ( other.y - y ) * ( other.y - y );
    }

    ggfloat_t dist_to( const Vec2& other ) const {
        return sqrt( this->dist_sq_to( other ) );
    }

public:
    Vec2 respect_to( const Vec2& other ) const {
        return { x - other.x, y - other.y };
    }

    Vec2 operator () ( const Vec2& other ) const {
        return this->respect_to( other );
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
        return this->polar( other.respect_to( *this ).angel(), dist );
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

    Vec2& spin( ggfloat_t theta, const Vec2& other ) {
        *this = this->respect_to( other ).spin( theta ) + other;

        return *this;
    }

    Vec2 spinned( ggfloat_t theta ) const {
        return Vec2{ *this }.spin( theta );
    }

    Vec2 spinned( ggfloat_t theta, const Vec2& other ) const {
        return Vec2{ *this }.spin( theta, other );
    }

public:
    bool is_further_than( const Vec2& other, HEADING heading ) const {
        switch( heading ) {
            case HEADING_NORTH: return y > other.y;
            case HEADING_EAST:  return x > other.x;
            case HEADING_SOUTH: return y < other.y;
            case HEADING_WEST:  return x < other.x;
        }

        return false;
    }

public:
    template< typename X_T >
    auto X( const Ray2& ray ) const;

    template< typename X_T >
    auto X( const Clust2& clust ) const;

public:
    Vec2& operator = ( const Vec2& other ) {
        x = other.x; y = other.y; return *this;
    }

    Vec2& operator = ( ggfloat_t val ) {
        x = y = val; return *this;
    }

    bool operator == ( const Vec2& other ) const {
        return x == other.x && y == other.y;
    }

    Vec2 operator + ( const Vec2& other ) const {
        return { x + other.x, y + other.y };
    }

    Vec2 operator - ( const Vec2& other ) const {
        return { x - other.x, y - other.y };
    }

    Vec2 operator * ( const Vec2& other ) const {
        return { x * other.x, y * other.y };
    }

    Vec2 operator / ( const Vec2& other ) const {
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

    Vec2& operator += ( const Vec2& other ) {
        x += other.x;
        y += other.y;

        return *this;
    }

    Vec2& operator -= ( const Vec2& other ) {
        x -= other.x;
        y -= other.y;

        return *this;
    }

    Vec2& operator *= ( const Vec2& other ) {
        x *= other.x;
        y *= other.y;

        return *this;
    }

    Vec2& operator /= ( const Vec2& other ) {
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
    operator const D2D1_POINT_2F& () const {
        return *reinterpret_cast< const D2D1_POINT_2F* >( this );
    }

    operator D2D1_POINT_2F& () {
        return *reinterpret_cast< D2D1_POINT_2F* >( this );
    }

};



template< typename T > concept ggX_result =
    std::is_same_v< bool, T > || std::is_same_v< Vec2, T >;



class Ray2 {
public:
    static bool intersection_assert( 
        ggfloat_t ox1, ggfloat_t oy1, ggfloat_t dx1, ggfloat_t dy1,
        ggfloat_t ox2, ggfloat_t oy2, ggfloat_t dx2, ggfloat_t dy2
    ) {
        ggfloat_t first = Vec2::cross_product( ox1, oy1, ox2, oy2 );
        ggfloat_t second = Vec2::cross_product( ox1, oy1, ox2 + dx2, oy2 + dy2 );

        if( std::signbit( first ) == std::signbit( second ) ) return false;

        first = Vec2::cross_product( ox2, oy2, ox1, oy1 );
        second = Vec2::cross_product( ox2, oy2, ox1 + dx1, oy1 + dy1 );

        if( std::signbit( first ) == std::signbit( second ) ) return false;

        return true;
    }

    static bool intersection_point(
        ggfloat_t ox1, ggfloat_t oy1, ggfloat_t dx1, ggfloat_t dy1,
        ggfloat_t ox2, ggfloat_t oy2, ggfloat_t dx2, ggfloat_t dy2,
        ggfloat_t* ix, ggfloat_t* iy
    ) {
        ggfloat_t det = dy1*-dx2 + dx1*dy2;

        if( det == 0.0 ) return false;

        ggfloat_t sx = -( -ox1*dy1 + oy1*dx1 );
        ggfloat_t sy = -( -ox2*dy2 + oy2*dx2 );

        ggfloat_t x = ( sx*-dx2 + dx1*sy ) / det;
        ggfloat_t y = ( dy1*sy - sx*dy2 ) / det;
        ggfloat_t vx = x - ox1;
        ggfloat_t vy = y - oy1;

        if( vx < 0.0 || vx > dx1 ) return false;
        if( vy < 0.0 || vy > dy1 ) return false;
 
        vx = x - ox2;
        vy = y - oy2; 

        if( vx < 0.0 || vx > dx2 ) return false;
        if( vy < 0.0 || vy > dy2 ) return false;

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

public:
    ggfloat_t slope() const {
        return ( this->drop().y - origin.y ) / ( this->drop().x - origin.x );
    }

    ggfloat_t y_int() const {
        return origin.y - this->slope() * origin.x;
    }

    std::tuple< ggfloat_t, ggfloat_t, ggfloat_t > coeffs() const {
        return { vec.y, -vec.x, vec.y * origin.x - vec.x * origin.y };
    }

public:
    template< typename X_T >
    auto X( const Vec2& vec ) const {
        this->X< X_T >( Ray2{ Vec2::O(), vec } );
    }

    template< typename X_T >
    auto X( const Ray2& other ) const {
        if constexpr( std::is_same_v< bool, X_T > )
            return this->_intersect_vec( other ).has_value();
        else if constexpr( std::is_same_v< Vec2, X_T > )
            return this->_intersect_vec( other );
    }

    template< typename X_T >
    auto X( const Clust2& clust ) const;

_ENGINE_PROTECTED:
    std::optional< Vec2 > _intersect_vec( const Ray2& other ) const {
        /* Hai noroc nea' Peter +respect. */

        static auto has_any_normal_component = [] ( Vec2 normalized ) -> bool {
            return ( normalized.x >= 0.0 && normalized.x <= 1.0 )
                    ||
                    ( normalized.y >= 0.0 && normalized.y <= 1.0 );
        };

        auto [ alpha, bravo, charlie ] = this->coeffs();
        auto [ delta, echo, foxtrot ] = other.coeffs();

        ggfloat_t golf = alpha * echo - bravo * delta;

        if( golf == 0.0 ) return {};

        Vec2 X_vec = {
            ( charlie * echo - bravo * foxtrot ) / golf,
            ( alpha * foxtrot - delta * charlie ) / golf
        };

        if( !has_any_normal_component( X_vec( origin ) / vec ) ) return {};

        if( !has_any_normal_component( X_vec( other.origin ) / other.vec ) ) return {};

        return X_vec;
    }

};



class Clust2 {
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

public:
    Clust2( const Clust2& other )
        : _origin( other._origin ),
            _vrtx  ( other._vrtx ),
            _scaleX( other._scaleX ),
            _scaleY( other._scaleY ),
            _angel ( other._angel )
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
    Clust2& relocate_at( const Vec2& vec ) {
        _origin = vec;
        return *this;
    }

    Clust2& operator = ( const Vec2& vec ) {
        return this->relocate_at( vec );
    }

    Clust2& relocate_by( size_t idx, const Vec2& vec ) {
        _origin += vec.respect_to( this->operator()( idx ) );
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
        Vec2 vrtx = { 0.0, edge_length * sqrt( 3.0 ) / 3.0 };

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
            return ( static_cast< ggfloat_t >( std::invoke( generator ) % 10001 ) / 10000 )
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
    template< typename X_T >
    auto X( const Vec2& vec ) const {
        return this->X< X_T >( Ray2{ Vec2::O(), vec } );
    }

    template< typename X_T >
    auto X( const Ray2& ray ) const {
        if constexpr( std::is_same_v< bool, X_T > )
            return this->_intersect_ray_bool( ray );
        else if constexpr( std::is_same_v< Vec2, X_T > )
            return this->_intersect_ray_vec( ray );
    }

    template< typename X_T >
    auto X( const Clust2& other ) const {
        if constexpr( std::is_same_v< bool, X_T > )
            return this->_intersect_bool( other );
        else if constexpr( std::is_same_v< Vec2, X_T > )
            return this->_intersect_vec( other );
    }

_ENGINE_PROTECTED:
    bool _intersect_ray_bool( const Ray2& ray ) const {
        for( size_t idx = 0; idx < this->vrtx_count(); ++idx )
            if( this->_mkray( idx ).X< bool >( ray ) )
                return true;

        return false;
    }

    std::vector< Vec2 > _intersect_ray_vec( const Ray2& ray ) const {
        std::vector< Vec2 > Xs{};

        for( size_t idx = 0; idx < this->vrtx_count(); ++idx ) {
            auto vec = this->_mkray( idx ).X< Vec2 >( ray );

            if( vec.has_value() )
                Xs.push_back( vec.value() );
        }

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



template< typename X_T >
auto Vec2::X( const Ray2& ray ) const {
    return ray.X< X_T >( *this );
}

template< typename X_T >
auto Vec2::X( const Clust2& clust ) const {
    return clust.X< X_T >( *this );
}


template< typename X_T >
auto Ray2::X( const Clust2& clust ) const {
    return clust.X< X_T >( *this );
}



#pragma endregion D2



#pragma region D3

#pragma endregion D3



#pragma endregion Spatial





};