//
//  GPSLab1.hpp
//
//  Copyright © 2017 CGIS. All rights reserved.
//

#ifndef GPSLab1_hpp
#define GPSLab1_hpp

#include <stdio.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gps {
    //TODO: Compute the coordinates of a 3D point defined in homogeneous coordinates
    //      considering the following transformations:
    //      first translate the point with (2.0f, 0.0f, 1.0f) and
    //      then rotate it with 90 degrees around the X axis
    //HINT: the glm::rotate function assumes that the angle is specified in radians
    //      convert the angle from degrees to radians using the glm::radians function
    glm::vec4 TransformPoint(const glm::vec4 &point);

    //TODO: Compute the angle between two vectors
    //HINT: the acos function returns the value in radians
    //      convert the value in degrees using the glm::degrees function
    float ComputeAngle(const glm::vec3 &v1, const glm::vec3 &v2);
    
    //TODO: Test if a polygon defined by its vertices is convex
    //      the vertices are specified in counterclockwise order
    //      return true if the polygon is convex
    //      return false if the polygon is concave
    bool IsConvex(const std::vector<glm::vec2> &vertices);
    
    //TODO: Consider a polygon defined by its vertices (in counterclockwise order),
    //      compute for each edge a vector that is perpendicular to that edge,
    //      and pointing outwards the edge
    //      normalize each such vector
    std::vector<glm::vec2> ComputeNormals(const std::vector<glm::vec2> &vertices);
}

#include "GPSLab1.cpp"

#endif /* GPSLab1_hpp */
