#include "pointCloud.hpp"
#define BUFFER_SIZE 256
#define CLOUD_CFG_PATH "cfg/cloud.yaml"

PointCloud::PointCloud(){
	maxX			= -1e9;
	maxY			= -1e9;
	maxZ			= -1e9;
	minX			=  1e9;
	minY			=  1e9;
	minZ			=  1e9;
	lenX			=  0;
	lenY			=  0;
	lenZ			=  0;
	num_points		=  0;
}

int PointCloud::file_size(){
	io::CSVReader<6> in(cfg["cloud_filename"].as<string>().c_str());
	in.read_header(io::ignore_extra_column, "x", "y", "z", "r", "g", "b");
	double x; double y; double z; double r; double g; double b;
	int num_file_points = 0;
	while(in.read_row(x, y, z, r, g, b))
		num_file_points++;
	return num_file_points;
}

int PointCloud::load(){

	cfg = YAML::LoadFile(CLOUD_CFG_PATH);

	cout << "Processing file '" << cfg["cloud_filename"].as<string>() << "'" << endl;
	int num_file_points = this->file_size();

	cout << num_file_points << " points found." << endl;

	io::CSVReader<6> in(cfg["cloud_filename"].as<string>().c_str());
	in.read_header(io::ignore_extra_column, "x", "y", "z", "r", "g", "b");

	// Data Parsing
	point = new Point[num_file_points];
	num_points = 0;
	while(in.read_row(
		point[num_points].x,
		point[num_points].y,
		point[num_points].z,
		point[num_points].r,
		point[num_points].g,
		point[num_points].b
	))	num_points++;
	cout << num_points << " points loaded." << endl;
	return 0;
}

void PointCloud::write(){

	cfg = YAML::LoadFile(CLOUD_CFG_PATH);

	cout << "Storing file '" << cfg["cloud_filename"].as<string>() << "'" << endl;

	ofstream file;
	file.open(cfg["cloud_filename"].as<string>());
	file << "x,y,z,r,g,b" << endl;
	for(int i = 0; i < num_points; i++)
		file << point[i].x << "," << point[i].y << "," << point[i].z << "," << point[i].r << "," << point[i].g << "," << point[i].b << endl;
	file.close();
	cout << num_points << " points stored." << endl;
}

void PointCloud::compute_stats(){
	ctroid.x = 0;
	ctroid.y = 0;
	ctroid.z = 0;
	for (int i = 0; i < num_points; i++){
		if(point[i].x > maxX)
			maxX = point[i].x;
		if(point[i].y > maxY)
			maxY = point[i].y;
		if(point[i].z > maxZ)
			maxZ = point[i].z;
		if(point[i].x < minX)
			minX = point[i].x;
		if(point[i].y < minY)
			minY = point[i].y;
		if(point[i].z < minZ)
			minZ = point[i].z;
		ctroid.x += point[i].x;
		ctroid.y += point[i].y;
		ctroid.z += point[i].z;
	}
	lenX = maxX - minX;
	lenY = maxY - minY;
	lenZ = maxZ - minZ;
	ctroid.x /= num_points;
	ctroid.y /= num_points;
	ctroid.z /= num_points;
}

void PointCloud::rotate(float t, float p){
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
	for (int i = 0; i < num_points; i++){
		x = point[i].x;
		y = point[i].y;
		z = point[i].z;
		point[i].x = x*cos(p) + z*sin(p);
		point[i].y = x*sin(t)*sin(p) + y*cos(t) -z*sin(t)*cos(p);
		point[i].z = -x*sin(p)*cos(t) + y*sin(t) + z*cos(t)*cos(p);
	}
	compute_stats();
}

