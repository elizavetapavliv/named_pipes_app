#pragma once
#include <iostream>
#include <fstream>
#include "HashTableItem.h"
#include "TaxPayment.h"

using namespace std;

class HashTable
{
	int size;
	HashTableItem** table;
	fstream file;
	string fileName;
	int HashFunction(int id);
public:
	HashTable(int size, string fileName);
	~HashTable();
	void AddItem(TaxPayment tp, HANDLE hSemaphore, HANDLE hMutex, int* readersCount, int i);
	TaxPayment* GetItem(int id);
	void GetHandles(int id, HANDLE& hSemaphore, HANDLE& hMutex, int*& readersCount);
	void ModifyItem(TaxPayment tp);
};