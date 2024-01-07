//=====================================================================
//
// PyStand.cpp - 
//
// Created by skywind on 2022/02/03
// Last Modified: 2023/03/17 20:06
//
//=====================================================================
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <shlwapi.h>
#include <string>
#include <string.h>
#include <winbase.h>
#include <wincon.h>
#include "PyStand.h"

#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif


//---------------------------------------------------------------------
// dtor
//---------------------------------------------------------------------
PyStand::~PyStand()
{
	FreeLibrary(_hDLL);
}


//---------------------------------------------------------------------
// ctor
//---------------------------------------------------------------------
PyStand::PyStand(const wchar_t *runtime)
{
	_hDLL = NULL;
	_Py_Main = NULL;
	if (CheckEnviron(runtime) == false) {
		exit(1);
	}
	if (LoadPython() == false) {
		exit(2);
	}
}


//---------------------------------------------------------------------
// ctor for ansi
//---------------------------------------------------------------------
PyStand::PyStand(const char *runtime)
{
	_hDLL = NULL;
	_Py_Main = NULL;
	std::wstring rtp = Ansi2Unicode(runtime);
	if (CheckEnviron(rtp.c_str()) == false) {
		exit(1);
	}
	if (LoadPython() == false) {
		exit(2);
	}
}


//---------------------------------------------------------------------
// char to wchar_t
//---------------------------------------------------------------------
std::wstring PyStand::Ansi2Unicode(const char *text)
{
    // 计算输入字符串的长度
    int len = (int)strlen(text);
    // 创建一个空的宽字符字符串
    std::wstring wide;
    // 根据输入的字符串长度计算转换后的宽字符字符串的长度
    int require = MultiByteToWideChar(CP_ACP, 0, text, len, NULL, 0);
    // 如果转换后的宽字符字符串长度大于0
    if (require > 0) {
        // 调整宽字符字符串的大小为转换后的长度
        wide.resize(require);
        // 将输入的ANSI字符串转换为Unicode字符串
        MultiByteToWideChar(CP_ACP, 0, text, len, &wide[0], require);
    }
    // 返回转换后的Unicode字符串
    return wide;
}


//---------------------------------------------------------------------
// init: _args, _argv, _cwd, _pystand, _home, _runtime, 
//---------------------------------------------------------------------
bool PyStand::CheckEnviron(const wchar_t *rtp)
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
	LocalFree(argvw);

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

	// init: _runtime (embedded python directory)
	bool abspath = false;
	if (wcslen(rtp) >= 3) {
		if (rtp[1] == L':') {
			if (rtp[2] == L'/' || rtp[2] == L'\\')
				abspath = true;
		}
	}
	if (abspath == false) {
		_runtime = _home + L"\\" + rtp;
	}
	else {
		_runtime = rtp;
	}
	GetFullPathNameW(_runtime.c_str(), MAX_PATH + 1, path, NULL);
	_runtime = path;

	// check home
	std::wstring check = _runtime;
	if (!PathFileExistsW(check.c_str())) {
		std::wstring msg = L"Missing embedded Python3 in:\n" + check;
		MessageBoxW(NULL, msg.c_str(), L"ERROR", MB_OK);
		return false;
	}

	// check python3.dll
	std::wstring check2 = _runtime + L"\\python3.dll";
	if (!PathFileExistsW(check2.c_str())) {
		std::wstring msg = L"Missing python3.dll in:\r\n" + check;
		MessageBoxW(NULL, msg.c_str(), L"ERROR", MB_OK);
		return false;
	}

	// setup environment
	SetEnvironmentVariableW(L"PYSTAND", _pystand.c_str());
	SetEnvironmentVariableW(L"PYSTAND_HOME", _home.c_str());
	SetEnvironmentVariableW(L"PYSTAND_RUNTIME", _runtime.c_str());


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
	std::wstring runtime = _runtime;
	std::wstring previous;

	// save current directory
	wchar_t path[MAX_PATH + 10];
	GetCurrentDirectoryW(MAX_PATH + 1, path);
	previous = path;

	// python dll must be load under "runtime"
	SetCurrentDirectoryW(runtime.c_str());
	// LoadLibrary
	_hDLL = (HINSTANCE)LoadLibraryA("python3.dll");
	if (_hDLL) {
		_Py_Main = (t_Py_Main)GetProcAddress(_hDLL, "Py_Main");
	}	

	// restore director
	SetCurrentDirectoryW(previous.c_str());

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
// run string
//---------------------------------------------------------------------
int PyStand::RunString(const wchar_t *script)
{
    if (_Py_Main == NULL) {
        return -1;
    }
    int hr = 0;
    int i;
    _py_argv.resize(0);
    // 初始化参数
    _py_argv.push_back(_argv[0]);
    _py_argv.push_back(L"-I");
    _py_argv.push_back(L"-s");
    _py_argv.push_back(L"-S");
    _py_argv.push_back(L"-c");
    _py_argv.push_back(script);
    for (i = 1; i < (int)_argv.size(); i++) {
        _py_argv.push_back(_argv[i]);
    }
    // 终止参数
    _py_args.resize(0);
    for (i = 0; i < (int)_py_argv.size(); i++) {
        _py_args.push_back((wchar_t*)_py_argv[i].c_str());
    }
    hr = _Py_Main((int)_py_args.size(), &_py_args[0]);
    return hr;
}


//---------------------------------------------------------------------
// run ansi string
//---------------------------------------------------------------------
int PyStand::RunString(const char *script)
{
	std::wstring text = Ansi2Unicode(script);
	return RunString(text);
}



//---------------------------------------------------------------------
// LoadScript()
//---------------------------------------------------------------------
int PyStand::DetectScript()
{
	// init: _script (init script like PyStand.int or PyStand.py)
	int size = (int)_pystand.size() - 1;
	for (; size >= 0; size--) {
		if (_pystand[size] == L'.') break;
	}
	if (size < 0) size = (int)_pystand.size();
	std::wstring main = _pystand.substr(0, size);
	std::vector<const wchar_t*> exts;
	std::vector<std::wstring> scripts;
	_script.clear();
#if !(PYSTAND_DISABLE_STATIC)
	std::wstring test;
	test = _home + L"\\" + Ansi2Unicode(PYSTAND_STATIC_NAME);
	if (PathFileExistsW(test.c_str())) {
		_script = test;
	}
#endif
	if (_script.empty()) {
		exts.push_back(L".int");
		exts.push_back(L".py");
		exts.push_back(L".pyw");
		for (int i = 0; i < (int)exts.size(); i++) {
			std::wstring test = main + exts[i];
			scripts.push_back(test);
			if (PathFileExistsW(test.c_str())) {
				_script = test;
				break;
			}
		}
		if (_script.size() == 0) {
			// No default script found, try import the app and run it.
			std::wstring app;
			app = _home + L"\\app";
			if (PathFileExistsW(app.c_str())) {
				_script = app;
			}else{
				std::wstring msg = L"Can't find either of:\r\n";
				for (int j = 0; j < (int)scripts.size(); j++) {
					msg += scripts[j] + L"\r\n";
				}
				msg += app + L"\r\n";
				MessageBoxW(NULL, msg.c_str(), L"ERROR", MB_OK);
				return -1;
			}

		}
	}
	SetEnvironmentVariableW(L"PYSTAND_SCRIPT", _script.c_str());
	return 0;
}



//---------------------------------------------------------------------
// init script
//---------------------------------------------------------------------
const char *init_script = 
"import sys\n"
"import os\n"
"import site\n"
"PYSTAND = os.environ['PYSTAND']\n"
"PYSTAND_HOME = os.environ['PYSTAND_HOME']\n"
"sys.path_origin = [n for n in sys.path]\n"
"sys.PYSTAND = PYSTAND\n"
"sys.PYSTAND_HOME = PYSTAND_HOME\n"
"def MessageBox(msg, info = 'Message'):\n"
"    import ctypes\n"
"    ctypes.windll.user32.MessageBoxW(None, str(msg), str(info), 0)\n"
"    return 0\n"
"os.MessageBox = MessageBox\n"
"for n in ['.', 'lib', 'site-packages']:\n"
"    test = os.path.abspath(os.path.join(PYSTAND_HOME, n))\n"
"    if os.path.exists(test):\n"
"        sys.path.append(test)\n"
"try:\n"
"    from app import run\n"
"    run()\n"
"except Exception as e:\n"
"    os.MessageBox(str(e), 'Error')\n"
"";



//---------------------------------------------------------------------
// main
//---------------------------------------------------------------------

//! flag: -static
//! src: 
//! link: stdc++, shlwapi, resource.o
//! prebuild: windres resource.rc -o resource.o
//! mode: win
//! int: objs
int WINAPI // 此处为 WINAPI 宏，用于标识这是一个 Win32 API 函数
WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int show) // 主程序入口函数，接收实例化程序的句柄 hInst，前一个实例的句柄 hPrevInst，命令行参数 args 和显示状态 show
{
	PyStand ps("bin"); // 创建 PyStand 对象 ps，并传入参数 "bin"
	// if (ps.DetectScript() != 0) { // 调用 PyStand 对象的 DetectScript 方法，若返回值不为 0
	// 	return 3; // 返回 3
	// }

	int hr = ps.RunString(init_script); // 调用 PyStand 对象的 RunString 方法，将 init_script 作为参数，并将返回值赋给 hr
	// printf("finalize\n"); // 打印字符串 "finalize"
	return hr; // 返回 hr
}
