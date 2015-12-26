// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#ifdef _WIN32
#include "targetver.h"
#endif

#ifdef linux
#endif

#ifdef _UNIX
#endif

#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <ctime>
#include <cstring>
#include <string>
#include <iostream>
#include <map>
using namespace std;

#include "Config.h"

#ifdef USE_GL

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#endif

#include "types.h"
#include "utilities.h"
