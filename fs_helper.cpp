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

#include "fs_helper.h"

// do_mkdir() and mkpath(): http://stackoverflow.com/a/675193
// do_mkdir() and mkpath() are (c) Jonathan Leffler
#if defined(WIN32) || defined(WIN64)
char fsDirSlash = '\\';

int do_mkdir(const char *path) {
#else
char fsDirSlash = '/';

int do_mkdir(const char *path, mode_t mode) {
#endif
    Stat st;
    int status = 0;

    if (stat(path, &st) != 0) {
#if (defined(WIN32) || defined(WIN64))
    	if (mkdir(path) != 0 && errno != EEXIST) status = -1;
#elif defined __linux__
        if (mkdir(path, mode) != 0 && errno != EEXIST) status = -1;
#endif
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}

#if defined(WIN32) || defined(WIN64)
int mkpath(const char *path) {
#else
int mkpath(const char *path, mode_t mode) {
#endif
    char *pp;
    char *sp;
    int status;
#if defined(WIN32) || defined(WIN64)
    char *copypath = _strdup(path);
#else
    char *copypath = strdup(path);
#endif

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, fsDirSlash)) != 0) {
        if (sp != pp) {
            *sp = '\0';
            #if defined(WIN32) || defined(WIN64)
            status = do_mkdir(copypath);
            #else
            status = do_mkdir(copypath, mode);
            #endif
            *sp = fsDirSlash;
        }
        pp = sp + 1;
    }
    if (status == 0)
        #if defined(WIN32) || defined(WIN64)
        status = do_mkdir(path);
        #else
        status = do_mkdir(path, mode);
        #endif
    free(copypath);
    return (status);
}

int getStat(const string& node) {
	struct stat s;
	if (stat(node.c_str(),&s) == 0) {
		if (s.st_mode & S_IFDIR) {
			return isFolder;
		} else if (s.st_mode & S_IFREG) {
			return isFile;
		} else {
			return isDevice;
		}
	} else {
		return isEmpty;
	}
}

static inline void ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(),
            not1(ptr_fun<int, int>(isspace))));
}

static inline void rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(),
            not1(ptr_fun<int, int>(isspace))).base(), s.end());
}

void trim(string &s) {
    ltrim(s);
    rtrim(s);
}
