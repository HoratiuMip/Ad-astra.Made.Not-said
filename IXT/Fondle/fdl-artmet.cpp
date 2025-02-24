#include <IXT/comms.hpp>
#include <IXT/artmet.hpp>

using namespace IXT;

int main( int argc, char* argv[] ) {
/*
    UTCN-AUT | RTS | Lab I/2
*/
{
    art::matrix_r mat1{
        { 2, 3, 1 },
        { 7, 1, 6 },
        { 9, 2, 4 }
    };
    comms() << mat1;

    art::matrix< int > mat2{
        { 8, 5, 3 },
        { 3, 9, 2 },  
        { 2, 7, 3 }
    };
    comms() << mat2;

    comms() << "mat1 + mat2, " << ( mat1 + mat2 );
    comms() << "mat1 - mat2, " << ( mat1 - mat2 );
}

    if( argc > 1 ) return 0;

/////=/////=/////=/////
{
    art::vector_r vec1{ 1, 2, 3 };
    art::vector_r vec2{ 4, 5, 6 };

    comms() << "vec1 * vec2, " << ( vec1 * vec2 );
    comms() << "vec1 & vec2, " << ( vec1 & vec2 );
    comms() << "vec1^2, " << ( vec1.arg_in( (long double (*)(long double, long double) )(&std::pow), 2 ) );

    art::matrix_r mat1{
        { 2, 0, 0 },
        { 0, 2, 0 },
        { 0, 0, 2 }
    };

    comms() << ( mat1 * vec1 );
    comms() << ( mat1 * vec2 );

    comms() << mat1;

    comms() << art::matrix_r( 4, 3 );
}

    return 0;
}
