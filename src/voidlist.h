typedef struct VoidList
{
	void** entries;
	int capacity;
	int length;
} VoidList;

VoidList* createVoidList(int capacity);
void addVoidList(VoidList* list, void* item);
void removeVoidList(VoidList* list, void* item);
