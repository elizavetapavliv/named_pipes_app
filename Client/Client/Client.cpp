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

	cout << "��� ���������� � ��������: ";
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

		cout << "������������ ������ ����� - 1; ������ ������ - 2; ���������� ������ - 3" << endl;
		cin >> choice;
		cin.ignore();
		if (choice == 1)
		{
			cout << "����� ��������: ";
			cin >> cQuary.key;
			cQuary.isReading = false;
			if (!WriteFile(hNamedPipe, &cQuary, sizeof(ClientQuary), &dwBytesWritten, (LPOVERLAPPED)NULL))
				break;
			if (!ReadFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesRead, (LPOVERLAPPED)NULL))
				break;
			if (taxPayment.num != -1)
			{
				cout << "����� ��������: " << taxPayment.num << "; ��������: " << taxPayment.name
					<< "; ����� ��������: " << taxPayment.sum << endl;
				cout << "������� ������ � �������� (<��������> <����� ��������>): " << endl;
				cin >> taxPayment.name >> taxPayment.sum;
				cout << "������� 1 ��� �������� ������ �� ������: ";
				cin >> command;
				if (command == 1)
					if (!WriteFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesWritten, (LPOVERLAPPED)NULL))
						break;
				cout << "������� 0 ��� ���������� ������� � ������: ";
				cin >> command;
				if (command == 0)
					if (!WriteFile(hNamedPipe, &command, sizeof(int), &dwBytesWritten, (LPOVERLAPPED)NULL))
						break;
			}
			else
				cout << "��� ������� � ������ " << cQuary.key << endl;
		}
		else if (choice == 2)
		{
			cout << "����� ��������: ";
			cin >> cQuary.key;
			cQuary.isReading = true;
			if (!WriteFile(hNamedPipe, &cQuary, sizeof(ClientQuary), &dwBytesWritten, (LPOVERLAPPED)NULL))
				break;
			if (!ReadFile(hNamedPipe, &taxPayment, sizeof(TaxPayment), &dwBytesRead, (LPOVERLAPPED)NULL))
				break;
			if (taxPayment.num != -1)
			{
				cout << "����� ��������: " << taxPayment.num << "; ��������: " << taxPayment.name
					<< "; ����� ��������: " << taxPayment.sum << endl;
				cout << "������� 0 ��� ���������� ������� � ������: " << endl;
				cin >> command;
				if (command == 0)
					if (!WriteFile(hNamedPipe, &command, sizeof(int), &dwBytesWritten, (LPOVERLAPPED)NULL))
						break;
			}
			else
				cout << "��� ������� � ������ " << cQuary.key << endl;
		}
		else
			break;
	}

	CloseHandle(hNamedPipe);

	return 0;
}