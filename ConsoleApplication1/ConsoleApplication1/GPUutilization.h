#pragma once

#include <Pdh.h>
#include <map>
#include <vector>
#include <atlstr.h> 
typedef struct _sCounter
{
	HCOUNTER hCounter;
	LPCTSTR lpInstance;
	_sCounter()
	{
		hCounter = NULL;
		lpInstance = NULL;
	}
	_sCounter& operator=(const _sCounter& rici)
	{
		hCounter = rici.hCounter;
		lpInstance = rici.lpInstance;
		return *this;
	}
}SCOUNTER, *PSCOUNTER;
//CPU 利用率类
class GPUutilization
{
public:
	GPUutilization(LPCTSTR pName);
	~GPUutilization(void);

	void Init();
	void UnInit();
	CString GetUtilization();
	int GetCpuNum();
	void MakeCounterPath();
	HANDLE  GetProcessHandle(int nID);
	unsigned long GetProcessHandle(LPCTSTR pName);
	void GetAllProcessID();
	void EeumObjectItem(LPCTSTR strObjectName);
private:
	int m_nCPUCount;
	std::map<HQUERY, SCOUNTER> m_mapHquery;
	HQUERY m_hCPUQuery;
	HQUERY m_hPathQuery;
	HCOUNTER m_HCounter;
	std::map<LPCTSTR, unsigned long> m_mapProcessID;
	std::vector<CString> m_vecInstance;
	LPCTSTR m_strExeName;
};
