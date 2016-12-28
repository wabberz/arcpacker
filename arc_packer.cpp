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

#include "arc_packer.h"
#include "fs_helper.h"

arcHeader       fArcHeader;
arcFileEntry    fArcFileEntry;
string          currentArcFile;
string          currentArcFolder;
unsigned int    arcFileOffset;

#if defined(WIN32) || defined(WIN64)
string sysDirSlash = "\\";
#else
string sysDirSlash = "/";
#endif

int getFileEntry(int fileIndex, arcFileEntry &tArcFileEntry, istream &arcIn) {
    try {
        arcIn.seekg(arcStart + (fileIndex * arcEntryLength));
        arcIn.read((char*)&tArcFileEntry, sizeof(tArcFileEntry));
    } catch (...) {
        return -1;
    }

    
#if (defined(WIN32) || defined(WIN64))
    //strncpy_s(tArcFileEntry.fileName, tFileName.c_str(), 64);
#else
    string tFileName = tArcFileEntry.fileName;
    replace(tFileName.begin(), tFileName.end(), '\\', '/');
    strncpy(tArcFileEntry.fileName, tFileName.c_str(), 64);
#endif
    tArcFileEntry.fileName[sizeof(tArcFileEntry.fileName) - 1] = 0;

    tArcFileEntry.uncompressedLength -= 0x40000000;

    return 0;
}

int getCompressedFile(istream &arcIn, arcFileEntry &tArcFileEntry, string &compressedData, unsigned char &compressionLevel) {
    try {
        compressedData.clear();
        compressedData.resize(tArcFileEntry.dataLength);
        arcIn.seekg(tArcFileEntry.dataStart);
        arcIn.read(&compressedData[0], tArcFileEntry.dataLength);
    } catch (...) {
        return -1;
    }

    switch((*((unsigned short*)compressedData.substr(0,2).c_str()))) {
        case 0x0178:
            compressionLevel = 0;
            break;
        case 0x9C78:
            compressionLevel = 6;
            break;
        case 0xDA78:
            compressionLevel = 9;
            break;
        default:
            compressionLevel = 6;
            break;
    }
    if (flagVerbose >= 3) printf("(Level: %i)...", compressionLevel);

    return 0;
}

int unpackFile(int fileIndex, istream &arcIn, ostream &manifestOut) {
    string compressedData, uncompressedData;

    if (getFileEntry(fileIndex, fArcFileEntry, arcIn) != 0) {
        printf("Failed to get header data from ARC!\n\n");
        return -7;
    }

    if (flagVerbose >= 3) printf("FileName: %s;\nfileType: %08x;\ndataLength: %i;\nuncompressedLength: %i;\ndataStart: %08X;\n",fArcFileEntry.fileName,fArcFileEntry.fileType,fArcFileEntry.dataLength,(fArcFileEntry.uncompressedLength),fArcFileEntry.dataStart);

    string tFileName = fArcFileEntry.fileName;
    size_t rPos = tFileName.find_last_of(sysDirSlash.c_str());
    string path = tFileName.substr(0, rPos);
    rPos = currentArcFile.find_last_of(".");
    string folder = currentArcFolder + sysDirSlash + path;
    
    if (flagVerbose >= 2) printf("%s%s", folder.c_str(), sysDirSlash.c_str());

    unsigned char compressionLevel;
    if (getCompressedFile(arcIn, fArcFileEntry, compressedData, compressionLevel) != 0) {
        printf("Failed to get compressed file from ARC!\n\n");
        return -7;
    }

    bool decRet = decompressData(compressedData, uncompressedData, fArcFileEntry.uncompressedLength);
    if (!decRet) {
        printf("Decompression failed!\n\n");
        return -3;
    }

    string extention = uncompressedData.substr(0, 3);
    transform(extention.begin(), extention.end(), extention.begin(), ::tolower);
    char tFile[200];
    rPos = tFileName.find_last_of(sysDirSlash.c_str());
#if defined(WIN32) || defined(WIN64)
    sprintf_s(tFile, "%s.%08x.%s", tFileName.substr(rPos+1).c_str(), fArcFileEntry.fileType, extention.c_str());
#else
    sprintf(tFile, "%s.%08x.%s", tFileName.substr(rPos+1).c_str(), fArcFileEntry.fileType, extention.c_str());
#endif
    string file = tFile;
    if (flagVerbose >= 2) printf("%s\n", file.c_str());

    if (flagVerbose >= 3) printf("Folder: %s;\nFile: %s\n\n", folder.c_str(), file.c_str());

    #if defined(WIN32) || defined(WIN64)
    int rc = mkpath(folder.c_str());
    #else
    int rc = mkpath(folder.c_str(), 0777);
    #endif
    if (rc != 0) {
        printf("Couldn't create folder!\n\n");
        return -2;
    }

    ofstream binOut((folder + sysDirSlash.c_str() + file).c_str(), ios_base::out|ios_base::binary);
    binOut << uncompressedData;
    binOut.close();

    string manifestPath = path + "\\" + file;
    replace(manifestPath.begin(), manifestPath.end(), '/', '\\'); // Make sure that the Manifest always uses backslashes
    manifestOut << (manifestPath + "," + to_string(compressionLevel) + "\n");

    return 0;
}

int unpackARC(string arcFile) {
    currentArcFile = arcFile;

	ifstream arcIn;
	arcIn.open(currentArcFile.c_str(), ios::in|ios::binary);
	if (!arcIn.good()) {
		printf("Couldn't open archive!\n\n");
		return -1;
	}
	arcIn.seekg(0);
  	arcIn.read((char*)&fArcHeader, sizeof(fArcHeader));

  	if (flagVerbose >= 1) printf("Magic: %s; Version: %i; FileCount: %i\n", fArcHeader.magic, fArcHeader.version, fArcHeader.fileCount);

  	size_t rPos = currentArcFile.find_last_of(".");

    if (flagOutputFile.empty()) {
        currentArcFolder = currentArcFile.substr(0, rPos);
    } else {
        currentArcFolder = flagOutputFile;
    }

    if (getStat(currentArcFolder) != isEmpty) {
        if (flagOverwrite) {
            printf("\nThe folder '%s' already exists - and the overwrite flag isn't supported for unpacking, yet :(\n\n", currentArcFolder.c_str());
            return -8;
        } else {
            printf("\nThe folder '%s' already exists\n\n", currentArcFolder.c_str());
            return -8;
            /*
            bool waitForAnswer = true;
            string overwriteAnswer;
            uintmax_t deletedFiles = 0;
            do {
                printf("\nThe folder '%s' already exists - replace? (y/n): ");
                getline (cin,overwriteAnswer);

                if (overwriteAnswer == "y") {
                    waitForAnswer = false;
                } else if (overwriteAnswer == "n") {
                    waitForAnswer = false;
                }
            } while (waitForAnswer);
            */
        }
    }

    #if defined(WIN32) || defined(WIN64)
    int rc = mkpath(currentArcFolder.c_str());
    #else
    int rc = mkpath(currentArcFolder.c_str(), 0777);
    #endif
    if (rc != 0) {
    	printf("Couldn't create folder!\n\n");
  		return -2;
    }
    if (flagVerbose >= 1) printf("Writing manifest file to %s\n\n", (currentArcFolder + sysDirSlash + "manifest.txt").c_str());
    ofstream manifestOut((currentArcFolder + sysDirSlash + "manifest.txt").c_str(), ios_base::out|ios_base::binary);

    // make an empty file to mark Dual Destinies folders
    if (fArcHeader.version == 16) {
        if (flagVerbose >= 1) printf("Marking folder as DualDestinies via %s\n\n", (currentArcFolder + sysDirSlash + ".dualdestinies").c_str());
        ofstream ddOut((currentArcFolder + sysDirSlash + ".dualdestinies").c_str(), ios_base::out|ios_base::binary);
        ddOut << "This file marks the folder as a Dual Destinies ARC, so it will be repacked into the correct format.\n";
        ddOut.close();
    }

  	for (int fileIndex = 0; fileIndex < fArcHeader.fileCount; fileIndex++) {
  		unpackFile(fileIndex, arcIn, manifestOut);
    }

    manifestOut.close();

    return 0;
}

int repackARC(string arcFolder) {
    currentArcFolder    = arcFolder;
    currentArcFile      = arcFolder + "_repacked.arc";

    if (flagOutputFile.empty()) {
        currentArcFile  = arcFolder + "_repacked.arc";
    } else {
        currentArcFile  = flagOutputFile;
    }

    if (getStat(currentArcFile) != isEmpty) {
        if (flagOverwrite) {
            if (remove(currentArcFile.c_str()) != 0) {
                printf("Couldn't delete '%s'...\n", currentArcFile.c_str());
                return -9;
            }
            if (flagVerbose >= 1) printf("Deleted existing ARC '%s' before repacking it...\n", currentArcFile.c_str());
        } else {
            bool waitForAnswer = true;
            string overwriteAnswer;
            do {
                printf("The file '%s' already exists - replace? (y/n): ", currentArcFile.c_str());
                getline (cin,overwriteAnswer);

                if (overwriteAnswer == "y") {
                    if (remove(currentArcFile.c_str()) != 0) {
                        printf("Couldn't delete '%s'...\n", currentArcFile.c_str());
                        return -9;
                    }
                    if (flagVerbose >= 1) printf("Deleted existing ARC '%s' before repacking it...\n", currentArcFile.c_str());
                    waitForAnswer = false;
                } else if (overwriteAnswer == "n") {
                    printf("OK, then - quitting...\n\n");
                    return -8;
                    waitForAnswer = false;
                }
            } while (waitForAnswer);
        }
    }

    vector<string> manifestFiles;
    string tLine;
    ifstream manifestIn((arcFolder + sysDirSlash + "manifest.txt").c_str(), ios::in|ios::binary);
    if (!manifestIn.good()) {
        printf("Couldn't open manifest!\n\n");
        return -1;
    }

    while (getline(manifestIn, tLine)) {
        replace(tLine.begin(), tLine.end(), '\\', '/'); // In case the Manifest used forward-slashes
        trim(tLine);
        if (!tLine.empty()) manifestFiles.push_back(tLine);
    }

    bool isDDArchive = false;
    if (getStat(currentArcFolder + sysDirSlash + ".dualdestinies") != isEmpty) {
        if (flagVerbose >= 1) printf("Found the file to mark a Dual Destinies ARC, setting ARC type...\n");
        isDDArchive = true;
    }
    
    //fArcHeader.magic = {"A","R","C","\0"};
    if (flagDualDestinies || isDDArchive) {
        fArcHeader.version = 16;
        arcFileOffset = 0x8000;
    } else {
        fArcHeader.version = 17;
        arcFileOffset = 0xC + (0x50 *  manifestFiles.size());
        if (arcFileOffset % 32 != 0) arcFileOffset = ((arcFileOffset / 32) + 1) * 32;
    }
    fArcHeader.fileCount = manifestFiles.size();

    ofstream arcOut;
    arcOut.open(currentArcFile.c_str(), ios::out|ios::binary);
    if (!arcOut.good()) {
        printf("Couldn't open archive to write!\n\n");
        return -1;
    }
    arcOut.seekp(0);
    arcOut.write((char*)&fArcHeader, sizeof(fArcHeader));
    

    for (size_t fileIndex = 0; fileIndex < manifestFiles.size(); fileIndex++) {
        string aLine = manifestFiles.at(fileIndex);
        size_t rPos = aLine.find_last_of(",");
        unsigned char aCompressionLevel = atoi(aLine.substr(rPos + 1).c_str());

        string aFileName = aLine.substr(0,rPos);
        rPos = aFileName.find_last_of(".");
        string aFileExtension = aFileName.substr(rPos + 1);
        size_t lPos = aFileName.find_last_of(".", rPos - 1);
        uint32_t aFileType =  std::stoul(aFileName.substr(lPos + 1, (rPos - (lPos + 1))), nullptr, 16);
        string tFileName = aFileName.substr(0, lPos);
        #if (defined(WIN32) || defined(WIN64))
        #else
        replace(tFileName.begin(), tFileName.end(), '/', '\\');
        #endif
    
        // load file
        ifstream binIn((arcFolder + sysDirSlash + aFileName).c_str(), ios::in|ios::binary);
        if (!binIn.good()) {
            printf("Couldn't open file: %s!\n\n", (arcFolder + sysDirSlash + aFileName).c_str());
            return -1;
        }
        binIn.seekg(0, binIn.end);
        size_t aFileLength = binIn.tellg();
        binIn.seekg(0, binIn.beg);
        vector<char> readBuffer(aFileLength);
        binIn.read(readBuffer.data(), aFileLength);
        binIn.close();
        string uncompressedData(readBuffer.begin(),readBuffer.end());
        readBuffer.clear();

        // Compress file
        //compressData(string &uncompressedString, string &compressedString, unsigned char &compressionLevel);
        string compressedData;
        unsigned char useCompressionLevel = aCompressionLevel;
        if (flagCompression > -1) useCompressionLevel = flagCompression;
        bool decRet = compressData(uncompressedData, compressedData, useCompressionLevel);
        if (!decRet) {
            printf("Decompression failed!\n\n");
            return -3;
        }
        size_t compressedFileLength = compressedData.size();
        if (flagVerbose >= 3) printf("aFileName: '%s';\ntFileName: '%s';\nType: '%08x';\nExtension: '%s';\nCompression: %i;\nFileLength: %i;\ncompressedFileLength: %i;\n\n",
            aFileName.c_str(), tFileName.c_str(), aFileType, aFileExtension.c_str(), aCompressionLevel, (int)aFileLength, (int)compressedFileLength);

        // Prepare FileEntry
#if (defined(WIN32) || defined(WIN64)) && defined(_MSC_VER)
        strncpy_s(fArcFileEntry.fileName,tFileName.c_str(),64);
#else
        strncpy(fArcFileEntry.fileName,tFileName.c_str(),64);
#endif
        fArcFileEntry.fileType = aFileType;
        fArcFileEntry.dataLength = compressedFileLength;
        fArcFileEntry.uncompressedLength = aFileLength + 0x40000000;
        fArcFileEntry.dataStart = arcFileOffset;

        arcOut.seekp(arcStart + (fileIndex * arcEntryLength));
        arcOut.write((char*)&fArcFileEntry, sizeof(fArcFileEntry));

        arcOut.seekp(arcFileOffset);
        arcOut << compressedData;

        arcFileOffset += compressedFileLength;
    }
    
    arcOut.close();

    return 0;
}
