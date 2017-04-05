#include <fstream>
#include <Windows.h>
#include "message.h"
using namespace std;

ofstream fileCreateOut(char* fileName);
ifstream fileCreateIn(char* fileName);

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "rus");

	ifstream fin;
	ofstream fout;
	Message message;
	Message* messages;
	int command;
	int messageNum;
	char name[20];
	char text[20];
	bool isFree = false;

	HANDLE hWritedEvent;
	HANDLE hReadedEvent;
	HANDLE hMutex;
	HANDLE hWaited[2];

	hMutex = OpenMutex(SYNCHRONIZE, FALSE, "ReceiverMutex");

	WaitForSingleObject(hMutex, INFINITE);

	printf("%s", "¬ведите своЄ им€(не более 20 символов): ");
	scanf("%s", name);

	ReleaseMutex(hMutex);

	do 
	{
		WaitForSingleObject(hMutex, INFINITE);
		printf("%s", "¬ведите код комманды(0 - выход; 1 - запись): ");
		scanf("%d", &command);
		if (command == 1)
		{
			isFree = false;
			fin = fileCreateIn(argv[1]);
			fin >> messageNum;
			messages = new Message[messageNum];
			for (int i = 0; i < messageNum; i++)
			{
				fin.read((char*)&messages[i], sizeof(Message));
				if (messages[i].free)
				{
					isFree = true;
				}
			}
			fin.close();
			while (!isFree)
			{
				hReadedEvent = CreateEvent(NULL, NULL, FALSE, "ReadedEvent");
				hWaited[0] = hMutex;
				hWaited[1] = hReadedEvent;
				ReleaseMutex(hMutex);
				WaitForMultipleObjects(2, hWaited, TRUE, INFINITE);
				fin = fileCreateIn(argv[1]);
				fin >> messageNum;
				for (int i = 0; i < messageNum; i++)
				{
					fin.read((char*)&messages[i], sizeof(Message));
					if (messages[i].free)
					{
						isFree = true;
					}
				}
				fin.close();
				CloseHandle(hReadedEvent);
			}
			printf("%s: %s", name, "¬ведите текст сообщени€(не более 20 символов): ");
			scanf("%s", text);
			for (int i = 0; i < messageNum; i++)
			{
				if (messages[i].free)
				{
					sprintf(messages[i].name, "%s", name);
					sprintf(messages[i].text, "%s", text);
					messages[i].free = false;

					hWritedEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "WritedEvent");
					SetEvent(hWritedEvent);

					break;
				}
			}
			fout = fileCreateOut(argv[1]);
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

	CloseHandle(hMutex);

	return 0;
}