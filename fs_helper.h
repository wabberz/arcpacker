/*  This file is part of arcpacker.

    arcpacker is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    arcpacker is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with arcpacker.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include "globals.h"

using namespace std;

#define isFolder	-1
#define isFile		-2
#define isDevice	-3
#define isEmpty		0
typedef struct stat Stat;

#if defined(WIN32) || defined(WIN64)
int do_mkdir(const char *path);
int mkpath(const char *path);
#else
int do_mkdir(const char *path, mode_t mode);
int mkpath(const char *path, mode_t mode);
#endif
int getStat(const string& node);
void trim(string &s);
