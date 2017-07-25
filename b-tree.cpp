#include "b-tree.h"

//   > ------------------------------------------------------
NodeData::NodeData() :nextrecOff(0), idxLen(0), key(""), sep(':'), dataOff(0), datLen(0)
{}

//   > ------------------------------------------------------

NodeData::NodeData(const std::streamoff& next, const int& idxlen, const std::string& _key,
	const std::streamoff& datoff, const int& datlen, const char& ch = ':')
	: nextrecOff(next), idxLen(idxlen), key(_key), dataOff(datoff), datLen(datlen), sep(ch)
{}

//   > ------------------------------------------------------
NodeData::NodeData(const NodeData& other) : nextrecOff(other.nextrecOff), idxLen(other.idxLen), 
key(other.key), dataOff(other.dataOff), datLen(other.datLen)
{}

//   > ------------------------------------------------------
NodeData& NodeData::operator=(const NodeData& other)
{
	if (&other == this)return *this;
	nextrecOff = other.nextrecOff;
	idxLen = other.idxLen;
	key = other.key;
	dataOff = other.dataOff;
	datLen = other.datLen;
	return *this;
}

//   > --------------------------------------------------------
NodeData::~NodeData()
{}

//   > --------------------------------------------------------
const std::streamoff& NodeData::get_nextrec_offset() const
{
	return nextrecOff;
}

//   > --------------------------------------------------------
const int& NodeData::get_idx_len() const
{
	return idxLen;
}

//   > --------------------------------------------------------
const std::string& NodeData::get_key() const
{
	return key;
}

//   > --------------------------------------------------------
const char& NodeData::get_sep() const
{
	return sep;
}

//   > ---------------------------------------------------------
const std::streamoff& NodeData::get_data_offset() const
{
	return dataOff;
}

//   > --------------------------------------------------------
const int& NodeData::get_data_len() const
{
	return datLen;
}

//   > ------------------------------------------------------
void NodeData::set_nextrec_offset(const std::streamoff& offset)
{
	nextrecOff = offset;
}

//   > ------------------------------------------------------
void NodeData::set_idx_len(const int& len)
{
	idxLen = len;
}

//   > ------------------------------------------------------
void NodeData::set_key(const std::string& _key)
{
	key = _key;
}

//   > ------------------------------------------------------
void NodeData::set_sep(const char& ch)
{
	sep = ch;
}

//   > ------------------------------------------------------
void NodeData::set_data_offset(const std::streamoff& offset)
{
	dataOff = offset;
}

//   > ------------------------------------------------------
void NodeData::set_data_len(const int& len)
{
	datLen = len;
}

//   > ------------------------------------------------------
void NodeData::write_binary(std::fstream& outfile)
{
	using namespace std;
	outfile.write((char*)&nextrecOff, sizeof(streambuf));
	outfile.write((char*)&idxLen, sizeof(int));
	outfile.write((char*)&key, sizeof(key));
	outfile.write((char*)&sep, sizeof(char));
	outfile.write((char*)&dataOff, sizeof(streamoff));
	outfile.write((char*)&datLen, sizeof(int));
	char lf = '\n';
	outfile.write((char*)&lf, sizeof(char));
}

//   > -------------------------------------------------------
NodeData& NodeData::read_binary(std::fstream& infile)
{
	using namespace std;

	streamoff curr = infile.tellg();
	//streamoff nextrec_off;
	infile.read((char*)&nextrecOff, sizeof(streamoff));
	//int idx_len;
	infile.read((char*)&idxLen, sizeof(int));
	char* key_str = new char[9];
	infile.read(key_str, sizeof(char) * 8);
	key_str[8] = '\0';
	key = key_str;
	delete key_str;
	infile.read((char*)&sep, sizeof(char));
	infile.read((char*)&dataOff, sizeof(streamoff));
	infile.read((char*)&sep, sizeof(char));
	infile.read((char*)&datLen, sizeof(int));
	char lf;
	infile.read((char*)&lf, sizeof(char));
	if (lf != '\n') {
		infile.seekg(curr, ios::beg);
		throw runtime_error("Read index failed");
	}
	infile.seekg(curr, ios::beg);
	return *this;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//   > ------------------------------------------------------
BTreeNode* BTreeNode::split_leaf()
{
	using namespace std;

	BTreeNode *lhc = new BTreeNode(true), *rhc = new BTreeNode(true);
	for (size_t i = 0; i != content.size() / 2; ++i) {
		lhc->content.push_back(content[i]);
	}
	for (size_t i = content.size() / 2; i != content.size(); ++i) {
		rhc->content.push_back(content[i]);
	}
	BTreeNode* ret = new BTreeNode(false);
	ret->content.push_back(rhc->content[0]);
	ret->children.push_back(lhc);
	lhc->set_parent(ret);
	ret->children.push_back(rhc);
	rhc->set_parent(ret);
	return ret;
}

//   > ---------------------------------------------------------
BTreeNode* BTreeNode::split_nonleaf()
{
	using namespace std;
	
	BTreeNode *lhc = new BTreeNode(false), *rhc = new BTreeNode(false);
	BTreeNode *ret = new BTreeNode(false);
	ret->parent = nullptr;
	
	for (size_t i = 0; i != content.size() / 2; ++i) {
		lhc->content.push_back(content[i]);
		lhc->children.push_back(std::move(children[i]));
		children[i] = nullptr;
	}
	lhc->children.push_back(std::move(children[content.size() / 2]));
	children[content.size() / 2] = nullptr;
	for (size_t i = content.size() / 2 + 1; i != content.size(); ++i) {
		rhc->content.push_back(content[i]);
		rhc->children.push_back(std::move(children[i]));
		children[i] = nullptr;
	}
	rhc->children.push_back(std::move(children[content.size()]));
	children[content.size()] = nullptr;
	ret->content.push_back(content[content.size() / 2]);
	ret->children.push_back(lhc);
	ret->children.push_back(rhc);
	return ret;
}

//   > ---------------------------------------------------------
BTreeNode* BTreeNode::merge(BTreeNode* &other)
{
	using namespace std;

	if (other->content.size() != 1) throw runtime_error("merge size too large");

	string& t = other->content[0];
	vector<string>::iterator it = content.begin();
	while (it != content.end() && t > *it) {
		it++;
	}
	size_t pos = it - content.begin();
	content.insert(it, t);

	//   > whether memory LEAK? here
	delete children[pos];
	children[pos] = std::move(other->children[1]);
	other->children[1] = nullptr;
	vector<BTreeNode*>::iterator itc = children.begin() + pos;
	children.insert(itc, std::move(other->children[0]));
	other->children[0] = nullptr;
	delete other;

	return this;
}

//   > -------------------------------------------------------------
/*
 *	merge the current node with another node
 *  side means where to put the content
 */
BTreeNode* BTreeNode::merge(BTreeNode* & other, bool side)
{
	using namespace std;
	//   > side = true -> right
	//   > side = false -> left
	if (side) {
		size_t pos = 0;
		while (pos < parent->content.size() && parent->children[pos] != this) {
			pos++;
		}
		content.push_back(parent->content[pos]);
		for (size_t i = 0; i != other->content.size(); i++) {
			content.push_back(std::move(other->content[i]));
			children.push_back(std::move(other->children[i]));
			other->children[i] = nullptr;
		}
		parent->content.erase(parent->content.begin() + pos);
		parent->children.erase(parent->children.begin() + pos + 1);
	}
	else {
		size_t pos = 0;
		while (pos < parent->content.size() && parent->children[pos] != this) {
			pos++;
		}
		pos--;
		content.insert(content.begin(), parent->content[pos]);
		children.insert(children.begin(), std::move(*(other->children.end() - 1)));
		*(other->children.end() - 1) = nullptr;
		for (size_t i = content.size() - 1; i >= 0; i--) {
			content.insert(content.begin(), std::move(other->content[i]));
			children.insert(children.begin(), std::move(other->children[i]));
			other->children[i] = nullptr;
		}
		parent->content.erase(parent->content.begin() + pos);
		parent->children.erase(parent->children.begin() + pos);
	}
	delete other;
	return this;
}

//   > -------------------------------------------------------------
BTreeNode* BTreeNode::leaf_insert(const std::string& t, size_t& maxSize)
{
	using namespace std;

	size_t pos = 0;
	while (pos != content.size() && t > content[pos]) pos++;
	if (pos != 0 && content[pos - 1] == t) {
		throw runtime_error("already stored.");
	}
	vector<string>::iterator it = content.begin() + pos;
	content.insert(it, t);
	if (content.size() >= maxSize) {
		BTreeNode* res = split_leaf();
		if (parent == nullptr) {
			delete this;
			return res;
		}
		else parent->merge(res);
		return parent;
	}
	return this;
}

//   > -----------------------------------------------------------
BTreeNode* BTreeNode::leaf_insert(const NodeData& nd, size_t& maxSize)
{
	using namespace std;

	const string& t = nd.get_key();
	size_t pos = 0;
	while (pos != content.size() && t > content[pos]) pos++;
	if (pos != 0 && content[pos - 1] == t) {
		throw runtime_error("already stored.");
	}
	vector<string>::iterator it = content.begin() + pos;
	vector<NodeData>::iterator jt = idxdata.begin() + pos;
	content.insert(it, t);
	idxdata.insert(jt, nd);
	if (content.size() >= maxSize) {
		BTreeNode* res = split_leaf();
		if (parent == nullptr) {
			delete this;
			return res;
		}
		else return parent->merge(res);
	}
	return this;
}

//   > -----------------------------------------------------------
BTreeNode* BTreeNode::nonleaf_insert(const std::string& t, size_t& maxSize)
{
	using namespace std;

	size_t pos = 0;
	while (pos != content.size() && t >= content[pos]) pos++;
	BTreeNode* child = children[pos];
	if (child->leaf) child->leaf_insert(t, maxSize);
	else child->nonleaf_insert(t, maxSize);
	if (content.size() >= maxSize) {
		BTreeNode* res = split_nonleaf();
		if (parent == nullptr) {
			delete this;
			return res;
		}
		return parent->merge(res);
	}
	return this;
}

//   > ----------------------------------------------------------
BTreeNode* BTreeNode::nonleaf_insert(const NodeData& nd, size_t& maxSize)
{
	using namespace std;

	const string& t = nd.get_key();
	size_t pos = 0;
	while (pos != content.size() && t >= content[pos]) pos++;
	BTreeNode* child = children[pos];
	if (child->leaf) child->leaf_insert(nd, maxSize);
	else child->nonleaf_insert(nd, maxSize);
	if (content.size() > maxSize) {
		BTreeNode* res = split_nonleaf();
		if (parent == nullptr) {
			delete this;
			return res;
		}
		return parent->merge(res);
	}
	return this;
}

//   > ----------------------------------------------
BTreeNode::BTreeNode() : parent(nullptr), leaf(true)
{}

//   > -----------------------------------------------
BTreeNode::BTreeNode(const BTreeNode& other) : content(other.content),
children(other.children), parent(other.parent), leaf(other.leaf)
{}

//   > -------------------------------------------------
BTreeNode::BTreeNode(bool _leaf) : leaf(_leaf), parent(nullptr)
{}

//   > -----------------------------------------------
BTreeNode& BTreeNode::operator=(const BTreeNode &rhs)
{
	this->content = rhs.content;
	this->children = rhs.children;
	this->parent = rhs.parent;
	this->leaf = rhs.leaf;
	return *this;
}

//   > -------------------------------------------------
BTreeNode& BTreeNode::set_parent(BTreeNode* _parent)
{
	this->parent = _parent;
	return *this;
}

//   > -------------------------------------------------
/*
 *	const string& predecessor(const string& curr)
 *  get the predecessor of current node
 */
const std::string& BTreeNode::predecessor(const std::string& curr)
{
	using namespace std;

	if (this->leaf) {
		/*
		vector<string>::const_iterator it = children.cbegin();
		while (it != children.cend() && *it > curr) {
			it++;
		}
		if (it == children.cend()) {
			if (parent != nullptr) {
				if (parent->parent != nullptr) {
					size_t i = 0;
					for (; i != parent->parent->children.size(); ++i) {
						if (parent->parent->children[i] == parent) break;
					}
				}
			}
		}
		*/
		return "leaf node";
	}
	else {
		for (size_t pos = 0; pos != content.size(); ++pos) {
			if (content[pos] == curr) {
				BTreeNode* child = children[pos + 1];
				while (!child->leaf) {
					child = child->children[0];
				}
				return child->content[1];
			}
		}
		throw runtime_error(curr + "Not found");
	}
}

/*  -----------------------------------------------------------
 *	bool del(std::string& t)
 *  delete t from B+tree
 */
BTreeNode* BTreeNode::del(std::string& t, const size_t &maxSize)
{
	using namespace std;

	//   > leaf node
	if (leaf) {
		bool found = false;
		for (vector<string>::iterator it = content.begin(); it != content.end(); ++it) {
			if (*it == t) {
				content.erase(it);
				found = true;
				break;
			}
		}
		if (found) {
			if (content.size() >= maxSize / 2) return this;
			else {
				//   > content is too small
				//   > if parent is not empty
				//   > do merge
				if (parent != nullptr) {
					//   > this is the most left child of parent
					if (this == parent->children[0]) {
						//   > if right sibling is rich
						if (parent->children[1]->content.size() >= maxSize / 2 + 1) {
							content.push_back(parent->content[0]);
							parent->children[1]->content.erase(parent->children[1]->content.begin());
							parent->content[0] = parent->children[1]->content[0];
							return this;
						}
						else {
							for (vector<string>::iterator it = parent->children[1]->content.begin(); 
								it != parent->children[1]->content.end(); it++) {
								content.push_back(*it);
							}
							parent->content.erase(parent->content.begin());
							delete parent->children[1];
							parent->children.erase(parent->children.begin() + 1);
							//if (parent->content.empty()))
							return this;
						}
					}

					//   > this is the most right child of parent
					else if (this == *(parent->children.end())) {
						vector<BTreeNode*>& kids = parent->children;
						BTreeNode* &leftSibling = kids[kids.size() - 2];
						//   > if left sibling is rich
						//   > take one from sibling
						if (leftSibling->content.size() >= maxSize / 2 + 1) {
							content.insert(content.begin(), *(leftSibling->content.end() - 1));
							leftSibling->content.erase(leftSibling->content.end() - 1);
							parent->content[parent->content.size() - 1] = *content.begin();
							return this;
						}
						//   > if left sibling is poor
						//   > merge
						else {
							vector<string> tmp(leftSibling->content.begin(), leftSibling->content.end());
							for (vector<string>::iterator it = content.begin(); it != content.end(); ++it) {
								tmp.push_back(*it);
							}
							content = tmp;
							delete leftSibling;
							parent->children.erase(parent->children.end() - 2);
							parent->content.erase(parent->content.end() - 1);
							return this;
						}
					}

					//   > this is one of the middle children of the parent
					else {
						vector<BTreeNode*>& kids = parent->children;
						size_t pos = 0;
						while (pos != kids.size() && kids[pos] != this) pos++;
						BTreeNode *leftSibling(kids[pos - 1]), *rightSibling(kids[pos + 1]);
						
						//   > if left sibling is rich than right sibling
						if (leftSibling->content.size() > rightSibling->content.size()) {
							BTreeNode* sb = leftSibling;
							//   > if left sibling is quite rich
							if (sb->content.size() > maxSize / 2) {
								content.insert(content.begin(), *(sb->content.end() - 1));
								parent->content[pos - 1] = *(sb->content.end() - 1);
								sb->content.erase(sb->content.end() - 1);
								return this;
							}
							//   > if left sibling is just-so-so
							else {
								vector<string> tmp(sb->content.begin(), sb->content.end());
								for (vector<string>::iterator it = content.begin(); it != content.end(); ++it) {
									tmp.push_back(*it);
								}
								this->content = tmp;
								delete sb;
								parent->children.erase(parent->children.begin() + pos - 1);
								parent->content.erase(parent->content.begin() + pos - 1);
								return this;
							}
						}
						else {
							BTreeNode* sb = rightSibling;
							if (sb->content.size() > maxSize / 2) {
								content.push_back(sb->content[0]);
								sb->content.erase(sb->content.begin());
								parent->content[pos] = sb->content[0];
								return this;
							}
							else {
								for (vector<string>::iterator it = sb->content.begin(); it != sb->content.end(); ++it) {
									content.push_back(*it);
								}
								delete sb;
								parent->children.erase(parent->children.begin() + pos + 1);
								parent->content.erase(parent->content.begin() + pos);
								return this;
							}
						}
						
					}
				}
				//   > parent is nullptr
				else return this;
			}
		}
		else {
			throw runtime_error("Not found");
		}
	}
	//   > nonleaf node
	else {
		bool found = false;
		size_t pos = 0;
		while (pos < content.size() && content[pos] <= t) {
			//   > if t is found in content
			//   > replace t with its predecessor
			if (content[pos] == t) {
				found = true;
				content[pos] = predecessor(content[pos]);
				break;
			}
		}
		found = children[pos]->del(t, maxSize);
		//   > check the remain size
		if (content.size() >= maxSize / 2) {
			return this;
		}
		else {
			size_t i = 0;

			if (parent == nullptr) {
				if (content.size() == 0) {
					return children[0];
				}
				else {
					return this;
				}
			}

			for (; i != parent->children.size(); i++) {
				if (parent->children[i] == this) break;
			}
			if (i == 0) {
				merge(parent->children[i + 1], true);
			}
			else if (i == parent->children.size() - 1) {
				merge(parent->children[i - 1], false);
			}
			else {
				BTreeNode* &leftSibling = parent->children[i - 1];
				BTreeNode* & rightSibling = parent->children[i + 1];
				if (leftSibling->content.size() > rightSibling->content.size()) {
					merge(leftSibling, false);
				}
				else {
					merge(rightSibling, true);
				}
			}
			return this;
		}
	}
}

//   > -----------------------------------------------------------
const NodeData& BTreeNode::search(const std::string& t)
{
	using namespace std;

	if (leaf) {
		for (vector<string>::iterator it = content.begin(); it != content.end(); ++it) {
			if (*it == t) {
				return idxdata[it-content.begin()];
			}
		}
		throw runtime_error("Not found");
	}
	else {
		size_t i = 0;
		while (i != content.size() && t >= content[i]) {
			i++;
		}
		return children[i]->search(t);
	}
}

//   > -----------------------------------------------------------
std::string BTreeNode::traverse()
{
	using namespace std;

	ostringstream oss;
	if (leaf) {
		for (vector<string>::const_iterator it = content.cbegin(); it != content.cend(); ++it) {
			oss << *it << " ";
		}
	}
	else {
		for (size_t i = 0; i != content.size(); i++) {
			oss << children[i]->traverse() << " " << content[i] << " ";
		}
		oss << children[content.size()]->traverse() << " ";
	}
	return oss.str();
}

//   > -----------------------------------------------------------
//   >destructor
BTreeNode::~BTreeNode()
{
	using namespace std;

	for (vector<BTreeNode*>::iterator it = children.begin(); it != children.end(); ++it) {
		delete *it;
	}
	parent = nullptr;
}

/*----------------------------------------------------------------------*/

BPTree::BPTree() : root(nullptr), maxSize(3)
{}

//   > --------------------------------------------------------------
BPTree::BPTree(const BPTree& other) : root(other.root), maxSize(other.maxSize)
{}

//   > ---------------------------------------------------------------
BPTree::BPTree(BTreeNode* _root, size_t _size)
	:root(_root), maxSize(_size) 
{}

//   > -----------------------------------------------------------------
BPTree& BPTree::operator=(const BPTree& rhs)
{
	this->root = rhs.root;
	this->maxSize = rhs.maxSize;
	return *this;
}

//   > ----------------------------------------------------------------
BPTree& BPTree::set_size(size_t _size)
{
	this->maxSize = _size;
	return *this;
}

//    > ---------------------------------------------------------------
BPTree& BPTree::insert(const std::string& t, const std::streamoff& offset)
{
	if (root == nullptr) {
		root = new BTreeNode(true);
		root->leaf_insert(t, maxSize);
	}
	else {
		if (root->leaf) {
			BTreeNode* res = root->leaf_insert(t, maxSize);
			root = res;
		}
		else {
			root = root->nonleaf_insert(t, maxSize);
		}
	}
	return *this;
}

//   > ---------------------------------------------------------------------------
BPTree& BPTree::insert(const NodeData& nd)
{
	if (root == nullptr) {
		root = new BTreeNode(true);
		root->leaf_insert(nd.get_key(), maxSize);
	}
	else {
		if (root->leaf) {
			BTreeNode* res = root->leaf_insert(nd.get_key(), maxSize);
			root = res;
		}
		else {
			root = root->nonleaf_insert(nd.get_key(), maxSize);
		}
	}
	return *this;
}

//   > ---------------------------------------------------------------------------
bool BPTree::del(std::string& t, const size_t& maxSize)
{
	using namespace std;
	if (root != nullptr) {
		bool flag = root->del(t, maxSize);
		if (!flag) throw std::runtime_error("Not found");
		return flag;
	}
	else throw runtime_error("empty tree");
}

//   > ------------------------------------------------------------------------
const NodeData& BPTree::search(const std::string& t)
{
	using namespace std;
	if (root != nullptr) {
		return root->search(t);
	}
	else throw runtime_error("empty tree");
}

//   > -------------------------------------------------------------------------
std::string BPTree::traverse()
{
	//   > pre-order traverse
	using namespace std;

	ostringstream oss;
	for (size_t i = 0; i != root->content.size(); ++i) {
		oss << root->children[0]->traverse() << " " << root->content[0] << " ";
	}
	oss << root->children[root->content.size()]->traverse();
	return oss.str();
}