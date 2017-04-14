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

	hMutex = OpenMutex(SYNCHRONIZE, TRUE, "ReceiverMutex");

	printf("%s", "¬ведите своЄ им€(не более 20 символов): ");
	scanf("%s", name);

	while (true)
	{
		printf("%s: %s", name, "¬ведите код комманды(0 - выход; 1 - запись): ");
		scanf("%d", &command);
		WaitForSingleObject(hMutex, INFINITE);
		if (command == 1)
		{
			printf("%s: %s", name, "¬ведите текст сообщени€(не более 20 символов): ");
			scanf("%s", text);
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
			if (!isFree)
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
				}
				fin.close();
				CloseHandle(hReadedEvent);
			}
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
	}

	CloseHandle(hMutex);

	return 0;
}