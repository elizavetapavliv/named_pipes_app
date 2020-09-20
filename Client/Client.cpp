#include <iostream>
#include <string>
#include <windows.h>
#include "TaxPayment.h"
#include "ClientQuary.h"

using namespace std;

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");

	HANDLE hNamedPipe;
	string machineName;
	string pipeName;

	cout << "Имя компьютера с сервером: ";
	getline(cin, machineName);
	pipeName = "\\\\" + machineName + "\\pipe\\demo_pipe";
	hNamedPipe = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	if (hNamedPipe == INVALID_HANDLE_VALUE)
	{
		cout << "err " << GetLastError();
		return GetLastError();
	}

	while (true)
	{
		int choice;
		int command;
		DWORD dwBytesWritten;
		DWORD dwBytesRead;
		ClientQuary cQuary;
		TaxPayment taxPayment;

		cout << "Модфификация записи файла - 1; чтение записи - 2; завершение работы - 3" << endl;
		cin >> choice;
		cin.ignore();
		if (choice == 1)
		{
			cout << "Номер компании: ";
			cin >> cQuary.key;
			cQuary.isReading = false;
			if (!WriteFile(hNamedPipe, &cQuary, sizeof(ClientQuary), &dwBytesWritten, (LPOVERLAPPED)NULL))
				break;
			if (!ReadFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesRead, (LPOVERLAPPED)NULL))
				break;
			if (taxPayment.num != -1)
			{
				cout << "Номер компании: " << taxPayment.num << "; название: " << taxPayment.name
					<< "; сумма платежей: " << taxPayment.sum << endl;
				cout << "Введите записи о компании (<название> <сумма платежей>): " << endl;
				cin >> taxPayment.name >> taxPayment.sum;
				cout << "Введите 1 для отправки записи на сервер: ";
				cin >> command;
				if (command == 1)
					if (!WriteFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesWritten, (LPOVERLAPPED)NULL))
						break;
				cout << "Введите 0 для завершения доступа к записи: ";
				cin >> command;
				if (command == 0)
					if (!WriteFile(hNamedPipe, &command, sizeof(int), &dwBytesWritten, (LPOVERLAPPED)NULL))
						break;
			}
			else
				cout << "Нет записей с ключом " << cQuary.key << endl;
		}
		else if (choice == 2)
		{
			cout << "Номер компании: ";
			cin >> cQuary.key;
			cQuary.isReading = true;
			if (!WriteFile(hNamedPipe, &cQuary, sizeof(ClientQuary), &dwBytesWritten, (LPOVERLAPPED)NULL))
				break;
			if (!ReadFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesRead, (LPOVERLAPPED)NULL))
				break;
			if (taxPayment.num != -1)
			{
				cout << "Номер компании: " << taxPayment.num << "; название: " << taxPayment.name
					<< "; сумма платежей: " << taxPayment.sum << endl;
				cout << "Введите 0 для завершения доступа к записи: " << endl;
				cin >> command;
				if (command == 0)
					if (!WriteFile(hNamedPipe, &command, sizeof(int), &dwBytesWritten, (LPOVERLAPPED)NULL))
						break;
			}
			else
				cout << "Нет записей с ключом " << cQuary.key << endl;
		}
		else
			break;
	}

	CloseHandle(hNamedPipe);

	return 0;
}