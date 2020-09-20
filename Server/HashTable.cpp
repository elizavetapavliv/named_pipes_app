#define _CRT_SECURE_NO_WARNINGS
#include "HashTable.h"

HashTable::HashTable(int size, string fileName)
{
	file.open(fileName, fstream::binary | fstream::out);
	this->fileName = fileName;
	this->size = size;
	table = new HashTableItem*[size];
	for (int i = 0; i < size; i++)
		table[i] = nullptr;
}

HashTable::~HashTable()
{
	for (int i = 0; i < size; i++)
	{
		if (table[i] != nullptr)
		{
			HashTableItem *prevItem = nullptr;
			HashTableItem *item = table[i];
			while (item != nullptr)
			{
				prevItem = item;
				item = item->nextItem;
				delete prevItem;
			}
		}
	}
	delete[]table;
}

void HashTable::AddItem(TaxPayment tp, HANDLE hSemaphore, HANDLE hMutex, int* readersCount, int i)
{
	int ind = HashFunction(tp.num);
	if (table[ind] == nullptr)
	{
		table[ind] = new HashTableItem();
		table[ind]->taxPayment = tp;
		table[ind]->hSemaphore = hSemaphore;
		table[ind]->hMutex = hMutex;
		table[ind]->readersCount = readersCount;
		table[ind]->nextItem = nullptr;
	}
	else
	{
		HashTableItem* item = new HashTableItem();
		item->taxPayment = tp;
		item->nextItem = table[ind]->nextItem;
		table[ind]->nextItem = item;
	}
	file.write((char*)&tp, sizeof(TaxPayment));
	if (i == size - 1)
		file.close();
}

TaxPayment* HashTable::GetItem(int id)
{
	int ind = HashFunction(id);
	HashTableItem* item = table[ind];
	while (item != nullptr && item->taxPayment.num != id)
		item = item->nextItem;
	if (item != nullptr)
		return &(item->taxPayment);
	else
		return nullptr;
}

void HashTable::ModifyItem(TaxPayment taxPayment)
{
	int ind = HashFunction(taxPayment.num);
	HashTableItem* item = table[ind];

	while (item != nullptr && item->taxPayment.num != taxPayment.num)
		item = item->nextItem;
	if (item != nullptr)
	{
		strcpy(item->taxPayment.name, taxPayment.name);
		item->taxPayment.sum = taxPayment.sum;

		TaxPayment tp;
		int pos = 0;

		file.open(fileName, fstream::binary | fstream::out | fstream::in);
		while (file.read((char*)&tp, sizeof(tp)))
		{
			if (tp.num == taxPayment.num)
			{
				file.seekp(pos, ios::beg);
				file.write((char*)&taxPayment, sizeof(taxPayment));
				break;
			}
			pos += sizeof(TaxPayment);
		}
		file.close();
	}
}

void HashTable::GetHandles(int id, HANDLE& hSemaphore, HANDLE& hMutex, int*& readersCount)
{
	int ind = HashFunction(id);
	HashTableItem* item = table[ind];
	while (item != nullptr && item->taxPayment.num != id)
		item = item->nextItem;
	if (item != nullptr)
	{
		hSemaphore = item->hSemaphore;
		hMutex = item->hMutex;
		readersCount = item->readersCount;
	}
	else
		readersCount = nullptr;
}

int HashTable::HashFunction(int id)
{
	return id % size;
}