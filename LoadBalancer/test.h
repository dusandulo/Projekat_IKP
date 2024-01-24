#include <windows.h>

void test_list();
void test_hashtable();
void test_messages();
DWORD WINAPI add_msg_to_queue(LPVOID param);
DWORD WINAPI dequeue_message(LPVOID param);
