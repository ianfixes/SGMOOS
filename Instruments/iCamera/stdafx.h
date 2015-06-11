#pragma once

#define WINVER		0x040A   // <--- !!!!

#include <iostream>
using namespace std;

#define USE_WTL
#include <atlbase.h>
#include <atlapp.h>
extern CAppModule _Module;

#include "Bcam.h"
using namespace Bcam;

