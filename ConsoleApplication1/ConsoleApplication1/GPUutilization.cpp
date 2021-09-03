#include "pch.h"
#include "GPUutilization.h" 
#include <cmath>
#include <iostream>
#include <TlHelp32.h>
#include <pdhmsg.h>
using namespace std; 


#pragma comment(lib,"pdh.lib")

int _get_cpu_num_win32()
{
	/* pointer to GetActiveProcessorCount() */
	int nCPUs = 0;
	typedef DWORD(WINAPI *GETACTIVEPC)(WORD);
	GETACTIVEPC	get_act = NULL;

	if (NULL == get_act)
		get_act = (GETACTIVEPC)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetActiveProcessorCount");

	if (NULL != get_act)
	{
		nCPUs = (int)get_act(0xffff);
	}
	else
	{
		SYSTEM_INFO sysInfo;
		GetNativeSystemInfo(&sysInfo);
		nCPUs = (int)sysInfo.dwNumberOfProcessors;
	}

	return nCPUs;
}

GPUutilization::GPUutilization(LPCTSTR pName)
	: m_nCPUCount(0)
	, m_HCounter(0)
	, m_strExeName(pName)
{
}

GPUutilization::~GPUutilization(void)
{
}

void GPUutilization::Init()
{
	EeumObjectItem(_T("GPU Adapter Memory"));
	MakeCounterPath();
	auto it = m_vecInstance.begin();
	for (; it != m_vecInstance.end(); ++it)
	{
		HQUERY hCPUQuery = NULL;
		HCOUNTER hCounter = NULL;
		SCOUNTER scounter;
		scounter.hCounter = hCounter;
		scounter.lpInstance = *it;

		PdhOpenQuery(NULL, NULL, &hCPUQuery);

		m_mapHquery[hCPUQuery] = scounter;
	}


	//GPU Engine(pid_3416_luid_0x00000000_0x0000BE27_phys_0_eng_0_engtype_3D)\\Utilization Percentage
	//pid_23168_luid_0x00000000_0x0000B9E3_phys_0_eng_1_engtype_VideoDecode
	CString szFullCounterPath;
	CString szObjectName = _T("\\GPU Engine");
	CString szEngType = _T("VideoDecode");
	CString szProcessID;
	unsigned long nPID = GetProcessHandle(m_strExeName);
	auto itHquery = m_mapHquery.begin();
	for (; itHquery != m_mapHquery.end(); ++itHquery)
	{
		//HQUERY hCPUQuery = (itHquery->first);
		if (itHquery->first != NULL)
		{
			CString szInstanceName;
			szInstanceName.Format(_T("(pid_%ld_%s_eng_1_engtype_%s)"), nPID, itHquery->second.lpInstance, szEngType);
			CString szCounterName = _T("\\Utilization Percentage");
			szFullCounterPath.Format(_T("%s%s%s"), szObjectName, szInstanceName, szCounterName);
			PDH_STATUS status = PdhAddCounter(itHquery->first, szFullCounterPath, NULL, &itHquery->second.hCounter);
			if (status != ERROR_SUCCESS)
			{
				continue;
			}
			PdhCollectQueryData(itHquery->first);
		}
	}
	
}

void GPUutilization::UnInit()
{
	if (m_hCPUQuery != NULL)
	{
		PdhCloseQuery(m_hCPUQuery);
	}
	m_HCounter = 0;

}

CString GPUutilization::GetUtilization()
{
	CString strInfo;
	int usage = -1;
	auto itHquery = m_mapHquery.begin();
	for (; itHquery != m_mapHquery.end(); ++itHquery)
	{
		HQUERY hCPUQuery = (itHquery->first);
		if (hCPUQuery != NULL)
		{
			PdhCollectQueryData(hCPUQuery);

			PDH_FMT_COUNTERVALUE pdhValue;
			DWORD dwValue;
			PDH_STATUS status = PdhGetFormattedCounterValue(itHquery->second.hCounter, PDH_FMT_DOUBLE, &dwValue, &pdhValue);
			if (status == ERROR_SUCCESS)
			{
				usage = floor(pdhValue.doubleValue + 0.5);
				if (usage > 100)
				{
					usage = 100;
				}
				CString strTemp;
				strTemp.Format(_T("%s:%d\n"), itHquery->second.lpInstance, usage);
				strInfo += strTemp;
			}
			PdhCollectQueryData(hCPUQuery);
		}
	}
	

	return strInfo;
}

int GPUutilization::GetCpuNum()
{
	return m_nCPUCount;
}

void GPUutilization::MakeCounterPath()
{
	PDH_STATUS status;

	status = PdhOpenQuery(NULL, NULL, &m_hPathQuery);

	if (status != ERROR_SUCCESS)
		std::cout << "Open Query Error" << std::endl;

	PDH_COUNTER_PATH_ELEMENTS pcpe;
	TCHAR szFullPathBuffer[MAX_PATH] = TEXT("");

	DWORD dwSize = sizeof(szFullPathBuffer);

	pcpe.szMachineName = NULL;//(LPWSTR)_T("À×Éñ±Ê¼Ç±¾");
	pcpe.szObjectName = (LPWSTR)_T("GPU Engine");
	pcpe.szInstanceName = (LPWSTR)_T("*");
	pcpe.szCounterName = (LPWSTR)_T("Utilization Percentage");
	pcpe.dwInstanceIndex = -1;
	pcpe.szParentInstance = NULL;

	status = PdhMakeCounterPath(&pcpe, szFullPathBuffer, &dwSize, 0);

	if (status != ERROR_SUCCESS)
		std::cout << "Make Path Error" << std::endl;

	std::cout << "Path: " << szFullPathBuffer << std::endl;
}

HANDLE  GPUutilization::GetProcessHandle(int nID)
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, nID);
}

unsigned long GPUutilization::GetProcessHandle(LPCTSTR pName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return NULL;
	}
	PROCESSENTRY32 pe = { sizeof(pe) };
	BOOL fOk;
	for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe))
	{
		if (!_tcscmp(pe.szExeFile, pName))
		{
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
	}
	return NULL;
}

void GPUutilization::GetAllProcessID()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	cout << "CreateToolhelp32Snapshot TH32CS_SNAPPROCESS" << endl;
	cout << "cntThreads \t cntUsage \t dwFlags \t pcPriClassBase \t szExeFile \t th32DefaultHeapID \t th32ModuleID \t th32ParentProcessID \t th32ProcessID \t" << endl;
	PROCESSENTRY32 pe32 = { sizeof(pe32) };
	if (Process32First(hSnapshot, &pe32))
	{
		do {
			cout << pe32.cntThreads << "\t";
			cout << pe32.cntUsage << "\t";
			cout << pe32.dwFlags << "\t";
			cout << pe32.pcPriClassBase << "\t";
			_tprintf(_T("%s\t"), pe32.szExeFile);
			cout << pe32.th32DefaultHeapID << "\t";
			cout << pe32.th32ModuleID << "\t";
			cout << pe32.th32ParentProcessID << "\t";
			cout << pe32.th32ProcessID << "\t";
			cout << endl;
			m_mapProcessID[pe32.szExeFile] = pe32.th32ProcessID;
		} while (Process32Next(hSnapshot, &pe32));
	}
	CloseHandle(hSnapshot);
	cout << endl;
}

void GPUutilization::EeumObjectItem(LPCTSTR strObjectName)
{
	PDH_STATUS status = ERROR_SUCCESS;
	LPWSTR pwsCounterListBuffer = NULL;
	DWORD dwCounterListSize = 0;
	LPWSTR pwsInstanceListBuffer = NULL;
	DWORD dwInstanceListSize = 0;
	LPWSTR pTemp = NULL;

	// Determine the required buffer size for the data. 
	status = PdhEnumObjectItems(
		NULL,                   // real-time source
		NULL,                   // local machine
		strObjectName,         // object to enumerate
		pwsCounterListBuffer,   // pass NULL and 0
		&dwCounterListSize,     // to get required buffer size
		pwsInstanceListBuffer,
		&dwInstanceListSize,
		PERF_DETAIL_WIZARD,     // counter detail level
		0);

	if (status == PDH_MORE_DATA)
	{
		// Allocate the buffers and try the call again.
		pwsCounterListBuffer = (LPWSTR)malloc(dwCounterListSize * sizeof(WCHAR));
		pwsInstanceListBuffer = (LPWSTR)malloc(dwInstanceListSize * sizeof(WCHAR));

		if (NULL != pwsCounterListBuffer && NULL != pwsInstanceListBuffer)
		{
			status = PdhEnumObjectItems(
				NULL,                   // real-time source
				NULL,                   // local machine
				strObjectName,         // object to enumerate
				pwsCounterListBuffer,
				&dwCounterListSize,
				pwsInstanceListBuffer,
				&dwInstanceListSize,
				PERF_DETAIL_WIZARD,     // counter detail level
				0);

			if (status == ERROR_SUCCESS)
			{
				wprintf(L"Counters that the Process objects defines:\n\n");

				// Walk the counters list. The list can contain one
				// or more null-terminated strings. The list is terminated
				// using two null-terminator characters.
				for (pTemp = pwsCounterListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1)
				{
					wprintf(L"%s\n", pTemp);
				}

				wprintf(L"\nInstances of the Process object:\n\n");

				// Walk the instance list. The list can contain one
				// or more null-terminated strings. The list is terminated
				// using two null-terminator characters.
				for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1)
				{
					wprintf(L"%s\n", pTemp);
					m_vecInstance.push_back(pTemp);
				}
			}
			else
			{
				wprintf(L"Second PdhEnumObjectItems failed with %0x%x.\n", status);
			}
		}
		else
		{
			wprintf(L"Unable to allocate buffers.\n");
			status = ERROR_OUTOFMEMORY;
		}
	}
	else
	{
		wprintf(L"\nPdhEnumObjectItems failed with 0x%x.\n", status);
	}

	if (pwsCounterListBuffer != NULL)
		free(pwsCounterListBuffer);

	if (pwsInstanceListBuffer != NULL)
		free(pwsInstanceListBuffer);
}
