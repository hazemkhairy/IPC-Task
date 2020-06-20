#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <vector>
#include <filesystem>
#include <direct.h>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
vector <PROCESS_INFORMATION>processHandles;
char path[1000] = {};
int maxlength = 1000;
void addLogger()
{
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD dwProcessId = 0;
	DWORD dwThreadId = 0;
	ZeroMemory(&si, sizeof(si));
	si.dwFlags |= DETACHED_PROCESS;
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	BOOL bCreateProcess = NULL;
	
	
	string newPath = path;

	newPath += "\\..\\Debug\\logger.exe";
	wstring x = wstring(newPath.begin(), newPath.end());
	
	bCreateProcess = CreateProcess(
		x.c_str(),
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi
	);
	if (bCreateProcess == FALSE)
	{
		cout << "Create Process Failed & Error NO - " << GetLastError() << endl;
	}
	else
	{

		cout << "Created Logger Successfully\n";
		cout << "LOGGER ID -> " << pi.dwProcessId << endl;
		processHandles.push_back(pi);
	}

}

void addSlave()
{
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD dwProcessId = 0;
	DWORD dwThreadId = 0;
	ZeroMemory(&si, sizeof(si));
	si.dwFlags |= DETACHED_PROCESS;
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	BOOL bCreateProcess = NULL;

	string newPath = path;

	newPath += "\\..\\Debug\\slave.exe";
	wstring x = wstring(newPath.begin(), newPath.end());
	bCreateProcess = CreateProcessW(
		x.c_str(),
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	);
	if (bCreateProcess == FALSE)
	{
		cout << "Create Process Failed & Error NO - " << GetLastError() << endl;
	}
	else
	{

		cout << "Created NEW Slave Successfull\n";
		cout << "SLAVE ID -> " << pi.dwProcessId << endl;
		processHandles.push_back(pi);
	}

}
void deleteProcess()
{
	cout << "Enter process ID to kill  : ";
	int x;
	cin >> x;
	for (int i = 0; i < processHandles.size(); i++)
	{

		if (WORD(x) == processHandles[i].dwProcessId)
		{
			TerminateProcess(processHandles[i].hProcess, 0);
			processHandles[i] = processHandles[processHandles.size() - 1];
			processHandles.pop_back();
			cout << "KILLED\n";
			break;
		}
	}
}


int main(int argc, const char** argv)
{
	_getcwd(path, maxlength);
	
	addLogger();
	while (true)
	{
		cout << "enter 1 to add new process\n";
		cout << "enter 2 to delete process\n";

		int x;
		cin >> x;
		if (x == 1)
		{
			addSlave();

		}
		else if (x == 2)
		{
			deleteProcess();

		}
	}
	system("PAUSE");
}