//
//  GPSLab1.cpp
//
//  Copyright © 2017 CGIS. All rights reserved.
//

#include "GPSLab1.hpp"

namespace gps {
    glm::vec4 TransformPoint(const glm::vec4 &point)
    {
        glm::mat4 tm = glm::mat4{ 1.0 };
        tm = glm::rotate( tm, glm::radians( 90.0f ), glm::vec3{ 1.0, .0, .0 } );
        tm = glm::translate( tm, glm::vec3{ 2.0, .0, 1.0 } );

        return tm * point;
    }
    
    float ComputeAngle(const glm::vec3 &v1, const glm::vec3 &v2)
    {
        float rez = glm::dot( v1, v2 ) / ( glm::length( v1 ) * glm::length( v2 ) );
        rez = acos( rez );

        return glm::degrees( rez );
    }
    
    bool IsConvex(const std::vector<glm::vec2> &vertices)
    {
        static auto glm_vec_2_to_3 = [] ( glm::vec2 v ) -> glm::vec3 {
            return { v.x, v.y, .0f };
        };

        if( vertices.size() < 3 )
            return false;

        glm::vec3 crt[] = {
            glm_vec_2_to_3( vertices[ 0 ] ),
            glm_vec_2_to_3( vertices[ 1 ] ),
            glm_vec_2_to_3( vertices[ 2 ] )
        };

        bool master_sgn = glm::cross( crt[ 1 ] - crt[ 0 ], crt[ 2 ] - crt[ 1 ] ).z < .0;

        for( size_t idx = 3; idx < vertices.size(); ++idx ) {
            crt[ 0 ] = crt[ 1 ]; crt[ 1 ] = crt[ 2 ];
            crt[ 2 ] = glm_vec_2_to_3( vertices[ idx ] );

            bool sgn = glm::cross( crt[ 1 ] - crt[ 0 ], crt[ 2 ] - crt[ 1 ] ).z < .0;

            if( sgn != master_sgn )
                return false;
        }

        return true;
    }
    
    std::vector<glm::vec2> ComputeNormals(const std::vector<glm::vec2> &vertices)
    {
        std::vector<glm::vec2> normalsList;

        if( vertices.size() < 2 )
            return normalsList;

        normalsList.reserve( vertices.size() );

        glm::vec2 ref = vertices.front();

        for( size_t idx = 1; idx < vertices.size(); ++idx ) {
            glm::vec2 v = vertices[ idx ] - ref;
            v.x *= -1.0;
            std::swap( v.x, v.y );

            normalsList.emplace_back( glm::normalize( v ) );

            ref = vertices[ idx ];
        }

        return normalsList;
    }
}
