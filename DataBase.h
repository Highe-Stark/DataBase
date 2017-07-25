#pragma once
#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "b-tree.h"
#include <map>

class Database
{
private:
	std::string pathname;
	std::fstream fidx, fdat;
	std::string passwd;
	//   > level limits the access of user
	enum Level { readonly, read_write };
	Level accessLevel;
	std::map<std::streamoff, int> spareSpace;
	BPTree idx;
	int keySize = 8;

	/*
	 * private
	 * get_passwd(ifstream& _ifdat)
	 * Usage: get password from .dat file
	 * Memo: password is intended to protect database from arbitry modification
	 */
	const std::string get_passwd(std::fstream& _ifdat);

	/*
	 * private
	 * build_idx(ifstream& _ifidx)
	 * Usage: build index tree from .idx file
	 */
	void build_idx(std::fstream& _ifidx);

	/*
	 * private
	 * build_spare(ifstream& _ifidx)
	 * Usage: build spare space list from .idx file
	 */
	void build_spare(std::fstream& _ifidx);

	/*
	 * private
	 * streamoff last_idx(fstream& _fidx)
	 * offset of the last idx in file stream
	 */
	NodeData& last_idx(fstream& _fidx);

public:
	/*
	 * Database default constructor
	 */
	Database();

	/*
	 * Database constructor
	 * Database(const std::string& path)
	 * construct database with database path
	 */
	Database(const std::string& path);

	/*
	 * bool Database::open(const std::string& path)
	 * open database in the path
	 * if open successfully, return true
	 * otherwise return false
	 */
	bool open(const std::string& path);

	/*
	 * bool Database::close()
	 * close database
	 * if close successfully, return true
	 * otherwise return false
	 */
	bool close();

	/*
	 * Database method -> auto fetch(const std::string& key)
	 * fetch record by key
	 */
	const std::string& fetch(const std::string& key);

	/*
	 * bool store(const std::string& key, T& t)
	 * store a piece of record
	 * return true if stored successfully
	 * otherwise false
	 */
	bool store(const std::string& key, const std::string& t);

	/*
	 * bool del(const T& t)
	 * delete record t
	 * if delete successfully, return true
	 * otherwise return false
	 */
	template <typename T>
	bool del(const T& t);

	/*
	 * void check()
	 * check the usage of memory
	 */
	void check();

	/*
	 * Destructor
	 */
	~Database();
};

#endif