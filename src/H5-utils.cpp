#include "H5-utils.hpp"

// H5G_LINK    0  Object is a symbolic link.  
// H5G_GROUP   1  Object is a group.  
// H5G_DATASET 2  Object is a dataset.  
// H5G_TYPE    3  Object is a named datatype.  
herr_t H5utils::objs_list(hid_t loc_id, const char *name, void *opdata){
	std::map<std::string,Node*> *children=(std::map<std::string,Node*> *)opdata;
	Node *node = new Node();
	node->name = name;
	node->type = H5Gget_objtype_by_idx(loc_id,0);
	(*children)[std::string(name)] = node;
	return 0;
}

void H5utils::iterate(H5::H5File *file, Node *node){
	if(node->type == H5G_GROUP){
		node->addr+="/";
		file->iterateElems(node->addr, NULL, H5utils::objs_list, &(node->children));
		for(auto child:node->children){
			child.second->addr = node->addr+child.first;
			H5utils::iterate(file, child.second);
		}
	}
	if(node->type == H5G_DATASET){
		H5::DataSet dataset = file->openDataSet(node->addr);
		H5::DataSpace dataspace = dataset.getSpace();
		int rank = dataspace.getSimpleExtentNdims();
		hsize_t *dims_out = new hsize_t[rank];
		dataspace.getSimpleExtentDims(dims_out, NULL);
		node->size = dims_out[0];
		node->data = new double[node->size+1];
		node->data_type = dataset.getDataType();
		dataset.read((void*)(node->data), node->data_type);
		dataset.close();
	}
}

H5utils::H5utils(char *filename){
	H5::H5File *file = new H5::H5File(filename, H5F_ACC_RDONLY);
	H5utils::iterate(file,&node);
	file->close();
}
