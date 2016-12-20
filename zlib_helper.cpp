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

#include "zlib_helper.h"

bool decompressData(string &compressedString, string &uncompressedString, unsigned uncompressedLength) {
	unsigned compressedLength = (unsigned)compressedString.size();
	
	if (compressedLength < 1) {
		uncompressedString = compressedString;
		printf("Decompression Error: No input\n\n");
		return false;
	}

	uncompressedString.clear();

	if (uncompressedLength == 0) uncompressedLength = compressedString.size();
	char* uncompressBuffer = (char*) calloc(sizeof(char), uncompressedLength);

	z_stream stream;
	stream.next_in = (Bytef *) compressedString.c_str();
	stream.avail_in = compressedString.size();
	stream.total_out = 0;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;

	if (inflateInit2(&stream, MAX_WBITS) != Z_OK) {
		free (uncompressBuffer);
		printf("Decompression Error: Couldn't init stream\n\n");
		return false;
	}

	bool done = false;
	while (!done) {
		if (stream.total_out >= uncompressedLength) {
			char* uncompressBufferNew = (char*) calloc(sizeof(char), (uncompressedLength * 1.5));
			memcpy(uncompressBufferNew, uncompressBuffer, uncompressedLength);
			uncompressedLength *= 1.5;
			free(uncompressBuffer);
			uncompressBuffer = uncompressBufferNew;
		}

		stream.next_out = (Bytef *)(uncompressBuffer + stream.total_out);
		stream.avail_out = uncompressedLength - stream.total_out;

		int error = inflate(&stream, Z_SYNC_FLUSH);
		if (error == Z_STREAM_END) done = true;
		else if (error != Z_OK) {
			printf("Decompression Error: Inflation failed\n\n");
			break;
		}
	}

	if (inflateEnd(&stream) != Z_OK) {
		free(uncompressBuffer);
		printf("Decompression Error: Couldn't end inflation\n\n");
		return false;
	}

	for (size_t i = 0; i < stream.total_out; ++i) uncompressedString += uncompressBuffer[i];

	free(uncompressBuffer);
	return true;
}

bool compressData(string &uncompressedString, string &compressedString, const unsigned char &compressionLevel) {
    z_stream stream;
    memset(&stream, 0, sizeof(stream));

    if (deflateInit2(&stream, compressionLevel, Z_DEFLATED, MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
    	printf("Compression Error: Couldn't init stream\n\n");
		return false;
	}

    stream.next_in = (Bytef*)uncompressedString.data();
    stream.avail_in = uncompressedString.size();

    int ret;
    char outbuffer[32768];
    compressedString.clear();

    do {
        stream.next_out = reinterpret_cast<Bytef*>(outbuffer);
        stream.avail_out = sizeof(outbuffer);

        ret = deflate(&stream, Z_FINISH);

        if (compressedString.size() < stream.total_out) {
            compressedString.append(outbuffer, stream.total_out - compressedString.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&stream);

    if (ret != Z_STREAM_END) {
        printf("Compression Error: Deflation failed\n\n");
		return false;
	}

    return true;
}