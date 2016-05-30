// ProjPacker.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "CommandLineHelper.h"
#include <Shlwapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>

#pragma warning(disable: 4996)
#pragma comment(lib, "shlwapi.lib")

int  removeDir(const char*  dirPath)
{

	struct _finddata_t fb;   //查找相同属性文件的存储结构体
	char  path[250];          
	long    handle;
	int  resultone;
	int   noFile;            //对系统隐藏文件的处理标记
	
	noFile = 0;
	handle = 0;

	
	//制作路径
	strcpy(path,dirPath);
    strcat (path,"/*");

	handle = _findfirst(path,&fb);
	//找到第一个匹配的文件
	if (handle != 0)
	{
		//当可以继续找到匹配的文件，继续执行
		while (0 == _findnext(handle,&fb))
		{
			//windows下，常有个系统文件，名为“..”,对它不做处理
			noFile = strcmp(fb.name,"..");
			
			if (0 != noFile)
			{
				//制作完整路径
				memset(path,0,sizeof(path));
				strcpy(path,dirPath);
				strcat(path,"/");
				strcat (path,fb.name);
				//属性值为16，则说明是文件夹，迭代
				if ((fb.attrib & _A_SUBDIR) != 0)
				{
					 removeDir(path);	
				}
				//非文件夹的文件，直接删除。对文件属性值的情况没做详细调查，可能还有其他情况。
				else
				{
					remove(path);
				}
			}	
		}
		//关闭文件夹，只有关闭了才能删除。找这个函数找了很久，标准c中用的是closedir
		//经验介绍：一般产生Handle的函数执行后，都要进行关闭的动作。
		_findclose(handle);
	}
		//移除文件夹
		resultone = rmdir(dirPath);
		return  resultone;
}

int run_command(const char* _pszFile, const char* _pszParameters)
{
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = _pszFile;
	ShExecInfo.lpParameters = _pszParameters;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;
	if(ShellExecuteEx(&ShExecInfo))
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(ShExecInfo.hProcess,INFINITE))
		{
			return 0;
		}
		else
		{
			printf("Copy project failed\n");
			return -8;
		}
	}
	else
	{
		int nErr = GetLastError();
		printf("ShellExecuteEx error:%d\n", nErr);
		return -9;
	}

	return -1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	CommandLineHelper hlp;
	if(!hlp.InitParam())
	{
		return -1;
	}

	const char* pszOutput = hlp.GetParam("output");
	if(NULL == pszOutput)
	{
		printf("Invalid output parameter\n");
		return -3;
	}

	const char* pszRoot = hlp.GetParam("root");
	if(NULL == pszRoot)
	{
		printf("Invalid root parameter\n");
		return -4;
	}

	const char* pszPsw = hlp.GetParam("password");
	if(NULL == pszPsw)
	{
		pszPsw = "";
	}

	//	doing process
	char szWorkingPath[MAX_PATH];
	GetModuleFileName(NULL, szWorkingPath, sizeof(szWorkingPath));
	PathRemoveFileSpec(szWorkingPath);

	char szDir[MAX_PATH];
	sprintf(szDir, "%s\\tmp", szWorkingPath);
	if(PathFileExists(szDir))
	{
		removeDir(szDir);
	}

	if(0 != mkdir(szDir))
	{
		return -5;
	}

	char szCmd[MAX_PATH * 2];
	sprintf(szCmd, "%s\\BackMirClient %s\\BackMirClient\\ /s /i /y", pszRoot, szDir);
	if(0 != run_command("xcopy", szCmd))
	{
		printf("Failed to copy BackMirClient project\n");
		return -6;
	}

	sprintf(szCmd, "%s\\BackMirServer %s\\BackMirServer\\ /s /i /y", pszRoot, szDir);
	if(0 != run_command("xcopy", szCmd))
	{
		printf("Failed to copy BackMirServer project\n");
		return -7;
	}

	sprintf(szCmd, "%s\\CommonModule %s\\CommonModule\\ /s /i /y", pszRoot, szDir);
	if(0 != run_command("xcopy", szCmd))
	{
		printf("Failed to copy CommonModule project\n");
		return -8;
	}

	sprintf(szCmd, "%s\\BackMir\\Config %s\\Config\\ /s /i /y", pszRoot, szDir);
	if(0 != run_command("xcopy", szCmd))
	{
		printf("Failed to copy config files\n");
		return -9;
	}

	sprintf(szCmd, "%s\\BackMir\\Help %s\\Help\\ /s /i /y", pszRoot, szDir);
	if(0 != run_command("xcopy", szCmd))
	{
		printf("Failed to copy help files\n");
		return -10;
	}

	//	zip file...
	if(pszPsw[0] != 0)
	{
		sprintf(szCmd, "a -tzip -p%s %s %s\\*", pszPsw, pszOutput, szDir);
	}
	else
	{
		sprintf(szCmd, "a -tzip %s %s\\*", pszOutput, szDir);
	}
	
	if(0 != run_command("7z", szCmd))
	{
		printf("Failed to execute 7z\n");
		return -10;
	}

	//	remove all temporary files
	removeDir(szDir);

	printf("Done!\n");

	return 0;
}

