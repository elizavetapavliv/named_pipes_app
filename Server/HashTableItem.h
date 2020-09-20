#pragma once
#include <windows.h>
#include "TaxPayment.h"

struct HashTableItem
{
	TaxPayment taxPayment;
	HANDLE hMutex;
	HANDLE hSemaphore;
	int* readersCount;
	HashTableItem* nextItem;
};