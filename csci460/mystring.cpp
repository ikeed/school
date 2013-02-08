#include <sstream>
#include <iostream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include "mystring.h"

string &ltrim(string &s) {
        s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
        return s;
}

// trim from end
string &rtrim(string &s) {
        s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
        return s;
}

// trim from both ends
string &trim(string &s) {
        return ltrim(rtrim(s));
}

bool parse(string buf, string & type, string & contents)
{
	string line = buf;

	if (line == "") {
		type = "";
		contents = "";
		return false;
	} else {
		istringstream iss(line);
		iss >> type;
		getline(iss, contents, '\0');
		contents = trim(contents);
	}
	return true;
}

