//=====================================================================
//
// PyStand.cpp - 
//
// Created by skywind on 2022/02/03
// Last Modified: 2022/02/03 23:39:54
//
//=====================================================================
#include "PyStand.h"
#include <shlwapi.h>
#include <winbase.h>

#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif


//---------------------------------------------------------------------
// dtor
//---------------------------------------------------------------------
PyStand::~PyStand()
{
}


//---------------------------------------------------------------------
// ctor
//---------------------------------------------------------------------
PyStand::PyStand()
{
	_hDLL = NULL;
	_Py_Main = NULL;
	if (CheckEnviron() == false) {
		exit(1);
	}
	if (LoadPython() == false) {
		exit(2);
	}
}


//---------------------------------------------------------------------
// init: _args, _argv, _cwd, _pystand, _home
//---------------------------------------------------------------------
bool PyStand::CheckEnviron()
{
	// init: _args, _argv
	LPWSTR *argvw;
	int argc;
	_args = GetCommandLineW();
	argvw = CommandLineToArgvW(_args.c_str(), &argc);
	if (argvw == NULL) {
		MessageBoxA(NULL, "Error in CommandLineToArgvW()", "ERROR", MB_OK);
		return false;
	}
	_argv.resize(argc);
	for (int i = 0; i < argc; i++) {
		_argv[i] = argvw[i];
	}

	// init: _cwd (current working directory)
	wchar_t path[MAX_PATH + 10];
	GetCurrentDirectoryW(MAX_PATH + 1, path);
	_cwd = path;

	// init: _pystand (full path of PyStand.exe)
	GetModuleFileNameW(NULL, path, MAX_PATH + 1);
	_pystand = path;

	// init: _home
	int size = (int)wcslen(path);
	for (; size > 0; size--) {
		if (path[size - 1] == L'/') break;
		if (path[size - 1] == L'\\') break;
	}
	path[size] = 0;
	SetCurrentDirectoryW(path);
	GetCurrentDirectoryW(MAX_PATH + 1, path);
	_home = path;
	SetCurrentDirectoryW(_cwd.c_str());

	// check home
	std::wstring check = _home + L"\\runtime";
	if (!PathFileExistsW(check.c_str())) {
		std::wstring msg = L"Missing embedded Python3 in:\n" + check;
		MessageBoxW(NULL, msg.c_str(), L"ERROR", MB_OK);
		return false;
	}

	// check python3.dll
	std::wstring check2 = _home + L"\\runtime\\python3.dll";
	if (!PathFileExistsW(check2.c_str())) {
		std::wstring msg = L"Missing python3.dll in:\r\n" + check;
		MessageBoxW(NULL, msg.c_str(), L"ERROR", MB_OK);
		return false;
	}

#if 0
	wprintf(L"%s - %s\n", _pystand.c_str(), path);
	MessageBoxW(NULL, _pystand.c_str(), _home.c_str(), MB_OK);
#endif

	return true;
}


//---------------------------------------------------------------------
// load python
//---------------------------------------------------------------------
bool PyStand::LoadPython()
{
	std::wstring runtime = _home + L"\\runtime";
	// python dll must be load under "runtime"
	SetCurrentDirectoryW(runtime.c_str());
	// LoadLibrary
	_hDLL = (HINSTANCE)LoadLibraryA("python3.dll");
	if (_hDLL) {
		_Py_Main = (t_Py_Main)GetProcAddress(_hDLL, "Py_Main");
	}	
	// restore director
	SetCurrentDirectoryW(_cwd.c_str());
	if (_hDLL == NULL) {
		std::wstring msg = L"Cannot load python3.dll from:\r\n" + runtime;
		MessageBoxW(NULL, msg.c_str(), L"ERROR", MB_OK);
		return false;
	}
	else if (_Py_Main == NULL) {
		std::wstring msg = L"Cannot find Py_Main() in:\r\n";
		msg += runtime + L"\\python3.dll";
		MessageBoxW(NULL, msg.c_str(), L"ERROR", MB_OK);
		return false;
	}
	return true;
}


//---------------------------------------------------------------------
// entry
//---------------------------------------------------------------------

//! flag: -static
//! src: 
//! mode: win
//! int: objs
int APIENTRY 
WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int show)
{
	PyStand ps;

	// MessageBoxA(NULL, "Hello, World !!", "DD", MB_OK);
	return 0;
}


