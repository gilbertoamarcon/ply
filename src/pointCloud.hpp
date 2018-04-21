#ifndef __POINTCLOUD_HPP__
#define __POINTCLOUD_HPP__
#include "csv.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <yaml-cpp/yaml.h>
#include "GL/freeglut.h"
#include "GL/gl.h"

using namespace std; 

struct Point{
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
};

class PointCloud{
	public:
		YAML::Node cfg;
		float maxX;
		float maxY;
		float maxZ;
		float minX;
		float minY;
		float minZ;
		float lenX;
		float lenY;
		float lenZ;
		int num_points;
		Point ctroid;
		Point *point;
		PointCloud();
		int file_size();
		int load();
		void write();
		void compute_stats();
		void rotate(float t, float p);
};

#endif
