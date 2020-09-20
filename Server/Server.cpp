#include <string>
#include <iomanip>
#include "ClientQuary.h"
#include "ThreadInfo.h"

using namespace std;

DWORD WINAPI ClientThread(LPVOID);

int main()
{
	setlocale(LC_ALL, "Russian");

	string binaryFileName;
	HashTable* table;
	TaxPayment* taxPayments;
	HANDLE* hMutexes;
	HANDLE* hSemaphores;
	TaxPayment tp;
	int* readersCounts;
	int clientsNum = 0;
	int entriesNum = 0;

	cout << "Имя бинарного файла: ";
	getline(cin, binaryFileName);
	cout << "Количество записей в файле: ";
	cin >> entriesNum;
	cout << "Введите записи о компании (<номер> <название> <сумма платежей>): " << endl;

	table = new HashTable(entriesNum, binaryFileName);
	taxPayments = new TaxPayment[entriesNum];
	hMutexes = new HANDLE[entriesNum];
	hSemaphores = new HANDLE[entriesNum];
	readersCounts = new int[entriesNum];

	for (int i = 0; i < entriesNum; i++)
	{
		cin >> taxPayments[i].num >> taxPayments[i].name >> taxPayments[i].sum;
		readersCounts[i] = 0;
		hMutexes[i] = CreateMutex(NULL, FALSE, NULL);
		if (hMutexes[i] == NULL)
			return GetLastError();
		hSemaphores[i] = CreateSemaphore(NULL, 1, 1, NULL);
		if (hSemaphores[i] == NULL)
			return GetLastError();
		table->AddItem(taxPayments[i], hSemaphores[i], hMutexes[i], &readersCounts[i], i);
	}

	ifstream file(binaryFileName, ios::binary);

	cout << "Номер" << setw(10) << "Название" << setw(15) << "Сумма платежей" << endl;
	while (file.read((char*)&tp, sizeof(TaxPayment)))
		cout << tp.num << setw(10) << tp.name << setw(15) << tp.sum << endl;
	file.close();
	
	ThreadInfo tInfo;
	HANDLE* hThreads;

	cout << "Количество клиентов: ";
	cin >> clientsNum;
	hThreads = new HANDLE[clientsNum];
	tInfo.table = table;

	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	HANDLE hNamedPipe = nullptr;

	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.lpSecurityDescriptor = &sd;
	for (int i = 0; i < clientsNum; i++)
	{
		hNamedPipe = CreateNamedPipe("\\\\.\\pipe\\demo_pipe", PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0, INFINITE, &sa);
		if (hNamedPipe == INVALID_HANDLE_VALUE)
		{
			cout << "error " << GetLastError()<<endl;
			return GetLastError();
		}
		if (!ConnectNamedPipe(hNamedPipe, (LPOVERLAPPED)NULL))
		{
			CloseHandle(hNamedPipe);
			return GetLastError();
		}

		DWORD IDThread;

		tInfo.hNamedPipe = hNamedPipe;
		hThreads[i] = CreateThread(NULL, 0, ClientThread, (LPVOID)&tInfo, 0, &IDThread);
		if (hThreads == NULL)
			return GetLastError();
	}

	WaitForMultipleObjects(clientsNum, hThreads, TRUE, INFINITE);

	file.open(binaryFileName, ios::binary);
	cout << "Номер" << setw(10) << "Название" << setw(15) << "Сумма платежей" << endl;
	while (file.read((char*)&tp, sizeof(TaxPayment)))
		cout << tp.num << setw(10) << tp.name << setw(15) << tp.sum << endl;
	file.close();

	char command;

	cout << "Введите любой символ для завершения работы сервера: ";
	cin >> command;

	for (int i = 0; i < clientsNum; i++)
	{
		CloseHandle(hMutexes[i]);
		CloseHandle(hSemaphores[i]);
		CloseHandle(hThreads[i]);
	}

	delete[]table;
	delete[]taxPayments;
	delete[]hSemaphores;
	delete[]hMutexes;
	delete[]readersCounts;
	delete[]hThreads;

	return 0;
}