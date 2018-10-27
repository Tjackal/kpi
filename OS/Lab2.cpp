#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "psapi.h"

int				NumProcessors;
DWORDLONG		TotalPhysMem;
SYSTEM_INFO		sysInfo;
int				status = 1;

typedef struct	ProcessInfo
{
	DWORD			Id;
	HANDLE			Handle;
	char			AppName[MAX_PATH];
	char			UserName[MAX_PATH];
	char			Status[MAX_PATH];
	ULARGE_INTEGER	PrevTime, PrevKernelTime, PrevUserTime;
	double			TimeUsage, MemUsage;
}				ProcessInfo;

void			GetAppName(ProcessInfo *Process)
{
	char	AppName[MAX_PATH] = "unknown";
	HMODULE hMod;
	DWORD	cbNeeded;
	int		j = 0;

	if (!EnumProcessModules(Process->Handle, &hMod, sizeof(hMod), &cbNeeded))
		return;
	GetModuleBaseName(Process->Handle, hMod, (LPTSTR)AppName, sizeof(AppName));
	for (int i = 0; AppName[i]; i += 2, j++)
		Process->AppName[j] = AppName[i];
	Process->AppName[j] = '\0';
}

void			GetUserName_(ProcessInfo *Process)
{
	HANDLE			hToken;
	DWORD			size;
	PTOKEN_USER		pTokenUser;
	char			UserName[MAX_PATH];
	char			DomainName[MAX_PATH];
	SID_NAME_USE	SidType;
	DWORD			NameSize;
	int				j = 0;

	strcpy_s(Process->UserName, "unknown");
	if (!OpenProcessToken(Process->Handle, TOKEN_QUERY, &hToken))
		return ;
	GetTokenInformation(Process->Handle, TokenUser, NULL, 0, &size);
	if (!(pTokenUser = (PTOKEN_USER)malloc(size)))
		return ;
	if (!GetTokenInformation(hToken, TokenUser, pTokenUser, size, &size))
		return ;
	NameSize = MAX_PATH;
	if (!LookupAccountSid(NULL, pTokenUser->User.Sid, (LPWSTR)UserName,
		&NameSize, (LPWSTR)DomainName, &NameSize, &SidType))
		return ;
	for (int i = 0; UserName[i]; i += 2, j++)
		Process->UserName[j] = UserName[i];
	Process->UserName[j] = '\0';
	free(pTokenUser);
	CloseHandle(hToken);
}

void			GetProcessStatus(ProcessInfo *Process)
{
	DWORD	ExitCode;
	char	s[MAX_PATH];

	if (!GetExitCodeProcess(Process->Handle, &ExitCode))
		return ;
	if (ExitCode == STILL_ACTIVE)
		strcpy_s(Process->Status, "ACTIVE");
	else
	{
		strcpy_s(Process->Status, "FINISHED: ");
		_itoa_s((int)ExitCode, s, 10);
		strcpy_s(Process->Status, s);
	}
}

void			GetProcessTime(ProcessInfo *Process)
{
	FILETIME				Time;
	FILETIME				KernelTime;
	FILETIME				UserTime;
	ULARGE_INTEGER			Now, Kernel, User;
	double					percent;

	if (!GetProcessTimes(Process->Handle, &Time, &Time, &KernelTime, &UserTime))
		return ;
	memcpy(&User, &UserTime, sizeof(FILETIME));
	memcpy(&Kernel, &KernelTime, sizeof(FILETIME));
	GetSystemTimeAsFileTime(&Time);
	memcpy(&Now, &Time, sizeof(FILETIME));
	percent = (Kernel.QuadPart - Process->PrevKernelTime.QuadPart) +
	(User.QuadPart - Process->PrevUserTime.QuadPart);
	percent /= (double)(Now.QuadPart - Process->PrevTime.QuadPart);
	percent /= (double)NumProcessors;
	percent *= 100;
	Process->PrevTime = Now;
	Process->PrevUserTime = User;
	Process->PrevKernelTime = Kernel;
	Process->TimeUsage = percent;
}

void			GetProcessMem(ProcessInfo *Process)
{
	PROCESS_MEMORY_COUNTERS	MemCounter;
	double					percent;

	if (!GetProcessMemoryInfo(Process->Handle, &MemCounter, sizeof(MemCounter)))
		return ;
	percent = MemCounter.WorkingSetSize;
	percent /= (double)TotalPhysMem;
	percent *= 100;
	Process->MemUsage = percent;
}

void			GetProcessInfo(ProcessInfo *Process)
{
	Process->Handle = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
		Process->Id);
	if (!Process->Handle)
		return ;
	GetAppName(Process);
	GetUserName_(Process);
	GetProcessStatus(Process);
	GetProcessTime(Process);
	GetProcessMem(Process);
	CloseHandle(Process->Handle);
}

void			PrintProcessInfo(ProcessInfo *Process)
{
	if (!Process->Handle)
		return ;
	//printf("%30s\t%s\t%s\t%f\t%f\n", Process->AppName, Process->UserName, Process->Status, Process->TimeUsage, Process->MemUsage);
	printf("App:    %s\n", Process->AppName);
	printf("User:   %s\n", Process->UserName);
	printf("Status: %s\n", Process->Status);
	printf("Time:   %f%%\n", Process->TimeUsage);
	printf("Mem:    %f%%\n", Process->MemUsage);
	printf("____________________________\n\n");
}

void			PrintMemInfo(void)
{
	MEMORYSTATUSEX memStatus;

	memStatus.dwLength = sizeof(memStatus);
	GlobalMemoryStatusEx(&memStatus);
	printf("Memory information:	   \n\n");
	printf("Total physical memory: %u MB\n", memStatus.ullTotalPhys / 1024 / 1024);
	printf("Free physical memory:  %u MB\n", memStatus.ullAvailPhys / 1024 / 1024);
	printf("Memory usage:          %u %%\n\n", memStatus.dwMemoryLoad);
	printf("Total page file size:  %u MB\n", memStatus.ullTotalPageFile / 1024 / 1024);
	printf("Free page file size:   %u MB\n", memStatus.ullAvailPageFile / 1024 / 1024);
	printf("Page file usage:       %lf %%\n\n", (1 - ((double)memStatus.ullAvailPageFile) / memStatus.ullTotalPageFile) * 100);
	printf("Total virtual memory:  %u MB\n", memStatus.ullTotalVirtual / 1024 / 1024);
	printf("Free virtual memory:   %u MB\n", memStatus.ullAvailVirtual / 1024 / 1024);
	printf("Virtual memory usage:  %lf %%\n\n", (1 - ((double)memStatus.ullAvailVirtual) / memStatus.ullTotalVirtual) * 100);
}

//void			InitProcessTime(ProcessInfo *Process)
//{
//	SYSTEM_INFO	sysInfo;
//	FILETIME	Time, KernelTime, UserTime;
//
//	GetSystemInfo(&sysInfo);
//	GetSystemTimeAsFileTime(&Time);
//	memcpy(&Process->PrevTime, &Time, sizeof(FILETIME));
//	GetProcessTimes(Process->Handle, &Time, &Time, &KernelTime, &UserTime);
//	memcpy(&Process->PrevKernelTime, &KernelTime, sizeof(FILETIME));
//	memcpy(&Process->PrevUserTime, &UserTime, sizeof(FILETIME));
//}

void			Init(void)
{
	SYSTEM_INFO		sysInfo;
	MEMORYSTATUSEX	memInfo;

	GetSystemInfo(&sysInfo);
	NumProcessors = sysInfo.dwNumberOfProcessors;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	TotalPhysMem = memInfo.ullTotalPhys;
}

int				main(void)
{
	ProcessInfo	Processes[1024];
	DWORD		ProcessesID[1024];
	DWORD		NprocInBytes;
	DWORD		NProcesses;

	Init();
	while (status)
	{
		system("cls");
		if (!EnumProcesses(ProcessesID, sizeof(ProcessesID), &NprocInBytes))
			return (-1);
		NProcesses = NprocInBytes / sizeof(DWORD);
		for (int i = 0; i < NProcesses; i++)
		{
			Processes[i].Id = ProcessesID[i];
			//	InitProcessTime(&Processes[i]);
			GetProcessInfo(&Processes[i]);
		}
		//printf("%30s\t%s\t%s\t%f\t%f\n", "App:", "User:", "Status:", "Time:", "Mem:");
		for (int i = 0; i < NProcesses; i++)
			PrintProcessInfo(&Processes[i]);
		PrintMemInfo();
		Sleep(7000);
	}
	return (0);
}
