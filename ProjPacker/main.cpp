// ProjPacker.cpp : �������̨Ӧ�ó������ڵ㡣
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

	struct _finddata_t fb;   //������ͬ�����ļ��Ĵ洢�ṹ��
	char  path[250];          
	long    handle;
	int  resultone;
	int   noFile;            //��ϵͳ�����ļ��Ĵ�����
	
	noFile = 0;
	handle = 0;

	
	//����·��
	strcpy(path,dirPath);
    strcat (path,"/*");

	handle = _findfirst(path,&fb);
	//�ҵ���һ��ƥ����ļ�
	if (handle != 0)
	{
		//�����Լ����ҵ�ƥ����ļ�������ִ��
		while (0 == _findnext(handle,&fb))
		{
			//windows�£����и�ϵͳ�ļ�����Ϊ��..��,������������
			noFile = strcmp(fb.name,"..");
			
			if (0 != noFile)
			{
				//��������·��
				memset(path,0,sizeof(path));
				strcpy(path,dirPath);
				strcat(path,"/");
				strcat (path,fb.name);
				//����ֵΪ16����˵�����ļ��У�����
				if ((fb.attrib & _A_SUBDIR) != 0)
				{
					 removeDir(path);	
				}
				//���ļ��е��ļ���ֱ��ɾ�������ļ�����ֵ�����û����ϸ���飬���ܻ������������
				else
				{
					remove(path);
				}
			}	
		}
		//�ر��ļ��У�ֻ�йر��˲���ɾ����������������˺ܾã���׼c���õ���closedir
		//������ܣ�һ�����Handle�ĺ���ִ�к󣬶�Ҫ���йرյĶ�����
		_findclose(handle);
	}
		//�Ƴ��ļ���
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

