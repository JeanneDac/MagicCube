#pragma once

#include "stdafx.h"
#include "Config.h"
#include "TcpServer.h"
#include "Session.h"
#include "ManualEvent.h"

bool endwith(const string&, const string&);
void printTime(FILE*);
void eventEntry(TcpServer*);