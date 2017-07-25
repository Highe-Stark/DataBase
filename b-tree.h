#pragma once
#ifndef _B-TREE_H
#define _B-TREE_H

#include <vector>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include <map>

class NodeData
{
private:
	std::streamoff nextrecOff;   //next record offset
	/* if database is quite large, int may cause some problem. */
	int idxLen;   //piece of index length
	std::string key;
	char sep;
	std::streamoff dataOff;
	int datLen;

public:
	NodeData();
	NodeData(const std::streamoff&, const int&, const std::string&, const std::streamoff&, const int&, const char& ch = ':');
	NodeData(const NodeData& other);
	NodeData& operator=(const NodeData& other);
	~NodeData();
	const std::streamoff& get_nextrec_offset() const;
	const int& get_idx_len() const;
	const std::string& get_key() const;
	const char& get_sep() const;
	const std::streamoff& get_data_offset() const;
	const int& get_data_len() const;
	void set_nextrec_offset(const std::streamoff&);
	void set_idx_len(const int&);
	void set_key(const std::string&);
	void set_sep(const char&);
	void set_data_offset(const std::streamoff&);
	void set_data_len(const int&);
	void write_binary(std::fstream& outfile);
	NodeData& read_binary(std::fstream& infile);
};

class BTreeNode
{
private:
	std::vector<std::string> content;
	std::vector<BTreeNode*> children;
	BTreeNode* parent;
	bool leaf;
	std::vector<NodeData> idxdata;

	//   > split leaf node 
	//   > return the parent node of the two splited nodes
	
	/* >> split idxdata NOT implemented yet << */
	BTreeNode* split_leaf();

	//   > split nonleaf node
	//   > return the raised up new node

	/* >> check whether need modification << */
	BTreeNode* split_nonleaf();

	//   > merge the current content with the raised node
	//   > move the raised node's children and free the raised node
	//   > after the merge
	//   > current content may need split too

	/* >> merge idxdata NOT implemented yet << */
	BTreeNode* merge(BTreeNode* &other);

	//   > merge the current node with another node

	/* >> merge idxdata NOT implemented yet << */
	BTreeNode* merge(BTreeNode* &other, bool side);

	//   > insert t into a leaf
	//   > return the position where t is inserted
	//   > 
	BTreeNode* leaf_insert(const std::string& t, size_t& maxSize);
	BTreeNode* leaf_insert(const NodeData& nd, size_t & maxSize);

	//   > insert t into a nonleaf node
	//   > return the position where t is inserted
	BTreeNode* nonleaf_insert(const std::string& t, size_t& maxSize);
	BTreeNode* nonleaf_insert(const NodeData& nd, size_t & maxSize);

public:
	//   > construct a node
	BTreeNode();

	//   > construct a node whose leaf status is _leaf
	BTreeNode(bool _leaf);

	//   > copy constructor
	BTreeNode(const BTreeNode& other);

	//   > assignment
	BTreeNode& operator=(const BTreeNode &rhs);
	 
	//   > set parent of the current node
	BTreeNode& set_parent(BTreeNode* _parent);

	//   > predecessor of current value
	const std::string& predecessor(const std::string&);

	//   > insert t into a node
	//   > after insertation 
	//   > if content's size is large than maxSize DO split
	//   > return
	//BTreeNode<std::string>& insert(std::string& t, const size_t& maxSize);
	BTreeNode* del(std::string& t, const size_t& maxSize);

	const NodeData& search(const std::string& t);

	std::string traverse();

	~BTreeNode();

	friend class BPTree;
};


class BPTree
{
private:
	BTreeNode* root;
	size_t maxSize;

public:
	BPTree();
	BPTree(const BPTree&);
	BPTree(BTreeNode* _root, size_t _size);
	BPTree& operator=(const BPTree& rhs);
	BPTree& set_size(size_t _size);
	//   >> need change <<
	BPTree& insert(const std::string&, const std::streamoff&);
	BPTree& insert(const NodeData& nd);
	bool del(std::string& t, const size_t& maxSize);
	const NodeData& search(const std::string& t);
	std::string traverse();
};

#endif