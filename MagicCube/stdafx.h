// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#ifdef _WIN32
#include "targetver.h"
#endif

#ifdef linux
#endif

#ifdef _UNIX
#endif

#include <stdio.h>
#include <cmath>
#include <ctime>

#include "types.h"

#include <string>
#include <iostream>

using namespace std;

#include <GL\glew.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "Config.h"

#define min(a,b) (((a)<(b))?(a):(b))

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
