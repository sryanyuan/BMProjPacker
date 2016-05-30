#include "CommandLineHelper.h"
//////////////////////////////////////////////////////////////////////////
CommandLineHelper::CommandLineHelper()
{
	m_cSplitterChar = '=';
}

CommandLineHelper::~CommandLineHelper()
{

}

bool CommandLineHelper::InitParam()
{
	ReleaseParam();

	int nArgs = 0;
	LPWSTR *szArglist = NULL;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	if( NULL != szArglist)   
	{   
		//szArglist���Ǳ������������   
		//nArgs�������в����ĸ���   
		//����ĵ�һ��Ԫ�ر�ʾ���̵�path��Ҳ����szArglist[0]��������Ԫ���������������
		for(int i = 0; i < nArgs; ++i)
		{
			char szArgs[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, szArglist[i], -1, szArgs, MAX_PATH, NULL, NULL);
			string xParam = szArgs;
			m_xParamList.push_back(xParam);
		}

		//ȡ�ò������ͷ�CommandLineToArgvW����Ŀռ�   
		LocalFree(szArglist);

		return true;
	}   

	return false;
}

void CommandLineHelper::ReleaseParam()
{
	m_xParamList.clear();
}

const char* CommandLineHelper::GetParam(const char* _pszSection)
{
	if(m_xParamList.empty())
	{
		return false;
	}

	AppParamList::const_iterator begIter = m_xParamList.begin();
	AppParamList::const_iterator endIter = m_xParamList.end();

	for(begIter;
		begIter != endIter;
		++begIter)
	{
		const string& xStr = *begIter;
		size_t fndPos = xStr.find(_pszSection);
		if(fndPos == 0)
		{
			const char* pszCStyleValue = xStr.c_str();
			if(strlen(pszCStyleValue) > strlen(_pszSection) + 1)
			{
				if(pszCStyleValue[strlen(_pszSection)] == m_cSplitterChar)
				{
					//	OK
					return (pszCStyleValue + strlen(_pszSection) + 1);
				}
			}
		}
	}

	return NULL;
}