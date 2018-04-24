#include "H5-utils.hpp"

// H5G_LINK    0  Object is a symbolic link.  
// H5G_GROUP   1  Object is a group.  
// H5G_DATASET 2  Object is a dataset.  
// H5G_TYPE    3  Object is a named datatype.  
herr_t H5utils::load_datasets(hid_t loc_id, const char *name, void *opdata){
	std::pair<Node*,H5::H5File*> *pair=(std::pair<Node*,H5::H5File*> *)opdata;
	Node *node = new Node();
	node->addr = pair->first->addr+name;
	pair->first->children[std::string(name)] = node;
	if(H5Gget_objtype_by_idx(loc_id,0) == H5G_GROUP){
		node->addr+="/";
		std::pair<Node*,H5::H5File*> aux(node,pair->second);
		pair->second->iterateElems(node->addr, NULL, H5utils::load_datasets, &aux);
	}
	if(H5Gget_objtype_by_idx(loc_id,0) == H5G_DATASET){
		H5::DataSet dataset = pair->second->openDataSet(node->addr);
		H5::DataSpace dataspace = dataset.getSpace();
		int rank = dataspace.getSimpleExtentNdims();
		hsize_t *dims_out = new hsize_t[rank];
		dataspace.getSimpleExtentDims(dims_out, NULL);
		node->data.resize(dims_out[0]);
		dataset.read((void*)(&(node->data[0])), dataset.getDataType());
		dataset.close();
	}
	return 0;
}

H5utils::H5utils(char *filename){
	H5::H5File *file = new H5::H5File(filename, H5F_ACC_RDONLY);
	this->node = new Node();
	std::pair<Node*,H5::H5File*> aux(this->node,file);
	file->iterateElems("/", NULL, H5utils::load_datasets, &aux);
	file->close();
}

std::vector<double>& H5utils::get(std::string type, std::string name, std::string variable){
	return this->node->children[type]->children[name]->children[variable]->data;
}

std::vector<std::string> H5utils::get(std::string type, std::string name){
	std::vector<std::string> names;
	for(auto const& imap:this->node->children[type]->children[name]->children)
		names.push_back(imap.first);
	return names;
}

std::vector<std::string> H5utils::get(std::string type){
	std::vector<std::string> names;
	for(auto const& imap:this->node->children[type]->children)
		names.push_back(imap.first);
	return names;
}

std::vector<std::string> H5utils::get(){
	std::vector<std::string> names;
	for(auto const& imap:this->node->children)
		names.push_back(imap.first);
	return names;
}