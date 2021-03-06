#pragma once

#include "stdafx.h"
#include <fstream>

#ifndef _WIN32
#include <cmdline/cmdline.h>
#endif

#include "Config.h"
#include "CommandHandlers.h"
#include "TcpServer.h"
#include "CubeServer.h"
#include "Session.h"
#include "RoomInfo.h"

void eventEntry(TcpServer*);