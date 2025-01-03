#define  _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#include "list.h"

void init_list(list** l) {
	*l = (list*)malloc(sizeof(list));
	(*l)->head = NULL;
	(*l)->current = NULL;
	(*l)->tail = NULL;
	InitializeCriticalSection(&(*l)->cs);
}

void insert_first_node(node* new_node, list* l) {
	EnterCriticalSection(&l->cs);

	// If the list is empty, set the tail to the new node
	if (l->tail == NULL && l->head == NULL) {
		l->tail = new_node;
		l->head = new_node;
	}
	else {
		new_node->next = l->head;
		l->head = new_node;
	}

	// Set the head to the new node
	LeaveCriticalSection(&l->cs);
}

void insert_last_node(node* new_node, list* l) {
	EnterCriticalSection(&l->cs);

	// If the list is empty, set the head and tail to the new node
	if (l->head == NULL) {
		l->head = new_node;
		l->tail = new_node;
	}
	else {
		// Otherwise, add the new node to the end of the list
		l->tail->next = new_node;
		l->tail = new_node;
	}
	new_node->next = NULL;
	LeaveCriticalSection(&l->cs);

}

void delete_node(node* new_node, list* l) {
	EnterCriticalSection(&l->cs);
	node* current = l->head;
	node* previous = NULL;

	// Search for the node to delete
	while (current != NULL && current != new_node) {
		previous = current;
		current = current->next;
	}

	// If the node was found, delete it
	if (current != NULL) {
		// If the node to delete is the head of the list, update the head pointer
		if (current == l->head) {
			l->head = current->next;
		}
		// If the node to delete is the tail of the list, update the tail pointer
		if (current == l->tail) {
			l->tail = previous;
		}
		// Update the next pointer of the previous node to skip the deleted node
		if (previous != NULL) {
			previous->next = current->next;
		}
	}
	else {
		printf("Node is not in list.\n");
	}
	LeaveCriticalSection(&l->cs);

}

node* delete_first_node(list* l) {
	node* firstNode;
	EnterCriticalSection(&l->cs);

	if (l->head != NULL) {
		if (l->head == l->tail)
			l->tail = NULL;
		firstNode = l->head;
		l->head = l->head->next;
		LeaveCriticalSection(&l->cs);
		return firstNode;
	}
	else {
		printf("List is empty, no element at the beginning!\n");
		LeaveCriticalSection(&l->cs);
		return NULL;
	}

}

void move_first_node(list* to, list* from) {
	EnterCriticalSection(&from->cs);
	EnterCriticalSection(&to->cs);

	node* first_node_from = from->head;
	if (first_node_from != NULL) {
		from->head = from->head->next;
		if (from->head == NULL)
			from->tail = NULL;

		if (to->tail == NULL && to->head == NULL) {
			to->tail = first_node_from;
			to->head = first_node_from;
		}
		else {
			first_node_from->next = to->head;
			to->head = first_node_from;
		}
	}
	LeaveCriticalSection(&to->cs);
	LeaveCriticalSection(&from->cs);
}

node* find_previous_node(list* lst, node* target) {
	node* current = NULL;
	EnterCriticalSection(&lst->cs);
	current = lst->head;
	while (current != NULL) {
		if (current->next == target) {
			LeaveCriticalSection(&lst->cs);
			return current;
		}
		current = current->next;
	}
	LeaveCriticalSection(&lst->cs);
	return NULL;
}

void move_specific_node(list* to, list* from, node* n) {
	EnterCriticalSection(&from->cs);
	EnterCriticalSection(&to->cs);
	node* prev = NULL;
	node* curr = from->head;
	while (curr != NULL && curr != n) {
		prev = curr;
		curr = curr->next;
	}

	prev = curr->next;
	if (to->head == NULL) {
		to->head = curr;
	}
	to->tail = curr;
	curr->next = NULL;

	LeaveCriticalSection(&to->cs);
	LeaveCriticalSection(&from->cs);
}

void print_list(list* l) {
	EnterCriticalSection(&l->cs);
	printf("LIST: \n");
	node* current = l->head;
	while (current != NULL) {
		WCHAR* thread_name_READ = NULL;
		WCHAR* thread_name_WRITE = NULL;
		if (GetThreadDescription(current->thread_read, &thread_name_READ)) {
			printf("[ %ls ] --> ", thread_name_READ);
		}
		else {
			printf("[Unknown Read Thread]");
		}

		current = current->next;
	}
	printf("\n");
	LeaveCriticalSection(&l->cs);
}

void delete_list(list* l) {
	EnterCriticalSection(&l->cs);
	while (l->head != l->tail) {
		node* prev = l->head;
		l->head = l->head->next;
		if (prev->msgStruct != NULL)
			free(prev->msgStruct);
		free(prev);
	}
	l->head = NULL;
	l->tail = NULL;
	LeaveCriticalSection(&l->cs);
	DeleteCriticalSection(&l->cs);
	free(l);
}
