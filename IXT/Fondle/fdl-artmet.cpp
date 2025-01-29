#include <IXT/comms.hpp>
#include <IXT/artmet.hpp>

using namespace IXT;

int main() {
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
    
    return 0;
}
