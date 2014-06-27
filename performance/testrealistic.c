#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#ifdef EGEN
#include "../malloc.h"
#else
#include <malloc.h>
#endif
#include <string.h>
#include "test.c"

struct Node {
	char *name;
	struct Node *next;
} typedef Node;

Node *peopleList = NULL;
int peopleCount = 0;

void addPerson(char *name) {
	Node *node = malloc(sizeof(Node));
	if (!node) return;
	node->name = name;
	node->next = peopleList;
	peopleList = node;
	peopleCount++;
}

void removePerson(char *name) {
	Node *current = peopleList;	
	Node *prev = NULL;
	while (current) {
		if (strcmp(current->name, name)) {
			if (prev) {
				prev->next = current->next;
			} else {
				peopleList = current->next;
			}
			free(current->name);
			free(current);
			peopleCount--;
			break;
		}
		prev = current;
		current = current->next;
	}
}

void removeRandomPeople() {
	Node *current = peopleList;	
	Node *prev = NULL;
	while (current) {
		if (rand() % 100 > 50) {
			if (prev) {
				prev->next = current->next;
			} else {
				peopleList = current->next;
			}
			free(current->name);
			free(current);
			peopleCount--;
			break;
		}
		prev = current;
		current = current->next;
	}
}

int testRealistic() {
	int memoryNeeded = 0;
	int i;
	int j;

	for (j = 0; j < 1000; j++) {
		for (i = 0; i < 1000; i++) {
			int nameLength = (1 + rand() % 200);
			memoryNeeded += nameLength + sizeof(Node);
			char *name = malloc(nameLength);
			if (!name) continue;
			addPerson(name);
			int j;
			for (j = 0; j < nameLength; j++) {
				name[j] = 65 + rand() % 25;
			}
			//printf("%s\n", name);
		}
		removeRandomPeople();
	}
	return memoryNeeded;	
}

int main() {
	test(testRealistic);
}
