#include "HashTable.h"
#include "ThreadInfo.h"
#include "ClientQuary.h"

DWORD WINAPI ClientThread(LPVOID threadInfo)
{
	ThreadInfo* tInfo = (ThreadInfo*)threadInfo;
	HANDLE hNamedPipe = tInfo->hNamedPipe;
	HashTable* table = tInfo->table;
	HANDLE hMutex;
	HANDLE hSemaphore;
	int* readersCount = nullptr;

	while (true)
	{
		DWORD dwBytesRead;
		DWORD dwBytesWritten;
		TaxPayment taxPayment;
		ClientQuary cQuary;
		int finishKey;

		if (!ReadFile(hNamedPipe, &cQuary, sizeof(ClientQuary), &dwBytesRead, (LPOVERLAPPED)NULL))
			break;
		table->GetHandles(cQuary.key, hSemaphore, hMutex, readersCount);
		if (readersCount == nullptr)
		{
			taxPayment.num = -1;
			if (!WriteFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesWritten, (LPOVERLAPPED)NULL))
				break;
		}
		else
		{
			if (cQuary.isReading)
			{
				WaitForSingleObject(hMutex, INFINITE);
				(*readersCount)++;
				if ((*readersCount) == 1)
					WaitForSingleObject(hSemaphore, INFINITE);
				ReleaseMutex(hMutex);

				taxPayment = *(table->GetItem(cQuary.key));
				if (!WriteFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesWritten, (LPOVERLAPPED)NULL))
					break;
				if (!ReadFile(hNamedPipe, &finishKey, sizeof(int), &dwBytesRead, (LPOVERLAPPED)NULL))
					break;

				WaitForSingleObject(hMutex, INFINITE);
				(*readersCount)--;
				if ((*readersCount) == 0)
					ReleaseSemaphore(hSemaphore, 1, NULL);
				ReleaseMutex(hMutex);
			}
			else
			{
				WaitForSingleObject(hSemaphore, INFINITE);
				if (table->GetItem(cQuary.key) != nullptr)
					taxPayment = *(table->GetItem(cQuary.key));
				else
					taxPayment.num = -1;
				if (!WriteFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesWritten, (LPOVERLAPPED)NULL))
					break;

				if (!ReadFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesRead, (LPOVERLAPPED)NULL))
					break;
				table->ModifyItem(taxPayment);
				if (!ReadFile(hNamedPipe, &finishKey, sizeof(int), &dwBytesRead, (LPOVERLAPPED)NULL))
					break;
				ReleaseSemaphore(hSemaphore, 1, NULL);
			}
		}
	}

	DisconnectNamedPipe(hNamedPipe);
	CloseHandle(hNamedPipe);

	return 0;
}