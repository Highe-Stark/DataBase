#include "DataBase.h"

using namespace std;

const string Database::get_passwd(fstream& _ifdat)
{
	_ifdat.seekg(ios::beg);
	int length;
	// get real length of password
	_ifdat.read((char*)&length, sizeof(int));
	// get password
	char* passwd_str = new char[length + 1];
	_ifdat.read(passwd_str, sizeof(char)*length);
	// done, reset infstream position
	_ifdat.seekg(ios::beg);
	// change password into string
	// free tempapory memory
	passwd_str[length] = '\0';
	string ret(passwd_str);
	delete passwd_str;
	return ret;
}

void Database::build_idx(fstream& _ifidx)
{
	streamoff offsetIdx;
	_ifidx.seekg(ios::beg);
	_ifidx.read((char*)&offsetIdx, sizeof(streamoff));
	//   > if no index exists, empty database
	if (offsetIdx == 0) {
		_ifidx.seekg(0, ios::beg);
		return;
	}
	//   > if not empty database
	_ifidx.seekg(offsetIdx, ios::beg);
	NodeData nodat;
	try {	//   > get a piece of index and store it
			//   > use string as key
		nodat.read_binary(_ifidx);
		idx.insert(nodat);
		//   > if next record exists, continue read
		while (nodat.get_nextrec_offset() != 0) {
			_ifidx.seekg(nodat.get_nextrec_offset(), ios::cur);
			nodat.read_binary(_ifidx);
			idx.insert(nodat);
		}
	}
	catch (runtime_error err) {
		cout << err.what() << " " << "Aborting reading " + pathname + ".idx file" << endl;
	}
	_ifidx.seekg(0, ios::beg);
	return;
}

void Database::build_spare(fstream& _ifidx)
{
	streamoff spare_off;
	_ifidx.seekg(sizeof(streamoff), ios::beg);
	_ifidx.read((char*)&spare_off, sizeof(streamoff));
	//   > if no spare space
	if (spare_off == 0) {
		_ifidx.seekg(0, ios::beg);
		return;
	}
	_ifidx.seekg(spare_off, ios::beg);
	streamoff offset;
	int len;
	_ifidx.read((char*)&offset, sizeof(streamoff));
	_ifidx.read((char*)&len, sizeof(int));
	while (offset != 0) {
		spareSpace.insert(pair<streamoff, int>(offset, len));
		_ifidx.read((char*)&offset, sizeof(streamoff));
		_ifidx.read((char*)&len, sizeof(int));
	}
	_ifidx.seekg(0, ios::beg);
	return;
}

// ------------------------------------------------------------------
NodeData& Database::last_idx(fstream& _fidx)
{
	_fidx.seekg(0, ios::beg);
	streamoff idxOff;
	_fidx.read((char*)&idxOff, sizeof(streamoff));
	if (idxOff == 0) {
		return NodeData();
	}
	else {
		streamoff pos = 0;
		_fidx.seekg(idxOff, ios::beg);
		NodeData tmp;
		tmp.read_binary(_fidx);
		if (tmp.get_key() !="UNKNOWN")
			pos = _fidx.tellg();
		while (tmp.get_nextrec_offset() != 0) {
			tmp.read_binary(_fidx);
			if (tmp.get_key != "UNKNOWN") pos = _fidx.tellg();
		}
		_fidx.seekg(pos, ios::beg);
		return tmp;
	}
}

// -------------------------------------------------------------------

Database::Database() :pathname(""), passwd(""), accessLevel(read_write)
{}

Database::Database(const string& path) : pathname(path), fidx(path + ".idx"), fdat(path + ".dat")
{
	if (!fidx || !fdat) {
		fidx.close();
		fdat.close();
		fidx.open(path + ".idx", ios::in | ios::out | ios::trunc | ios::binary);
		fdat.open(path + ".dat", ios::in | ios::out | ios::trunc | ios::binary);
		streamoff tmp_offset = 0;
		fidx.write((char*)&tmp_offset, sizeof(streamoff));
		fidx.write((char*)&tmp_offset, sizeof(streamoff));
		int len = 0;
		passwd = "";
		fdat.write((char*)&len, sizeof(int));
		fdat.write(passwd.c_str(), sizeof(char) * 16);
		fidx.close();
		fdat.close();
		fidx.open(path + ".idx", ios::in | ios::out | ios::trunc | ios::binary);
		fdat.open(path + ".dat", ios::in | ios::out | ios::trunc | ios::binary);
		cout << "Database not found in the path, created successfully." << endl;
		accessLevel = read_write;
	}
	else {
		if (!fidx.is_open() || !fdat.is_open()) {
			throw runtime_error("file open failed.");
		}
		passwd = get_passwd(fidx);
		accessLevel = readonly;
		build_idx(fidx);
		build_spare(fidx);
	}
}

bool Database::open(const std::string& path)
{
	fidx.open(path + ".idx", ios::binary | ios::in);
	fdat.open(path + ".dat", ios::binary | ios::in);
	if (!fidx || !fdat) {
		cerr << "Database not found" << endl;
		return false;
	}
	accessLevel = readonly;
	passwd = get_passwd(fdat);
	build_idx(fidx);
	build_spare(fidx);
	cout << "Mode select: " << "\n" << "1 readonly\n" << "2 read & write" << endl;
	cout << "Enter number" << endl;
	int tmp;
	cin >> tmp;
	while (cin.fail()) {
		cin.clear();
		cin.ignore(1024, '\n');
		cout << "Bad Input! try again" << endl;
		cin >> tmp;
	}
	if (tmp == 2) {
		string getcode;
		cout << "password: ";
		cin >> getcode;
		while (getcode != passwd) {
			cout << "password not correct. Try again?" << endl;
			cout << "Y/N";
			char flag;
			flag = getchar();
			if (flag == 'Y' || flag == 'y') {
				cout << "password: ";
				cin >> getcode;
				continue;
			}
			else if (flag == 'N' || flag == 'n') {
				return true;
			}
			else {
				cout << "Invalid input! Aborting..." << endl;
				return true;
			}
		}
		accessLevel = read_write;
	}
	return true;
}

bool Database::close()
{
	//   >> check whether spare space data should be reposition <<
	fidx.close();
	fdat.close();
}

const string& Database::fetch(const string& key)
{
	NodeData res = idx.search(key);
	const streamoff& data_offset = res.get_data_offset();
	fdat.seekg(data_offset, ios::beg);
	const int& len = res.get_data_len();
	char *tmp = new char[len + 1];
	fdat.read(tmp, sizeof(char)*len);
	tmp[len] = '\0';
	string ret(tmp);
	return ret;
}

bool Database::store(const string& key, const string& t)
{
	if (accessLevel == readonly) {
		cout << "Access denied." << "\n"
			<< "You must do modification as Administrator." << endl;
		return false;
	}
	streamoff available = -1;
	for (map<streamoff, int>::iterator it = spareSpace.begin();
		it != spareSpace.end(); ++it) {
		if (it->second >= t.size()) {
			available = it->first;
			break;
		}
	}
	//   > if there is spare space large enough to store
	if (available != -1) {
		fidx.seekp(available, ios::beg);
		NodeData old;
		old.read_binary(fidx);
		fdat.seekp(old.get_data_offset(), ios::beg);
		fdat.write(t.c_str(), sizeof(char) *t.size());
		old.set_key(key);
		old.set_data_len(t.size());
		fidx.seekp(available, ios::beg);
		old.write_binary(fidx);

		//   > flush the buffer to disk
		//   > wait to be improved with buffer management
		//  >> update spare space data <<
		fdat.close();
		fidx.close();
		fdat.open(pathname + ".idx", ios::binary | ios::in | ios::out);
		fidx.open(pathname + ".dat", ios::binary | ios::in | ios::out);
		idx.insert(old);
	}
	//   > if there is no place large enough to store
	else {
		fdat.seekp(0, ios::end);
		streamoff newrec_off = fdat.tellp();
		fdat.write(t.c_str(), sizeof(char) * t.size());
		NodeData newNode(0, 35, key, newrec_off, t.size(), ':');
		streamoff tmp;
		fidx.seekg(0, ios::beg);
		fidx.read((char*)&tmp, sizeof(streamoff));
		//   > if empty database
		if (tmp == 0) {
			//   > tmp == 0, empty database
			//  >> trunc first <<
			fdat.seekp(sizeof(int), ios::beg);
			fdat.seekp(sizeof(char) * 16, ios::cur);
			newNode.set_data_offset(fdat.tellp());
			fidx.seekp(sizeof(streamoff) * 2, ios::beg);
			newNode.write_binary(fidx);
			fdat.write(t.c_str(), sizeof(char)*t.size());
			idx.insert(newNode);
		}
		//   > if not empty database
		else {
			NodeData& last_rec = last_idx(fidx);
		}
	}
}