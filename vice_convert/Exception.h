#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>
#include <string>

using std::exception;
using std::string;

/* Copied from http://stackoverflow.com/a/134612 */
class MyException : public exception
{
public:
	string s;
	MyException(string ss) : s(ss) {}
	~MyException() throw () {} // Updated
	const char* what() const throw() { return s.c_str(); }
};

#endif