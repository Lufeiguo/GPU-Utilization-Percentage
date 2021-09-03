// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "GPUutilization.h"
#include <windows.h> 
#include <atlstr.h> 


GPUutilization			m_GPUutilization(_T("wmplayer.exe"));//需要监控的进程
VOID   CALLBACK   TimerProc(HWND   hwnd, UINT   uMsg, UINT   idEvent, DWORD   dwTime);
VOID   CALLBACK   TimerProc(HWND   hwnd, UINT   uMsg, UINT   idEvent, DWORD   dwTime)
{
	CString ncpu = m_GPUutilization.GetUtilization();
	std::wcout << ncpu.GetString() << std::endl;
}
int main()
{
	//m_GPUutilization.EeumObjectItem();
	//m_GPUutilization.GetAllProcessID();
	//m_GPUutilization.GetProcessHandle(_T("2345RTProtect.exe"));
	//return 0;
	std::cout << "init......" << std::endl;
	m_GPUutilization.Init();
	
	int   timer1 = 1;
	HWND   hwndTimer;
	MSG   msg;

	SetTimer(NULL, timer1, 10000, TimerProc);
	int   itemp;
	while ((itemp = GetMessage(&msg, NULL, NULL, NULL)) && (itemp != 0) && (-1 != itemp))
	{
		if (msg.message == WM_TIMER)
		{
			std::cout << "i   get   the   message " << std::endl;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	m_GPUutilization.UnInit();
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
