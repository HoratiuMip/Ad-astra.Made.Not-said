/*
*/

#include <iostream>

#include <IXT/aritm.hpp>

using namespace IXT;



int main() {
    auto clust = Clust2::triangle( 10.0 );
    clust.relocate_at( { 0.0, 10.0 } );

    for( auto itr = clust.cinner_vrtx_begin(); itr != clust.cinner_vrtx_end(); ++itr )
        std::cout << itr->x << ' ' << itr->y << '\n';

    for( auto itr = clust.coutter_vrtx_begin(); itr != clust.coutter_vrtx_end(); ++itr )
        std::cout << (*itr).x << ' ' << (*itr).y << '\n';

    for( auto itr = clust.cinner_ray_begin(); itr != clust.cinner_ray_end(); ++itr )
        std::cout << (*itr).origin.x << ' ' << (*itr).origin.y << ' ' << (*itr).vec.x << ' ' << (*itr).vec.y << '\n';
}