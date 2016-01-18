#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#ifdef __linux
#endif

#if defined(__APPLE__) && defined(__MACH__)
#endif

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(T) \
	T(const T&) = delete;    \
	T& operator=(const T&) = delete;
#endif

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>

using std::vector;
using std::map;
using std::string;
using std::stringstream;
using std::ptrdiff_t;

#include "CubeTypes.h"
#include "utilities.h"
