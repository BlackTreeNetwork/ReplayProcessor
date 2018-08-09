/*
 * ReplayProcessor v1.0
 * Copyright (c) 2018 Blacktree Network
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "ReplayProcessor.hpp"
#include <iterator>
#include <iostream>
#include <cstdlib>
#include <thread>

static int URL_OF_SECOND = 0;

static thread *DETECT_URL_THREAD = NULL;

static bool killed = false;

static bool FILE_FOUND = false;

// Returns an array of IP lists that are assumed to be servers.
// This function can also be called through the DLL library.
__declspec(dllexport)
string* InitializeAddressArray()
{
	// There are standard IP addresses used on the platform.
	// The video data must be read through those IP addresses.
	string *addresses = new string[MAX_SIZE_ADDRESS]{
		"114.31.51.68"  ,"114.31.50.122" ,"114.31.50.123","114.31.50.124" ,"180.182.60.194",
		"180.182.60.196","180.182.60.210","180.182.60.211","180.182.60.247","180.182.60.248"};
	int _a1[] = { 0x31, 0x31, 0x34, 0x2E, 0x33, 0x31, 0x2E, 0x35, 0x31, 0x2E, 0x36, 0x38 };
	return addresses; 
}

__declspec(dllexport)
void ScarchWowzaPlayerURL(__in const TCHAR *TCHAR_PROGRAM_DIR,
	                      __in const TCHAR *TCHAR_SERVER_URL,
	                      __in const TCHAR *TCHAR_OUTPUT_FILE,
	                      __in const TCHAR *BJ_DIRECTORY)
{
	// Get current directory, Perhaps the working directory will be set to that BJ folder.
	TCHAR *directory = new TCHAR[PATH_LENGTH];
	GetCurrentDir(directory, PATH_LENGTH);

	// Before the program starts working, it moves all the files in the folder.
	// If there is no capacity (0KB), it will be deleted.
	DetectCreatedFile(directory, BJ_DIRECTORY);

	// It locks the thread that detects the server URL.
	// Don't kill detect_thread! It causes a forced termination of the program.
	killed = false;

	// Create a new thread if it does not exist.
	if (DETECT_URL_THREAD == NULL)
	{
		DETECT_URL_THREAD = new thread(&DetectServerAddress, directory, TCHAR_SERVER_URL);
		DETECT_URL_THREAD->detach();
	}

	thread *scarch = new thread(&ScarchServerAddress, TCHAR_SERVER_URL, TCHAR_PROGRAM_DIR, TCHAR_OUTPUT_FILE, BJ_DIRECTORY);
	scarch->join();

	delete scarch;
	delete directory;
}

void ScarchServerAddress(const TCHAR *TCHAR_SERVER_URL, const TCHAR *TCHAR_PROGRAM_DIR, const TCHAR *TCHAR_OUTPUT_FILE, const TCHAR *BJ_DIRECTORY)
{
	TCHAR *directory = new TCHAR[PATH_LENGTH];
	GetCurrentDir(directory, PATH_LENGTH);

	// Parameter arguments
	String rtmp_optional(_T(" -r rtmp://"));
	String output_optional(_T(" -v -o "));
	for (URL_OF_SECOND = 0; URL_OF_SECOND < 60; URL_OF_SECOND++)
	{
		string str_second = to_string(URL_OF_SECOND);
		TCHAR *tchar_str_second = ConvertToWideCharset(str_second);
		String *server_url = new String(TCHAR_SERVER_URL);

		//Padding "second" to get the broadcast start time correctly.
		if (URL_OF_SECOND < 10) server_url->append(_T("0") + String(tchar_str_second));
		else server_url->append(String(tchar_str_second));

		system("cls");
		String output = replace_all(TCHAR_OUTPUT_FILE, _T("."), _T("")) + _T(".rpdownload");
		String *total = new String(TCHAR_PROGRAM_DIR + rtmp_optional + *server_url + output_optional + output);
		_wsystem(total->c_str());

		delete tchar_str_second;
		delete server_url;
		delete total;

		size_t dir_size = get_directory_size(directory);
		if (dir_size != 0)
		{
			FILE_FOUND = true;
			DetectCreatedFile(directory, BJ_DIRECTORY);
			break;
		}
	}

	// Terminates the URL detection thread.
	// This will be periodically terminated or restarted each time your program browses through the IP list.
	killed = true;
	delete directory;
}

void DetectServerAddress(TCHAR* dir, const TCHAR* server_url)
{
	size_t size = 0;
	path ps(dir);
	while (true)
	{
		if (killed) break;
		size = get_directory_size(dir);
		if (size != 0)
		{
			String sec;
			if(URL_OF_SECOND < 10) sec = String(_T("0") + to_wstring(URL_OF_SECOND));
			else sec = to_wstring(URL_OF_SECOND);

			wcout << endl << _T("Server URL detected: ") << server_url << sec;
			wcout << endl;
			break;
		}
	}
	delete DETECT_URL_THREAD;
	DETECT_URL_THREAD = NULL;
}

void DetectCreatedFile(const TCHAR *TCHAR_TARGET, const TCHAR* WORK_DIR)
{
	vector<String> *dir_list = get_file_list(TCHAR_TARGET);
	for (int list = 0; list < dir_list->size(); list++)
	{
		String filename = dir_list->at(list);
		path file_path(filename);
		if (is_regular_file(filename))
		{
			if (file_size(file_path) == 0)
			{
				remove(file_path);
				cout << file_path.string() + " was removed: NOT_FOUND_AND_DUMPED" << endl;
			}
			else
			{
				// Move to previous directory.
				String new_file = replace_all(filename, String(String(_T("\\") + String(WORK_DIR)) + _T("\\")), String(_T("\\")));

				// if the changed name was existed there, pending the suffix.
				if (exists(new_file)) new_file.append(_T(".filexist"));

				if (ends_with(filename, _T(".rpdownload")))
				{
					rename(filename, replace_all(new_file, _T(".rpdownload"), _T(".flv")));
					cout << file_path.string() + " is video file. saving to main directory" << endl;
				}
				else
				{
					rename(filename, new_file);
					cout << file_path.string() + " was expected the created file: FILE_MOVED" << endl;
				}
				remove(filename);
				wait(2000L);
			}
		}
		else if (is_directory(filename))
		{
			remove(file_path);
			cout << file_path.string() + " was cleared: OTHER_DETECTED" << endl;
		}
	}
	delete dir_list;
}

size_t get_directory_size(TCHAR* dir)
{
	size_t size = 0;
	path ps(dir);
	for (recursive_directory_iterator it(ps); it != recursive_directory_iterator(); ++it)
	{
		if (!is_directory(*it))
		{
			size += file_size(*it);
		}
	}
	return size;
}

vector<String> *get_file_list(const String& ph)
{
	vector<String> *dir_list = new vector<String>();
	boost::filesystem::directory_iterator iterator(ph);
	for (; iterator != boost::filesystem::directory_iterator(); ++iterator)
	{
		dir_list->push_back(iterator->path().wstring());
	}
	return dir_list;
}

inline TCHAR *ConvertToWideCharset(string &s)
{
	std::basic_string<TCHAR> tstr;
	const char* all = s.c_str();
	int len = 1 + strlen(all);
	TCHAR* t = new TCHAR[len];
	if (NULL == t) throw std::bad_alloc();
	mbstowcs(t, all, len);
	return (TCHAR*)t;
}

inline bool ends_with(String const & value, String const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

String replace_all(__in const String &message, __in const String &pattern, __in const String &replace)
{
	String result = message;
	String::size_type pos = 0;
	String::size_type offset = 0;
	while ((pos = result.find(pattern, offset)) != std::string::npos)
	{
		result.replace(result.begin() + pos, result.begin() + pos + pattern.size(), replace);
		offset = pos + replace.size();
	}
	return result;
}

void CreateDirectories(TCHAR* Path)
{
	TCHAR DirName[256];
	TCHAR* p = Path;
	TCHAR* q = DirName;
	while (*p) {
		if (('\\' == *p) || ('/' == *p))
		{
			if (':' != *(p - 1))
				CreateFolder(DirName, NO_SECURITY);
		}
		*q++ = *p++;
		*q = '\0';
	}
	CreateFolder(DirName, NO_SECURITY);
}

int main(int argc, char** argv)
{
	string          *SERVER_ADDRESS;
	string          *BJ_NAME           = new string();
	string          *BJ_STARTDATE      = new string();
	string          *PARTNER_CODE      = new string();
	string          *OUTPUT_DIRECTORY  = new string();

	// WOWZA MEDIA PLAYER PORT
	string          *WOWZA_PORT        = new string(":1935");
	string          *WOWZA_STREAM_PATH = new string("/pop_cast/_definst_/");
	string          *PARTNER_FORMAT    = new string("_P-000");

	TCHAR           *TCHAR_BJ_NAME     = new TCHAR[50]  { 0, };
	TCHAR           *PROGRAM_DIRECTORY = new TCHAR[512] { 0, };

	// If the appropriate parameter value is present when the program is run, 
	// Read the values and save it.
	if (argc > 1)
	{

	}

	// Read the program directory.
	GetCurrentDir(PROGRAM_DIRECTORY, PATH_LENGTH);

	// Check the existing the rtmpdump module. 
	path RTMPDUMP_PROCESSOR(String(PROGRAM_DIRECTORY).append(_T("\\rtmpdump.exe")));
	if (!exists(RTMPDUMP_PROCESSOR))
	{
		MessageBox(NULL, _T("rtmpdump related module or its native program was not found.\nMake sure you have this program or native process."),
			_T("RTMPDUMP NOT FOUND"), MB_ICONHAND);
		return 0;
	}

	// Initialize the address database.
	SERVER_ADDRESS = InitializeAddressArray();

	cout << "ReplayProcessor version 1.0.0 " << endl;
	cout << "Copyright 2018, Blacktree Network all rights reserved." << endl;

	// It is the process of entering values appropriately for a question.
	cout << "What is the BJ's ID > "; getline(cin, *BJ_NAME);
	cout << "Input the broadcast start date. The date format's yyyymmddhhmm > "; getline(cin, *BJ_STARTDATE);
	cout << "What is partner number? Generally, The partner code is 1, Sometimes 41. (default: 1) " << endl;
	cout << "If nothing is entered, set it to default value." << endl;
	cout << "Input the partner number (optional) > ";                            getline(cin, *PARTNER_CODE);
	cout << "Input the output filename (optional) > ";                           getline(cin, *OUTPUT_DIRECTORY);

	// If the user doesn't input the partner code, Set the default code.
	// For Windows system programming to work properly,
	// It uses a value with a Wide Character type.
	if (PARTNER_CODE->length() == 0)     *PARTNER_CODE = "01";
	TCHAR_BJ_NAME = ConvertToWideCharset(*BJ_NAME);

	int current = 0;
	int max_size = MAX_SIZE_ADDRESS;

	// Make a directory and change the current directory that BJ.
	CreateDirectories(TCHAR_BJ_NAME);
	SetCurrentDir(TCHAR_BJ_NAME);

	// This program will search for data using a list of server IP guesses.
	while (current < max_size)
	{
		string *SERVER_URL           = new string();
		string *SAVED_OUTPUT_DIR     = new string();

		// Search the database entirely through the server IP.
		string *ADDRESS              = &SERVER_ADDRESS[current];

		// The path that project directory. It uses the entire command.
		// The command consists of the following attributes:
		// $PROJECT_PATH\rtmpdump.exe or DLL library
		TCHAR *TCHAR_PROGRAM_PATH    = new TCHAR[4096]{ 0, };

		// the command arguments, It shows output name.
		// The command consists of the following attributes:
		// $OUTPUTFILES.$FORMAT 
		TCHAR *TCHAR_OUTPUT_FILE     = new TCHAR[PATH_LENGTH]{ 0, };

		// The command consists of the following attributes:
		// $SERVER_URL:$WOWZA_PORT/$WOWZA_PLAYER_PATH/$BJID_$PARTNERPATH_$STARTDATE
		TCHAR *TCHAR_SERVER_URL      = new TCHAR[PATH_LENGTH]{ 0, };

		// Configure the server URL.
		SERVER_URL->append(*ADDRESS + *WOWZA_PORT);
		SERVER_URL->append(*WOWZA_STREAM_PATH);
		SERVER_URL->append(*BJ_NAME);
		SERVER_URL->append(*PARTNER_FORMAT + *PARTNER_CODE + "_" + *BJ_STARTDATE);

		if(OUTPUT_DIRECTORY->length() == 0)
		{
			SAVED_OUTPUT_DIR->append(*BJ_NAME + "_" + *ADDRESS + "_" + *BJ_STARTDATE);
		}
		else
		{
			SAVED_OUTPUT_DIR = OUTPUT_DIRECTORY;
		}

		String *PROGRAM_NAME = new String(_T("\\"));

		// Configure the command.
		// Insert the column for preventing path that include the spaces.
		// Also, Read the RTMPDUMP process from the program directory.
		_tcscat(TCHAR_PROGRAM_PATH, _T("\""));
		_tcscat(TCHAR_PROGRAM_PATH, PROGRAM_DIRECTORY);
		_tcscat(TCHAR_PROGRAM_PATH, _T("\\rtmpdump.exe\""));

		// For Windows system programming to work properly,
		// It uses a value with a Wide Character type.
		TCHAR_SERVER_URL  = ConvertToWideCharset(*SERVER_URL);
		TCHAR_OUTPUT_FILE = ConvertToWideCharset(*SAVED_OUTPUT_DIR);

		ScarchWowzaPlayerURL(TCHAR_PROGRAM_PATH, TCHAR_SERVER_URL, TCHAR_OUTPUT_FILE, TCHAR_BJ_NAME);

		delete(PROGRAM_NAME);
		delete(SERVER_URL);

		delete[](TCHAR_SERVER_URL);
		delete[](TCHAR_OUTPUT_FILE);
		delete[](TCHAR_PROGRAM_PATH);
		if (FILE_FOUND)
		{
			cout << "The file was found and download completely. Check the file in the folder." << endl;
			system("pause");
			break;
		}
		current++;
	}

	/*
	delete   BJ_NAME;
	delete   BJ_STARTDATE;
	delete   PARTNER_CODE;
	delete   OUTPUT_DIRECTORY;
	delete   WOWZA_PORT;
	delete   WOWZA_STREAM_PATH;
	delete   PARTNER_FORMAT;
	delete   PROGRAM_DIRECTORY;
	delete[] TCHAR_BJ_NAME;
	delete[] SERVER_ADDRESS;
	*/

	return 0;
}

