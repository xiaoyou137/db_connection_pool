#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

#define LOG(str) \
    cout << __FILE__ << ":" << __LINE__ << " " \
    << __TIME__ << ": " << str << endl;

#endif
