#include "voidlist.h"
#include <stdbool.h>
#include <stdlib.h>

VoidList* createVoidList(int capacity)
{
	VoidList* ret;

	ret = (VoidList*)malloc(sizeof(VoidList));
	ret->entries = (void**)malloc(sizeof(void*) * capacity);
	ret->capacity = capacity;
	ret->length = 0;

	return ret;
}

void addVoidList(VoidList* list, void* item)
{
	if(list->length >= list->capacity)
		return;

	list->entries[list->length] = item;
	list->length++;
}

void removeVoidList(VoidList* list, void* item)
{
	int index;
	bool found;

	found = false;

	for(index = 0; index < list->length; index++)
		if(list->entries[index] == item)
		{
			found = true;
			break;
		}

	if(!found)
		return;

	list->entries[index] = list->entries[list->length-1];
	list->length--;
}
