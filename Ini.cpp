// Ini.cpp: CIni �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "Ini.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CIni::CIni(LPCTSTR lpszIniFilename)
{
	this->m_lpszIniFilename =
		(LPTSTR)malloc((lstrlen(lpszIniFilename) + 1) * sizeof(TCHAR));

	lstrcpy(this->m_lpszIniFilename, lpszIniFilename);
}

CIni::~CIni()
{
	if (this->m_lpszIniFilename)
	{
		free(this->m_lpszIniFilename);
	}
}

BOOL CIni::IniFileOpen(void)
{

}