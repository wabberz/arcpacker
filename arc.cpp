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
// Original Python implementation by FrozenFish24, ported to C++ and extended by Uwabami.

#include <iostream>
#include <stdlib.h>
/*
#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#endif
*/

using namespace std;

#include "globals.h"
#include "arc_packer.h"

void showHelp() {
	printf ("MT-Engine ARC Un/Repacker Ver. 0.9.2 BETA, (c)2016 Uwabami\nOriginal Python code by FrozenFish24, ported to C++ and extended by Uwabami.\n\n"
			"Use:\n\tarc [flags] [file(s)/folder(s)]"
			"\n\nFlags:"
			"\n\t-c {0-9}\tforce zlib compression level\n\t\t\t\t 0 - no compression\n\t\t\t\t 9 - maximum compression\n\t\t\t\t-1 - will use manifest/default"
			"\n\t-dd\t\tpack Dual Destinies compatible ARC"
			"\n\t-f\t\toverwrite ARC files without asking"
			"\n\t-h\t\tshow this information and quit"
		/*	"\n\t-i\t\tignore manifest while packing\n\t\t\t\t (games will most likely not accept the result)" */
			"\n\t-o [file]\tdefine archive/folder name for output"
			"\n\t-q\t\tno output except errors (will override verbose flags)"
			"\n\t-v\t\tbe verbose"
			"\n\t-vv\t\tbe very verbose"
			"\n\t-vvv\t\tbe extremely verbose (show debug messages)"
			"\n\nUsually, the compression level of each file will be applied from the manifest.\nIf there's no manifest, it will default to 6 - but the games will not accept an\nARC with a different file order, so keep this in mind. If you define a level\noutside the range of 0-9, the flag will be ignored."
			"\n\nInput will be auto-detected. If you supply a folder, it will be compressed to an\nARC file. An ARC file will be unpacked to a folder. You can pack/unpack\nmultiple folders/files at the same time.\n\n");
}

int main(int argc, char **argv) {
	int startArg = 1;

	if (argc == 1) { showHelp(); return 0; }

	int i;
	for (i = 1; i < argc; i++) {
		if (string(argv[i]) == "-c") {
			if ((i+1) > argc) {
				printf("Missing argument: %s\n\n", argv[i]);
				return -5;
			}
			flagCompression = atoi(argv[++i]);
		} else if (string(argv[i]) == "-dd")	flagDualDestinies = true;
		else if (string(argv[i]) == "-f")		flagOverwrite = true;
		else if (string(argv[i]) == "-h" || argc == 1) {
			showHelp();
			return 0;
	/*	} else if (string(argv[i]) == "-i")		flagIgnoreManifest = true; */
		} else if (string(argv[i]) == "-o") {
			if ((i+1) > argc) {
				printf("Missing argument: %s\n\n", argv[i]);
				return -6;
			}
			flagOutputFile = argv[++i];
		} else if (string(argv[i]) == "-q")		flagQuiet = true;
		else if (string(argv[i]) == "-v")		flagVerbose = 1;
		else if (string(argv[i]) == "-vv")		flagVerbose = 2;
		else if (string(argv[i]) == "-vvv")		flagVerbose = 3;
		else {
			if(string(argv[i]).substr(0,1) != "-") break;
			printf("Unknown argument: %s\n\n", argv[i]);
			return -4;
		}
	}

	startArg = i;

	if (flagQuiet) flagVerbose = -1;
#if defined(WIN32) || defined(WIN64)
	if (flagVerbose >= 0) printf("MT-Engine ARC Un/Repacker Ver. 0.9.2 BETA (Windows)\nOriginal Python code by FrozenFish24, ported to C++ by Uwabami.\n\n");
#else
	if (flagVerbose >= 0) printf("MT-Engine ARC Un/Repacker Ver. 0.9.2 BETA (*NIX)\nOriginal Python code by FrozenFish24, ported to C++ by Uwabami.\n\n");
#endif

	if (startArg > argc) {
		printf("Nothing to do...\n\n");
		return 0;
	}

	if (flagCompression < -1 || flagCompression > 9) {
		if (flagVerbose >= 0) printf("!!! The compression level you set is invalid and will be ignored\n\n");
		flagCompression = -1;
	}

	if (flagVerbose >= 3)
		printf ("startArg: %i;\n"
				"flagCompression: %i\n"
				"flagDualDestinies: %i\n"
				"flagOverwrite: %i\n"
				"flagIgnoreManifest: %i\n"
				"flagOutputFile: %s\n"
				"flagQuiet: %i\n"
				"flagVerbose: %i\n\n",
				i, flagCompression, flagDualDestinies, flagOverwrite, flagIgnoreManifest, flagOutputFile.c_str(), flagQuiet, flagVerbose);

	int errReturn;
	for (i = startArg; i < argc; i++) {
		switch (getStat(argv[i])) {
			case isFolder:
				errReturn = repackARC(argv[i]);
				if (errReturn == 0) {
					if (flagVerbose >= 2) printf("\n...finished repacking %s!\n\n", argv[i]);
				} else {
					return errReturn;
				}
				break;
			case isFile:
				if (flagVerbose >= 0) printf("Unpacking: %s...\n\n", argv[i]);
				errReturn = unpackARC(argv[i]);
				if (errReturn == 0) {
					if (flagVerbose >= 2) printf("\n...finished unpacking %s!\n\n", argv[i]);
				} else {
					return errReturn;
				}
				break;
			case isDevice:
				if (flagVerbose >= 0) printf("Skipping device: %s\n\n", argv[i]);
				break;
			case isEmpty:
				if (flagVerbose >= 0) printf("Doesn't seem to exist: %s\n\n", argv[i]);
				break;
			default:
				if (flagVerbose >= 0) printf("Can't access: %s\n\n", argv[i]);
				break;
		}
	} if (flagVerbose >= 0) printf("Done.\n\n");

	return 0;
}
