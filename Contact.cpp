#include "Contact.h"

using namespace std;
Person::Person() :id(""), name("UNKNOWN"), country("UNKNOWN"), phone("UNKNOWN"), sex(UNKNOWN)
{
	cout << "NEED ID" << flush;
	while (id.size() != 8) {
		cout << "Invalid ID, ID length must be 8." << flush;
		cin >> id;
	}
}

Person::Person(const string& _id, const string& _name, const string& _country, const string& _phone, const Sex& _sex)
{
	id = _id;
	name = _name;
	country = _country;
	phone = _phone;
	sex = _sex;
}

Person::Person(Person&& other)
{
	id = other.id;
	name = other.name;
	country = other.country;
	phone = other.phone;
	sex = other.sex;
	
	other.id = "UNKNOWN";
	other.name = "UNKNOWN";
	other.country = "UNKNOWN";
	other.phone = "UNKNOWN";
	other.sex = UNKNOWN;
}

Person& Person::operator=(Person&& other)
{
	if (&other == this) return *this;
	id = other.id;
	name = other.name;
	country = other.country;
	phone = other.phone;
	sex = other.sex;

	other.id = "UNKNOWN";
	other.name = "UNKNOWN";
	other.country = "UNKNOWN";
	other.phone = "UNKNOWN";
	other.sex = UNKNOWN;

	return *this;
}

Person& Person::set_id(const std::string& _id)
{
	id = _id;
	return *this;
}

Person& Person::set_name(const std::string& _name)
{
	name = _name;
	return *this;
}

Person& Person::set_country(const std::string& _country)
{
	country = _country;
	return *this;
}

Person& Person::set_phone(const std::string& _phone)
{
	phone = _phone;
	return *this;
}

Person& Person::set_sex(const Sex& _sex)
{
	sex = _sex;
	return *this;
}

const string& Person::get_id() const
{
	return id;
}

const string& Person::get_name() const
{
	return name;
}

const string& Person::get_country() const
{
	return country;
}

const string& Person::get_phone() const
{
	return phone;
}

const string& Person::get_sex() const
{
	if (sex == UNKNOWN) return "UNKNOWN";
	else if (sex == female) return "female";
	else return "male";
}

const string Person::info() const
{
	ostringstream oss;
	oss << "Name: " << name << "\t" << "ID: " << id << "\t"
		<< "Sex: " << sex << "\t" << "Country: " << country
		<< "\t" << "Phone: " << phone;
	return oss.str();
}

std::ostream& operator<<(std::ostream& ios, const Person& self)
{
	ios << self.info();
	return ios;
}

Person::~Person()
{}