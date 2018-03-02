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
#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <String>
#include <vector>

#include <tchar.h>
#include "boost/filesystem.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#ifndef UNICODE  
typedef std::string String;
#else
typedef std::wstring String;
#endif

#define MAX_SIZE_ADDRESS 9

using namespace std;
using namespace boost::filesystem;

__declspec(dllexport) 
void    ScarchWowzaPlayerURL(__in const TCHAR *TCHAR_PROGRAM_DIR,__in const TCHAR *TCHAR_SERVER_URL, __in const TCHAR *TCHAR_OUTPUT_FILE,
	__in const TCHAR *BJ_DIRECTORY);

__declspec(dllexport) 
string* InitializeAddressArray();
void    DetectCreatedFile(const TCHAR *TCHAR_TARGET, const TCHAR *WORK_DIR);
void    ScarchServerAddress(const TCHAR *TCHAR_SERVER_URL, const TCHAR *TCHAR_PROGRAM_DIR, const TCHAR *TCHAR_OUTPUT_FILE, const TCHAR *BJ_DIRECTORY);
void    DetectServerAddress(TCHAR* dir, const TCHAR* server_url);


inline TCHAR   *ConvertToWideCharset(string &str);

vector<String> *get_file_list(const String& ph);

inline bool    ends_with(String const &value, String const &ending);

size_t         get_directory_size(TCHAR* dir);

String replace_all(
	__in const String &message,
	__in const String &pattern,
	__in const String &replace
);

namespace replay
{
	class Processor
	{
	private:
		string          *RTMP_OPTIONAL = new string("-r rtmp://");

		string          *BJ_NAME = new string();
		string          *BJ_STARTDATE = new string();
		string          *PARTNER_CODE = new string();
		string          *OUTPUT_DIRECTORY = new string();

		TCHAR           *TCHAR_BJ_NAME = new TCHAR[50]{ 0, };
		TCHAR           *PROGRAM_DIRECTORY = new TCHAR[512]{ 0, };
	};

	class AddressSelector
	{
		private:
			// WOWZA MEDIA PLAYER PORT
			String *SERVER_ADDRESS;
			String *WOWZA_PORT           = new String(L":1935");
			String *WOWZA_STREAM_PATH    = new String(L"/pop_cast/_definst_/");
			String *PARTNER_FORMAT       = new String(L"_P-000");
		public:
			String *get_wowza_port()     { return this->WOWZA_PORT;  }
			String *get_stream_path()    { return this->WOWZA_STREAM_PATH;  }
			String *get_partner_format() { return this->PARTNER_FORMAT; }
			AddressSelector() {};
			~AddressSelector() {};
	};
}