/* Copyright 2015-2017 - Nico Curti, Alessandro Fabbri */

/***************************************************************************
This file is part of srsa.
srsa is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
srsa is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with srsa. If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _WIN32
#include <Windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <dirent.h>
#endif

#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <ctime>
#include <vector>
#include <string>
#include <cstdlib>

#include <unistd.h>

std::vector<std::string> split(std::string const& original, char separator)
{
	std::vector<std::string> results;
	std::string::const_iterator start = original.begin(), end = original.end(), next = std::find(start, end, separator);
	while (next != end)
	{
		results.push_back(std::string(start, next));
		start = next + 1;
		next = std::find(start, end, separator);
	}
	results.push_back(std::string(start, next));
	return results;
}

bool fileExists(std::string file)
{
	bool ret;
	std::ifstream file_to_check(file.c_str());
	if (file_to_check.is_open())
		ret = true;
	else
		ret = false;
	file_to_check.close();
	return ret;
}

bool dirExists(const std::string& dirName_in)
{
#ifdef _WIN32
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
#elif defined(__linux__) || defined(__APPLE__)
	DIR* dir = opendir(dirName_in.c_str());
	if (dir)
	{
		closedir(dir);
		return true;
	}
	else if (ENOENT == errno)
		return false;
	return false;
#endif
}

std::vector<std::string> GetFileNamesInDirectory(std::string directory)
{
	std::vector<std::string> files;
#ifdef _WIN32
	WIN32_FIND_DATA fileData;
	HANDLE hFind;
	if (!((hFind = FindFirstFile((directory + "*").c_str(), &fileData)) == INVALID_HANDLE_VALUE))
		while (FindNextFile(hFind, &fileData))
			if (fileExists(directory + fileData.cFileName))
				files.push_back(directory + fileData.cFileName);

	FindClose(hFind);
#elif __linux__
	DIR*    dir;
	dirent* pdir;
	dir = opendir((directory + "*").c_str());

	while (pdir = readdir(dir))
		if (fileExists(pdir->d_name))
			files.push_back(pdir->d_name);

#endif
	return files;
}

void Hide(std::string &filename)
{
#ifdef _WIN32
	DWORD attributes = GetFileAttributes(filename.c_str());
	SetFileAttributes(filename.c_str(), attributes + FILE_ATTRIBUTE_HIDDEN);
#elif __linux__
	filename = "/." + filename;
#endif
}

void Copy(std::string source, std::string dest)
{
	std::string command;
	std::vector<std::string> tkn_name;
#ifdef _WIN32
	tkn_name = split(source, '\\');
	command = "copy .\\" + tkn_name[tkn_name.size() - 1] + " " + dest + tkn_name[tkn_name.size() - 1];
	std::system(command.c_str());
	Hide(dest + tkn_name[tkn_name.size() - 1]);	
#elif __linux__
	tkn_name = split(source, '/');
	command = tkn_name[tkn_name.size() - 1];
	Hide(command);
	command = "cp " + source + " " + dest + command;
	std::cout << command << std::endl; std::cin.get();
	std::system(command.c_str());
#endif
}

std::vector<std::string> GetSubDir(const std::string& path)
{
	std::vector<std::string> directories;
#ifdef _WIN32
	WIN32_FIND_DATA findfiledata;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char fullpath[MAX_PATH];
	GetFullPathName(path.c_str(), MAX_PATH, fullpath, 0);
	std::string fp(fullpath);

	hFind = FindFirstFile((LPCSTR)(fp + "\\*").c_str(), &findfiledata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((findfiledata.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY && (findfiledata.cFileName[0] != '.'))
				directories.push_back(findfiledata.cFileName);
		} while (FindNextFile(hFind, &findfiledata) != 0);
	}
#elif __linux__
	DIR *dir = opendir(path.c_str()); //bug	
	dirent *entry = readdir(dir);
	while (entry != NULL)
	{
		if (entry->d_type == DT_DIR)
			directories.push_back(entry->d_name);
		entry = readdir(dir);
	}
	closedir(dir);
#endif
	return directories;
}

std::string FindPath(const std::string &root = "C:")
{
	srand((unsigned int)time(NULL));
	std::string path = root;
	std::vector<std::string> dirs = GetSubDir(root + "/");
	while (dirs.size() != 0)
	{
		path += "/" + dirs[rand() % dirs.size()];
		dirs = GetSubDir(path);
	}
	return path;
}

std::string LocalPath()
{
#ifdef _WIN32
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
#elif defined(__linux__) || defined(__APPLE__)
/*
	char szTmp[32];
	int bytes = MIN(readlink(szTmp, pBuf, len), len - 1);
	if (bytes >= 0)
		pBuf[bytes] = '\0';
*/
	char buf[1024];
	getcwd(buf, sizeof(buf));
	return std::string(buf);
#endif
}
