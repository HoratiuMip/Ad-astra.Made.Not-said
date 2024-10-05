/*
*/
//
//  main.cpp
//  Lab1
//
//  Copyright © 2017 CGIS. All rights reserved.
//

#include <conio.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Lab-resources/Lab-1/GPSLab1.hpp"

void TestTransformPoint()
{
	glm::vec4 correctResult(3.0f, -1.0f, 1.0f, 1.0f);

	glm::vec4 result = gps::TransformPoint(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

	std::cout << "Test TransformPoint()" << std::endl;
	std::cout << "  correct result: " << correctResult.x <<" "<< correctResult.y << " " << correctResult.z<< " " << correctResult.w << std::endl;
	std::cout << "  your result:    " << result.x <<" " << result.y << " " << result.z << " " << result.w << " " << std::endl << std::endl;
}

void TestComputeAngle()
{
	glm::vec3 v1(2.0f, 1.0f, 0.0f);
	glm::vec3 v2(0.0f, 2.0f, 1.0f);

	float angle = gps::ComputeAngle(v1, v2);

	std::cout << "Test ComputeAngle()" << std::endl;
	std::cout << "  correct result: " << "66.4218" << std::endl;
	std::cout << "  your result:    " << angle << std::endl << std::endl;
}

void TestIsConvex()
{
	glm::vec2 v1(0.0f, 0.0f);
	glm::vec2 v2(1.0f, 0.0f);
	glm::vec2 v3(0.5f, 0.5f);
	glm::vec2 v4(1.0f, 1.0f);
	glm::vec2 v5(0.0f, 1.0f);

	std::vector<glm::vec2> vertices;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	vertices.push_back(v5);

	std::cout << "Test IsConvex()" << std::endl;
	std::cout << "  correct result: " << "false" << std::endl;
	std::cout << "  your result:    " << std::boolalpha << gps::IsConvex(vertices) << std::endl << std::endl;
}

void TestComputeNormals()
{
	glm::vec2 v1(0.0f, 0.0f);
	glm::vec2 v2(2.0f, 0.0f);
	glm::vec2 v3(2.0f, 2.0f);
	glm::vec2 v4(0.0f, 2.0f);

	std::vector<glm::vec2> vertices;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);

	std::vector<glm::vec2> result = gps::ComputeNormals(vertices);

	glm::vec2 n1(0.0f, -1.0f);
	glm::vec2 n2(1.0f, 0.0f);
	glm::vec2 n3(0.0f, 1.0f);
	glm::vec2 n4(-1.0f, 0.0f);

	std::vector<glm::vec2> correctResult;
	correctResult.push_back(n1);
	correctResult.push_back(n2);
	correctResult.push_back(n3);
	correctResult.push_back(n4);

	std::cout << "Test ComputeNormals()" << std::endl;
	std::cout << "  correct result: " << std::endl;
	for (auto normal : correctResult)
		std::cout << "      " << normal.x <<" "<< normal.y << std::endl;
	std::cout << std::endl;

	std::cout << "  your result:    " << std::endl;
	for (auto normal : result)
		std::cout << "      " << normal.x <<" "<< normal.y << std::endl;
	std::cout << std::endl;
}

int main(int argc, const char * argv[]) {

	TestTransformPoint();
	TestComputeAngle();
	TestIsConvex();
	TestComputeNormals();

	_getch();
	return 0;
}
