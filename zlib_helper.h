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

#include <stdlib.h>
#include <string.h>
#include <sstream>
#include "zlib.h"
#include "zconf.h"

#include "globals.h"

using namespace std;

typedef unsigned char Byte;
typedef Byte Bytef;

bool decompressData(string &compressedString, string &uncompressedString, unsigned uncompressedLength = 0);
bool compressData(string &uncompressedString, string &compressedString, const unsigned char &compressionLevel = 6);