#include <iostream>
#include <fstream>
#include <Windows.h>
#include "message.h"
using namespace std;

ofstream fileCreateOut(char* fileName);
ifstream fileCreateIn(char* fileName);

int main()
{
	setlocale(LC_ALL, "rus");

	char lpszAppName[] = "Sender.exe";
	char* lpszCommandLine = new char[1000];
	char* fileName = new char[260];
	int messageNum;
	int processNum;
	int command;
	bool isEmpty;
	ifstream fin;
	ofstream fout;
	Message message;
	Message* messages;

	HANDLE hReadedEvent;
	HANDLE hWritedEvent;
	HANDLE hMutex;
	HANDLE hWaited[2];
	STARTUPINFO* si;
	PROCESS_INFORMATION* pi;

	hMutex = CreateMutex(NULL, TRUE, "ReceiverMutex");

	printf("%s", "¬ведите им€ файла: ");
	scanf("%s", fileName);
	printf("%s", "¬ведите количество сообщений: ");
	scanf("%d", &messageNum);

	fout = fileCreateOut(fileName);
	sprintf(lpszCommandLine, "%s %s", lpszAppName, fileName);
	fout << messageNum;
	message.free = true;
	sprintf(message.name, "%s", "");
	sprintf(message.text, "%s", "");
	for (int i = 0; i < messageNum; i++)
		fout.write((char*)&message, sizeof(Message));
	fout.close();

	printf("%s", "¬ведите количесво процессов Sender: ");
	scanf("%d", &processNum);

	si = new STARTUPINFO[processNum];
	pi = new PROCESS_INFORMATION[processNum];
	for (int i = 0; i < processNum; i++)
	{
		ZeroMemory(&si[i], sizeof(STARTUPINFO));
		si[i].cb = sizeof(STARTUPINFO);
		CreateProcess(NULL, lpszCommandLine, NULL, NULL, FALSE, NULL, NULL, NULL, &si[i], &pi[i]);
	}

	ReleaseMutex(hMutex);

	do
	{
		WaitForSingleObject(hMutex, INFINITE);
		printf("%s", "¬ведите код комманды(0 - выход; 1 - чтение): ");
		scanf("%d", &command);
		if (command == 1)
		{
			isEmpty = true;
			fin = fileCreateIn(fileName);
			fin >> messageNum;
			messages = new Message[messageNum];
			for (int i = 0; i < messageNum; i++)
			{
				fin.read((char*)&messages[i], sizeof(Message));
				if (!messages[i].free) {
					isEmpty = false;
				}
			}
			fin.close();
			while (isEmpty)
			{
				hWritedEvent = CreateEvent(NULL, NULL, FALSE, "WritedEvent");
				hWaited[0] = hMutex;
				hWaited[1] = hWritedEvent;
				ReleaseMutex(hMutex);
				WaitForMultipleObjects(2, hWaited, TRUE, INFINITE);
				fin = fileCreateIn(fileName);
				fin >> messageNum;
				for (int i = 0; i < messageNum; i++)
				{
					fin.read((char*)&messages[i], sizeof(Message));
					if (!messages[i].free)
					{
						isEmpty = false;
					}
				}
				fin.close();
				CloseHandle(hWritedEvent);
			}
			for (int i = 0; i < messageNum; i++)
			{
				if (!messages[i].free) {
					printf("Readed message: %s\n", messages[i].text);
					messages[i].free = true;

					hReadedEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "ReadedEvent");
					SetEvent(hReadedEvent);

					break;
				}
			}
			fout = fileCreateOut(fileName);
			fout << messageNum;
			for (int i = 0; i < messageNum; i++)
			{
				fout.write((char*)&messages[i], sizeof(Message));
			}
			fout.close();
		}
		else
		{
			break;
		}
		ReleaseMutex(hMutex);
	} while (command == 1);

	for (int i = 0; i < processNum; i++)
	{
		CloseHandle(pi[i].hThread);
		CloseHandle(pi[i].hProcess);
	}
	CloseHandle(hMutex);

	return 0;
}