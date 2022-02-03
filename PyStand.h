//=====================================================================
//
// PyStand.h - 
//
// Created by skywind on 2022/02/03
// Last Modified: 2022/02/03 23:39:52
//
//=====================================================================
#ifndef _PYSTAND_H_
#define _PYSTAND_H_

#include <stdio.h>
#include <windows.h>
#include <shlwapi.h>
#include <string>
#include <vector>


//---------------------------------------------------------------------
// PyStand
//---------------------------------------------------------------------
class PyStand
{
public:
	virtual ~PyStand();
	PyStand();

protected:
	bool CheckEnviron();
	bool LoadPython();

protected:
	typedef int (*t_Py_Main)(int argc, wchar_t **argv);
	t_Py_Main _Py_Main;

protected:
	HINSTANCE _hDLL;
	std::wstring _cwd;		// current working directory
	std::wstring _args;		// arguments
	std::wstring _pystand;	// absolute path of pystand
	std::wstring _home;		// home directory of PyStand.exe
	std::vector<std::wstring> _argv;
};


#endif

