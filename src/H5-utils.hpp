#ifndef __H5_UTILS_HPP__
#define __H5_UTILS_HPP__
#include <string>
#include <map>
#include "H5Cpp.h"

// H5G_LINK    0  Object is a symbolic link.  
// H5G_GROUP   1  Object is a group.  
// H5G_DATASET 2  Object is a dataset.  
// H5G_TYPE    3  Object is a named datatype.  

typedef struct Node{
	std::string addr;
	std::string name;
	int type;
	double *data;
	long int size;
	H5::DataType data_type;
	std::map<std::string,Node*> children;
	Node(){
		this->addr = "";
		this->name = "";
		this->type = H5G_GROUP;
		this->data = NULL;
		this->size = 0;
	}
};


class H5utils{
	private:
		static herr_t objs_list(hid_t loc_id, const char *name, void *opdata);
		static void iterate(H5::H5File *file, Node *node);
	public:
		Node node;
		H5utils(char *filename);
};

#endif
