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
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <vector>

#include "fs_helper.h"
#include "zlib_helper.h"
#include "globals.h"

const char  arcStart		= 0xC;
const char  arcEntryLength	= 0x50;

using namespace std;

struct arcHeader {
  char  magic[4] = "ARC";
  char  version;
  unsigned short fileCount;
};

struct arcFileEntry {
  char		fileName[64];
  uint32_t	fileType;
  uint32_t	dataLength;
  uint32_t	uncompressedLength;
  uint32_t	dataStart;
};

int repackARC(string arcFolder);
int unpackARC(string arcFile);
int unpackFile(int fileIndex, istream &arcIn, ostream &manifestOut);
int getFileEntry(int fileIndex, arcFileEntry &tArcFileEntry, istream &arcIn, ostream &manifestOut);
