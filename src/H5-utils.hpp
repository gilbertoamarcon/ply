#ifndef __H5_UTILS_HPP__
#define __H5_UTILS_HPP__
#include <string>
#include <utility>
#include <vector>
#include <map>
#include "H5Cpp.h"

// H5G_LINK    0  Object is a symbolic link.  
// H5G_GROUP   1  Object is a group.  
// H5G_DATASET 2  Object is a dataset.  
// H5G_TYPE    3  Object is a named datatype.  

typedef struct Node{
	std::string addr;
	std::vector<double> data;
	std::map<std::string,Node*> children;
	Node(){
		this->addr = "/";
	}
};


class H5utils{
	private:
		Node *node;
		static herr_t load_datasets(hid_t loc_id, const char *name, void *opdata);
	public:
		H5utils(char *filename);
		std::vector<double>& get(std::string type, std::string name, std::string variable);
		std::vector<std::string> get(std::string type, std::string name);
		std::vector<std::string> get(std::string type);
		std::vector<std::string> get();
};

#endif
