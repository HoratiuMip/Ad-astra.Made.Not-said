#pragma once
/*
Operations which could be specialized depending on T and AVX level are marked with "T&AVX".
*/

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>

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
    : dat_base_t{ len, T{} } 
    {}

    vector( const vector& other )
    : dat_base_t{ other.begin(), other.end() } 
    {}

    vector( vector&& other )
    : dat_base_t{ std::move( ( dat_base_t& )other ) }
    {}

public:
    vector& sc_mul( const T& sc ) {
        /* T&AVX */
        for( auto& e : *this ) e *= sc;
        return *this;
    }

    inline vector& operator *= ( const T& sc ) { return this->sc_mul( sc ); }
    inline vector operator * ( const T& sc ) { return vector{ *this }->sc_mul( sc ); }

    T dot( const vector& other ) {
        if( this->size() != other.size() ) {
            comms( this, ECHO_LEVEL_ERROR ) << "Dot multiplication of different length vectors, this ( " << this->size() << " ), other ( " << other.size() << " ).";
            return T{};
        }

        T rez = T{};

        /* T&AVX */
        for( DWORD idx = 0; idx < this->size(); ++idx ) {
            rez += this->operator[]( idx ) * other.operator[]( idx );
        }

        return rez;
    }

    inline T operator &= ( const vector& other ) { return this->dot( other ); }
    inline T operator & ( const vector& other ) const { return vector{ *this }.dot( other ); } 

    vector& ew_mul( const vector& other ) {
        if( this->size() != other.size() ) {
            comms( this, ECHO_LEVEL_ERROR ) << "Element-wise multiplication of different length vectors, this ( " << this->size() << " ), other ( " << other.size() << " ).";
            return *this;
        }

        /* T&AVX */
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

};

template< typename T > inline std::ostream& operator << ( std::ostream& os, const matrix< T >& mat ) {
    os << "matrix of ( " << mat.size() << "x" << mat.front().size() << " ) elements:\n"; 
    for( auto& row : mat ) {
        
    }
    return os << " ]\n";
}


using matrix_r = matrix< long double >;



} };
