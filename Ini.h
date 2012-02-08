// Ini.h: CIni クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INI_H__87F285E0_1D5F_47BA_8C09_6AE0E7664F12__INCLUDED_)
#define AFX_INI_H__87F285E0_1D5F_47BA_8C09_6AE0E7664F12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIni  
{
private:
	LPTSTR m_lpszIniFilename;
public:
	CIni(LPCTSTR lpszIniFilename);
	virtual ~CIni();

};

#endif // !defined(AFX_INI_H__87F285E0_1D5F_47BA_8C09_6AE0E7664F12__INCLUDED_)
