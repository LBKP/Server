#pragma once  

#include <muduo/base/Types.h>  
#include <map>  
#include <iostream>  
#include <fstream>  
#include <sstream>  
#include <algorithm>


/*
* \brief Generic configuration Class
*
*/
class Config {
	// Data  
protected:
	muduo::string delimiter_;  //separator between key and value  
	muduo::string comment_;    //separator between value and comments  
	std::map<muduo::string, muduo::string> contents_;  //extracted keys and values  

	// Methods  
public:

	Config(muduo::string filename, muduo::string delimiter = "=", muduo::string comment = "#");
	Config();
	//Search for key and read value or optional default value, call as read<T>  
	template<class T> T Read(const muduo::string& in_key) const;
	template<class T> T Read(const muduo::string& in_key, const T& in_value) const;
	template<class T> bool ReadInto(T& out_var, const muduo::string& in_key) const;
	template<class T>
	bool ReadInto(T& out_var, const muduo::string& in_key, const T& in_value) const;
	bool FileExist(muduo::string filename);
	void ReadFile(muduo::string filename, muduo::string delimiter = "=", muduo::string comment = "#");

	// Check whether key exists in configuration  
	bool KeyExists(const muduo::string& in_key) const;

	// Modify keys and values  
	template<class T> void Add(const muduo::string& in_key, const T& in_value);
	void Remove(const muduo::string& in_key);

	// Check or change configuration syntax  
	muduo::string GetDelimiter() const { return delimiter_; }
	muduo::string GetComment() const { return comment_; }
	muduo::string SetDelimiter(const muduo::string& in_s)
	{
		muduo::string old = delimiter_;  delimiter_ = in_s;  return old;
	}
	muduo::string SetComment(const muduo::string& in_s)
	{
		muduo::string old = comment_;  comment_ = in_s;  return old;
	}

	// Write or read configuration  
	friend std::ostream& operator<<(std::ostream& os, const Config& cf);
	friend std::istream& operator>>(std::istream& is, Config& cf);

protected:
	template<class T> static muduo::string T_as_string(const T& t);
	template<class T> static T string_as_T(const muduo::string& s);
	static void Trim(muduo::string& inout_s);


	// Exception types  
public:
	struct File_not_found {
		muduo::string filename;
		File_not_found(const muduo::string& filename_ = muduo::string())
			: filename(filename_) {}
	};
	struct Key_not_found {  // thrown only by T read(key) variant of read()  
		muduo::string key;
		Key_not_found(const muduo::string& key_ = muduo::string())
			: key(key_) {}
	};
};


/* static */
template<class T>
muduo::string Config::T_as_string(const T& t)
{
	// Convert from a T to a string  
	// Type T must support << operator  
	std::ostringstream ost;
	ost << t;
	return muduo::string(ost.str().c_str());
}


/* static */
template<class T>
T Config::string_as_T(const muduo::string& s)
{
	// Convert from a string to a T  
	// Type T must support >> operator  
	T t;
	muduo::string a;
	std::istringstream ist(s);
	ist >> t;
	return t;
}


/* static */
template<>
inline muduo::string Config::string_as_T<muduo::string>(const muduo::string& s)
{
	// Convert from a string to a string  
	// In other words, do nothing  
	return s;
}


/* static */
template<>
inline bool Config::string_as_T<bool>(const muduo::string& s)
{
	// Convert from a string to a bool  
	// Interpret "false", "F", "no", "n", "0" as false  
	// Interpret "true", "T", "yes", "y", "1", "-1", or anything else as true  
	bool b = true;
	muduo::string sup = s;
	std::transform(sup.begin(), sup.end(), sup.begin(), ::toupper);
	if (sup == muduo::string("FALSE") || sup == muduo::string("F") ||
		sup == muduo::string("NO") || sup == muduo::string("N") ||
		sup == muduo::string("0") || sup == muduo::string("NONE"))
		b = false;
	return b;
}


template<class T>
T Config::Read(const muduo::string& key) const
{
	// Read the value corresponding to key  
	auto p = contents_.find(key);
	if (p == contents_.end()) throw Key_not_found(key);
	return string_as_T<T>(p->second);
}


template<class T>
T Config::Read(const muduo::string& key, const T& value) const
{
	// Return the value corresponding to key or given default value  
	// if key is not found  
	auto p = contents_.find(key);
	if (p == contents_.end()) return value;
	return string_as_T<T>(p->second);
}


template<class T>
bool Config::ReadInto(T& var, const muduo::string& key) const
{
	// Get the value corresponding to key and store in var  
	// Return true if key is found  
	// Otherwise leave var untouched  
	auto p = contents_.find(key);
	bool found = (p != contents_.end());
	if (found) var = string_as_T<T>(p->second);
	return found;
}


template<class T>
bool Config::ReadInto(T& var, const muduo::string& key, const T& value) const
{
	// Get the value corresponding to key and store in var  
	// Return true if key is found  
	// Otherwise set var to given default  
	auto p = contents_.find(key);
	bool found = (p != contents_.end());
	if (found)
		var = string_as_T<T>(p->second);
	else
		var = value;
	return found;
}


template<class T>
void Config::Add(const muduo::string& in_key, const T& value)
{
	// Add a key with given value  
	muduo::string v = T_as_string(value);
	muduo::string key = in_key;
	//trim(key);
	//trim(v);
	contents_[key] = v;
	return;
}