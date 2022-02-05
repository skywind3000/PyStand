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
#include <string>
#include <winbase.h>
#include <wincon.h>

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
	int require = (int)strlen(text) * 2 + 10;
	std::wstring wide;
	wide.resize(require + 2);
	MultiByteToWideChar(CP_ACP, 0, text, -1, &wide[0], require);
	int size = wcslen(wide.c_str());
	wide.resize(size);
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
	SetEnvironmentVariableW(L"PYSTAND_SCRIPT", _script.c_str());

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
// run string
//---------------------------------------------------------------------
int PyStand::RunString(const wchar_t *script)
{
	if (_Py_Main == NULL) {
		return -1;
	}
	int hr = 0;
	int i;
	std::vector<std::wstring> argv;
	// init arguments
	argv.push_back(_argv[0]);
	argv.push_back(L"-I");
	argv.push_back(L"-s");
	argv.push_back(L"-S");
	argv.push_back(L"-c");
	argv.push_back(script);
	for (i = 1; i < (int)_argv.size(); i++) {
		argv.push_back(_argv[i]);
	}
	// finalize arguments
	std::vector<wchar_t*> real_argv;
	for (i = 0; i < (int)argv.size(); i++) {
		real_argv.push_back((wchar_t*)argv[i].c_str());
	}
	hr = _Py_Main((int)argv.size(), &real_argv[0]);
	return hr;
}


//---------------------------------------------------------------------
// run ansi string
//---------------------------------------------------------------------
int PyStand::RunString(const char *script)
{
	std::wstring text = Ansi2Unicode(script);
	return RunString(text.c_str());
}


//---------------------------------------------------------------------
// LoadScript()
//---------------------------------------------------------------------
int PyStand::DetectScript()
{
	// init: _script (init script like PyStand.int or PyStand.py)
	int size = (int)_pystand.size();
	for (; size > 0; size--) {
		if (_pystand[size] == L'.') break;
	}
	std::wstring main = _pystand.substr(0, size);
	std::vector<const wchar_t*> exts;
	std::vector<std::wstring> scripts;
	exts.push_back(L".int");
	exts.push_back(L".py");
	exts.push_back(L".pyw");
	_script = L"";
	for (int i = 0; i < (int)exts.size(); i++) {
		std::wstring test = main + exts[i];
		scripts.push_back(test);
		if (PathFileExistsW(test.c_str())) {
			_script = test;
			break;
		}
	}
	if (_script.size() == 0) {
		std::wstring msg = L"Can't find either of:\r\n";
		for (int j = 0; j < (int)scripts.size(); j++) {
			msg += scripts[j] + L"\r\n";
		}
		MessageBoxW(NULL, msg.c_str(), L"ERROR", MB_OK);
		return -1;
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
"import copy\n"
"PYSTAND = os.environ['PYSTAND']\n"
"PYSTAND_HOME = os.environ['PYSTAND_HOME']\n"
"PYSTAND_RUNTIME = os.environ['PYSTAND_RUNTIME']\n"
"PYSTAND_SCRIPT = os.environ['PYSTAND_SCRIPT']\n"
"sys.path_origin = [n for n in sys.path]\n"
"def MessageBox(msg, info = 'Message'):\n"
"    import ctypes\n"
"    ctypes.windll.user32.MessageBoxW(None, str(msg), str(info), 0)\n"
"    return 0\n"
"os.MessageBox = MessageBox\n"
"try:\n"
"    fd = os.open('CONOUT$', os.O_RDWR | os.O_BINARY)\n"
"    fp = os.fdopen(fd, 'w')\n"
"    sys.stdout = fp\n"
"    sys.stderr = fp\n"
"except Exception as e:\n"
"    pass\n"
"for n in ['lib', 'site-packages']:\n"
"    test = os.path.join(PYSTAND_HOME, n)\n"
"    if os.path.exists(test): sys.path.append(test)\n"
"sys.path.append(os.path.abspath(PYSTAND_HOME))\n"
"sys.argv = [PYSTAND_SCRIPT] + sys.argv[1:]\n"
"text = open(PYSTAND_SCRIPT).read()\n"
"environ = {'__file__': PYSTAND_SCRIPT}\n"
"saveloc = copy.copy(locals())\n"
"for k in saveloc:\n"
"    if k.startswith('__'):\n"
"        environ[k] = saveloc[k]\n"
"exec(text, globals(), environ)\n"
"";


//---------------------------------------------------------------------
// main
//---------------------------------------------------------------------

//! flag: -static
//! src: 
//! mode: win
//! int: objs
int APIENTRY 
WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int show)
{
	PyStand ps("runtime");
	if (ps.DetectScript() != 0) {
		return 3;
	}
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		freopen("CONIN$", "r", stdin);
		int fd = fileno(stdout);
		if (fd >= 0) {
			std::string fn = std::to_string(fd);
			SetEnvironmentVariableA("PYSTAND_STDOUT", fn.c_str());
		}
		fd = fileno(stdin);
		if (fd >= 0) {
			std::string fn = std::to_string(fd);
			SetEnvironmentVariableA("PYSTAND_STDIN", fn.c_str());
		}
	}
	int hr = ps.RunString(init_script);
	// printf("finalize\n");
	return hr;
}


