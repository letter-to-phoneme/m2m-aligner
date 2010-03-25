#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <ctime>

using namespace std;

template<typename T>
inline std::string join(const T& x, size_t pos, size_t npos, std::string delimeter = "")
 {
	 std::string sout;

	 size_t endPos;
	 endPos = pos + npos;

	 sout.clear();
	 for (typename T::const_iterator iter = x.begin() + pos; (iter != x.begin() + endPos); iter++)
	 {
		 sout = sout + *iter + delimeter;
	 }
	 sout.erase(sout.end() - delimeter.length(), sout.end());

	 return sout;
 }

inline void Tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = "\t")
{
	if (delimiters == "")
	{
		for (int i = 0; i < str.length(); i++)
		{
			tokens.push_back(str.substr(i,1));
		}
	}
	else
	{
		// Skip delimiters at beginning.
		string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		string::size_type pos     = str.find_first_of(delimiters, lastPos);

		while (string::npos != pos || string::npos != lastPos)
		{
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}
}


inline vector<string> splitBySpace(string line)
 {
         vector<string> lineList;
         string buff;
         stringstream ss(line);

         while (ss >> buff)
         {
                 lineList.push_back(buff);
         }

         return lineList;
}


inline string removeAllWhite (std::string &str)
{
    std::string temp;
    for (unsigned int i = 0; i < str.length(); i++)
        if (str[i] != ' ') temp += str[i];
    return (temp);
}

class BadConversion : public std::runtime_error {
	public:
	BadConversion(const std::string& s)
	 : std::runtime_error(s)
	 { }
};

template<typename T>
inline std::string stringify(const T& x)
{
	std::ostringstream o;
	if (!(o << x))
	 throw BadConversion(std::string("stringify(")
						 + typeid(x).name() + ")");
	return o.str();
}


template<typename T>
inline void convert(const std::string& s, T& x,
                 bool failIfLeftoverChars = true)
{
	istringstream i(s);
	
	char c;
	if (!(i >> x) || (failIfLeftoverChars && i.get(c)))
	 throw BadConversion(s);
}

template<typename T>
inline T convertTo(const std::string& s,
                bool failIfLeftoverChars = true)
{
	T x;
	convert(s, x, failIfLeftoverChars);
	return x;
}


