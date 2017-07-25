#pragma once
#ifndef _CONTACT_H
#define _CONTACT_H

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

class Person
{
private:
	std::string id, name, country, phone;
	enum Sex { UNKNOWN = -1, female, male };
	Sex sex;

public:
	Person();
	Person(const std::string& _id, const std::string& _name = "UNKNOWN",
		const std::string& _country = "UNKNOWN", const std::string& _phone = "UNKNOWN", const Sex& _sex = UNKNOWN);
	Person(Person&&);
	Person& operator=(Person&&);

	Person& set_id(const std::string& _id);
	Person& set_name(const std::string& _name);
	Person& set_country(const std::string& _country);
	Person& set_phone(const std::string& _phone);
	Person& set_sex(const Sex& _sex);

	const std::string& get_id() const;
	const std::string& get_name() const;
	const std::string& get_country() const;
	const std::string& get_phone() const;
	const std::string& get_sex() const;
	const std::string info() const;

	friend std::ostream& operator<<(std::ostream& ios, const Person& self);
	//friend std::istream& operator>>(std::istream& ios, Person& self);

	~Person();
};

#endif