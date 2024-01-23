#include <windows.h>

void test_list();
void test_hashtable();
void test_dynamic_enqueue_dequeue();
DWORD WINAPI producer(LPVOID param);
DWORD WINAPI consumer(LPVOID param);
