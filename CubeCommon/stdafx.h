#pragma once

#ifdef _WIN32
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#ifdef linux
#endif

#ifdef _UNIX
#endif


#include <vector>
#include <map>
#include <string>
#include <sstream>

using std::vector;
using std::map;
using std::string;
using std::stringstream;
using std::ptrdiff_t;

#include "types.h"
#include "utilities.h"
