#pragma once
#include <windows.h>
#include <iostream>
#include "HashTable.h"

using namespace std;

struct ThreadInfo
{
	HANDLE hNamedPipe;
	HashTable* table;
};