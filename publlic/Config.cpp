// Config.cpp  

#include "Config.h"  

using namespace std;


Config::Config(muduo::string filename, muduo::string delimiter,
	muduo::string comment)
	: delimiter_(delimiter), comment_(comment)
{
	// Construct a Config, getting keys and values from given file  

	std::ifstream in(filename.c_str());

	if (!in) throw File_not_found(filename);

	in >> (*this);
}


Config::Config()
	: delimiter_(muduo::string(1, '=')), comment_(muduo::string(1, '#'))
{
	// Construct a Config without a file; empty  
}



bool Config::KeyExists(const muduo::string& key) const
{
	// Indicate whether key is found  
	auto p = contents_.find(key);
	return (p != contents_.end());
}


/* static */
void Config::Trim(muduo::string& inout_s)
{
	// Remove leading and trailing whitespace  
	static const char whitespace[] = " \n\t\v\r\f";
	inout_s.erase(0, inout_s.find_first_not_of(whitespace));
	inout_s.erase(inout_s.find_last_not_of(whitespace) + 1U);
}


std::ostream& operator<<(std::ostream& os, const Config& cf)
{
	// Save a Config to os
	/*for (auto p = cf.contents_.begin(); p != cf.contents_.end(); ++p)
	{
		os << p->first << " " << cf.delimiter_ << " ";
		os << p->second << std::endl;
	}*/
	return os;
}

void Config::Remove(const muduo::string& key)
{
	// Remove key and its value  
	contents_.erase(contents_.find(key));
	return;
}

std::istream& operator>>(std::istream& is, Config& cf)
{
	// Load a Config from is  
	// Read in keys and values, keeping internal whitespace  
	typedef muduo::string::size_type pos;
	const muduo::string& delim = cf.delimiter_;  // separator  
	const muduo::string& comm = cf.comment_;    // comment  
	const pos skip = delim.length();        // length of separator  

	muduo::string nextline = "";  // might need to read ahead to see where value ends  

	while (is || nextline.length() > 0)
	{
		// Read an entire line at a time  
		muduo::string line;
		if (nextline.length() > 0)
		{
			line = nextline;  // we read ahead; use it now  
			nextline = "";
		}
		else
		{
			std::getline(is, line);
		}

		// Ignore comments  
		line = line.substr(0, line.find(comm));

		// Parse the line if it contains a delimiter  
		pos delimPos = line.find(delim);
		if (delimPos < muduo::string::npos)
		{
			// Extract the key  
			muduo::string key = line.substr(0, delimPos);
			line.replace(0, delimPos + skip, "");

			// See if value continues on the next line  
			// Stop at blank line, next line with a key, end of stream,  
			// or end of file sentry  
			bool terminate = false;
			while (!terminate && is)
			{
				std::getline(is, nextline);
				terminate = true;

				muduo::string nlcopy = nextline;
				Config::Trim(nlcopy);
				if (nlcopy == "") continue;

				nextline = nextline.substr(0, nextline.find(comm));
				if (nextline.find(delim) != muduo::string::npos)
					continue;

				nlcopy = nextline;
				Config::Trim(nlcopy);
				if (nlcopy != "") line += "\n";
				line += nextline;
				terminate = false;
			}

			// Store key and value  
			Config::Trim(key);
			Config::Trim(line);
			cf.contents_[key] = line;  // overwrites if key is repeated  
		}
	}

	return is;
}
bool Config::FileExist(muduo::string filename)
{
	bool exist = false;
	std::ifstream in(filename.c_str());
	if (in)
		exist = true;
	return exist;
}

void Config::ReadFile(muduo::string filename, muduo::string delimiter,
	muduo::string comment)
{
	delimiter_ = delimiter;
	comment_ = comment;
	std::ifstream in(filename.c_str());

	if (!in) throw File_not_found(filename);

	in >> (*this);
}