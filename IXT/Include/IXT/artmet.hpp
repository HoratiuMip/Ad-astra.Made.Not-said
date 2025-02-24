#pragma once
/*
Further to be specialized depending on T, and, perhaps, using AVX.
*/

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>
#include <IXT/stl_helper.hpp>

namespace _ENGINE_NAMESPACE { namespace art {



template< typename T > struct vector : public Descriptor, public std::vector< T > {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "art::vector" );

public:
    using dat_base_t = std::vector< T >;

public:
    using dat_base_t::dat_base_t;

    vector() = default;

    vector( DWORD len ) 
    : dat_base_t( len, T{} ) /* () instead of {}, don't want initializer list constructor */
    {}

    vector( const vector& other )
    : dat_base_t{ other.begin(), other.end() } 
    {}

    vector( vector&& other )
    : dat_base_t{ std::move( ( dat_base_t& )other ) }
    {}

public:
    DWORD len() const { return this->size(); }

public:
    T& inv_idx( DWORD c, DWORD r ) { return ( *this )[ r ][ c ]; }

public: 
#define _ARTMET_VECTOR_SC_OP_FUNC_T_SIG( op ) template< typename T_sc > requires requires{ std::declval< T >() op##= T_sc{}; }
#define _ARTMET_VECTOR_SC_OP_FUNC( name, op ) \
    _ARTMET_VECTOR_SC_OP_FUNC_T_SIG( op ) \
    inline vector& name( const T_sc& sc ) { \
        for( auto& e : *this ) e op##= sc; \
        return *this; \
    } \
    _ARTMET_VECTOR_SC_OP_FUNC_T_SIG( op ) \
    inline vector ct_##name( const T_sc& sc ) { \
        return vector{ *this }.name( sc ); \
    } \
    _ARTMET_VECTOR_SC_OP_FUNC_T_SIG( op ) inline vector& operator op##= ( const T_sc& sc ) { return this->name< T_sc >( sc ); } \
    _ARTMET_VECTOR_SC_OP_FUNC_T_SIG( op ) inline vector operator op ( const T_sc& sc ) { return this->ct_##name< T_sc >( sc ); }

    _ARTMET_VECTOR_SC_OP_FUNC( add, + )
    _ARTMET_VECTOR_SC_OP_FUNC( sub, - )
    _ARTMET_VECTOR_SC_OP_FUNC( mul, * )
    _ARTMET_VECTOR_SC_OP_FUNC( div, / )
    _ARTMET_VECTOR_SC_OP_FUNC( shr, >> )
    _ARTMET_VECTOR_SC_OP_FUNC( shl, << )

    template< typename ...FuncArgs, typename ...Args > 
    vector& arg_in( T( *func )( FuncArgs... ), Args&&... args ) {
        for( auto& e : *this ) e = func( e, std::forward< Args >( args )... );
        return *this;
    }

public:
    T dot( const vector& other ) {
        if( this->size() != other.size() ) {
            comms( this, ECHO_LEVEL_ERROR ) << "Dot multiplication of different length vectors, this ( " << this->size() << " ), other ( " << other.size() << " ).";
            return T{};
        }

        T rez = T{};

        for( DWORD idx = 0; idx < this->size(); ++idx ) {
            rez += this->operator[]( idx ) * other.operator[]( idx );
        }

        return rez;
    }

    inline T operator &= ( const vector& other ) { return this->dot( other ); }
    inline T operator & ( const vector& other ) const { return vector{ *this }.dot( other ); } 

public:
    vector& ew_mul( const vector& other ) {
        if( this->size() != other.size() ) {
            comms( this, ECHO_LEVEL_ERROR ) << "Element-wise multiplication of different length vectors, this ( " << this->size() << " ), other ( " << other.size() << " ).";
            return *this;
        }

        for( DWORD idx = 0; idx < this->size(); ++idx ) {
            this->operator[]( idx ) *= other.operator[]( idx );
        }

        return *this;
    }

    inline vector& operator *= ( const vector& other ) { return this->ew_mul( other ); }
    inline vector operator * ( const vector& other ) const { return vector{ *this }.ew_mul( other ); }

};

template< typename T > inline std::ostream& operator << ( std::ostream& os, const vector< T >& vec ) {
    os << "vector of ( " << vec.size() << " ) elements:\n[ "; 
    for( auto& e : vec ) os << e << ' '; 
    return os << " ]\n";
}

using vector_r = vector< long double >;



template< typename T > struct matrix : public Descriptor, std::vector< vector< T > > {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "art::matrix" );

public:
    using dat_base_t = std::vector< vector< T > >;

public:
    using dat_base_t::dat_base_t;

    matrix() = default;

    matrix( DWORD rc, DWORD cc ) {
        this->reserve( rc );
        for( DWORD r = 0; r < rc; ++r ) this->emplace_back().assign( cc, T{} );
    }

public:
    DWORD r_cnt() const { return this->size(); }
    DWORD c_cnt() const { return ( *this )[ 0 ].size(); }

public:
    vector< T > vec_mul_r ( const vector< T >& vec ) const {
        if( this->size() == 0 ) {
            comms( this, ECHO_LEVEL_ERROR ) << "Vector multiplication to the right, this NULL.";
            return {};
        }

        if( this->front().size() != vec.size() ) {
            comms( this, ECHO_LEVEL_ERROR ) << "Vector multiplication to the right lengths mismatch, this ( " << this->front().size() << " ), vector ( " << vec.size() << ").";
            return {};
        }

        vector< T > rez{}; rez.reserve( this->size() );

        for( DWORD idx = 0; idx < this->size(); ++idx ) {
            rez.push_back( ( this->cbegin() + idx )->operator&( vec ) );
        }

        return rez;
    }

    inline vector< T > operator * ( const vector< T > vec ) const { return this->vec_mul_r( vec ); }

public:
    template< typename T_rhs >
    matrix& mul( const matrix< T_rhs >& rhs ) {
        // NULL check?

        if( this->c_cnt() != rhs.r_cnt() ) {
            comms( this, ECHO_LEVEL_ERROR ) << "Mismatching sizes for matrix multiplication.";
            return *this;
        }

        matrix res( this->r_cnt(), rhs.c_cnt() );

        for( DWORD r_res = 0; r_res < res.r_cnt(); ++r_res ) {
            for( DWORD c_res = 0; c_res < res.c_cnt(); ++c_res ) {

            }
        }

        return *this;
    }

public:
    #define _ARTMET_MATRIX_EW_OP_FUNC_T_SIG( op ) template< typename T_rhs > //requires requires{ std::declval< T >() op##= T_rhs{}; }
    #define _ARTMET_MATRIX_EW_FUNC_NO_OP( name, op ) \
    _ARTMET_MATRIX_EW_OP_FUNC_T_SIG( op ) \
    inline matrix& name( const matrix< T_rhs >& rhs ) { \
        for( DWORD row = 0; row < this->size(); ++row ) { \
            if( row >= rhs.size() ) break; \
            for( DWORD col = 0; col < ( *this )[ row ].size(); ++col ) { \
                if( col >= rhs[ row ].size() ) break; \
                ( *this )[ row ][ col ] op##= rhs[ row ][ col ]; \
            } \
        } \
        return *this; \
    } \
    _ARTMET_MATRIX_EW_OP_FUNC_T_SIG( op ) \
    inline matrix ct_##name( const matrix< T_rhs >& rhs ) { \
        return matrix{ *this }.name( rhs ); \
    }
    #define _ARTMET_MATRIX_EW_FUNC( name, op ) \
    _ARTMET_MATRIX_EW_FUNC_NO_OP( name, op ) \
    _ARTMET_MATRIX_EW_OP_FUNC_T_SIG( op ) inline matrix& operator op##= ( const matrix< T_rhs >& rhs ) { return this->name< T_rhs >( rhs ); } \
    _ARTMET_MATRIX_EW_OP_FUNC_T_SIG( op ) inline matrix operator op ( const matrix< T_rhs >& rhs ) { return this->ct_##name< T_rhs >( rhs ); }

    _ARTMET_MATRIX_EW_FUNC( ew_add, + )
    _ARTMET_MATRIX_EW_FUNC( ew_sub, - )

};

template< typename T > inline std::ostream& operator << ( std::ostream& os, const matrix< T >& mat ) {
    os << "matrix of ( " << mat.size() << "x" << mat.front().size() << " ) elements:\n"; 
    for( auto& row : mat ) {
        os << "[ ";
        for( auto& e : row ) os << e << ' ';
        os << "]\n";
    }
    return os << '\n';
}


using matrix_r = matrix< long double >;



} };
