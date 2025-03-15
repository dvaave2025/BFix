#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CFG.h"
#include "CodeObject.h"
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include "InstructionDecoder.h"
#include <iostream>
#include "Operand.h"
#include <time.h>
#include "Instruction.h"
#include "Absloc.h"
#include "AbslocInterface.h"
#include <algorithm>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "BPatch_flowGraph.h"
//#include "CFGraph.h"

namespace dp = Dyninst::ParseAPI;
namespace st = Dyninst::SymtabAPI;
using namespace std;
using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

//#define INITIAL_GROUP_SIZE 10
#define INITIAL_NODE_SIZE 10
#define LOAD_FACTOR 0.75

typedef struct Node {
    Dyninst::Address pc;
	dp::Block *bb;
    odata *meta;
	size_t ccc;
    struct Node *next;
} Node;

typedef struct Group {
    Node **nodes;
    size_t size;
    size_t count;
} Group;

typedef struct HashTable {
    Group **groups;
    size_t size;
    size_t count;
} HashTable;

// Hash function to calculate index
size_t cchash(Dyninst::Address pc, size_t table_size) {
    return pc % table_size;
}

// Create a new node
Node* create_node(dp::Block *b,Dyninst::Address pc, odata *meta, size_t standc) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->bb = b;
	new_node->pc = pc;
	new_node->ccc = standc;
	new_node->meta = (odata*)malloc(sizeof(odata));
    memcpy(new_node->meta, meta, sizeof(odata));
    new_node->next = NULL;
    return new_node;
}

// Create a new group
Group* create_group(size_t size) {
    Group *group = (Group *)malloc(sizeof(Group));
    group->nodes = (Node **)calloc(size, sizeof(Node *));
    group->size = size;
    group->count = 0;
    return group;
}

// Create a new hash table
HashTable* create_table(size_t size) {
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    table->groups = (Group **)calloc(size, sizeof(Group *));
    table->size = size;
    table->count = 0;
    return table;
}

// Resize a group when the load factor exceeds the threshold
void resize_group(Group *group) {
    size_t new_size = group->size * 2;
    Node **new_nodes = (Node **)calloc(new_size, sizeof(Node *));
    
    for (size_t i = 0; i < group->size; i++) {
        Node *current = group->nodes[i];
        while (current != NULL) {
            Node *next = current->next;
            size_t new_index = cchash(current->pc, new_size);
            current->next = new_nodes[new_index];
            new_nodes[new_index] = current;
            current = next;
        }
    }
    
    free(group->nodes);
    group->nodes = new_nodes;
    group->size = new_size;
}

// Resize the hash table when the load factor exceeds the threshold
void resize_table(HashTable *table) {
    size_t new_size = table->size * 2;
    Group **new_groups = (Group **)calloc(new_size, sizeof(Group *));
    
    for (size_t i = 0; i < table->size; i++) {
        if (table->groups[i] != NULL) {
            size_t new_index = cchash(table->groups[i]->nodes[0]->pc, new_size);
            new_groups[new_index] = table->groups[i];
        }
    }
    
    free(table->groups);
    table->groups = new_groups;
    table->size = new_size;
}

// Add a node to a specific group
void add_node_to_group(Group *group, Dyninst::Address pc, odata *meta, dp::Block *b, size_t standc) {
    if ((float)group->count / group->size > LOAD_FACTOR) {
        resize_group(group);
    }
    
	size_t index = cchash(pc, group->size);
    Node *new_node = create_node(b, pc, meta, standc);
    new_node->next = group->nodes[index];
    group->nodes[index] = new_node;
    group->count++;
}

// Add a group to the hash table
void add_group(HashTable *table, size_t group_key) {
    if ((float)table->count / table->size > LOAD_FACTOR) {
        resize_table(table);
    }
    
    size_t index = cchash(group_key, table->size);
    //size_t index = group_key;
	if (table->groups[index] == NULL) {
        table->groups[index] = create_group(INITIAL_NODE_SIZE);
        table->count++;
    }
}

// Search for a node within a specific group
Node* search_node_in_group(Group *group, Dyninst::Address pc) {
    size_t index = cchash(pc, group->size);
    Node *current = group->nodes[index];
    while (current != NULL) {
        if (current->pc == pc) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Search for a node in a specific group in the hash table
Node* search_node_in_table(HashTable *table, size_t group_key, Dyninst::Address pc) {
    size_t group_index = cchash(group_key, table->size);
    Group *group = table->groups[group_index];
    if (group == NULL) {
        return NULL;
    }
    return search_node_in_group(group, pc);
}

// Free the memory allocated for a node
void free_node(Node *node) {
    //free(node->key);
    free(node->meta);
    free(node);
}

// Free the memory allocated for a group
void free_group(Group *group) {
    for (size_t i = 0; i < group->size; i++) {
        Node *current = group->nodes[i];
        while (current != NULL) {
            Node *next = current->next;
            free_node(current);
            current = next;
        }
    }
    free(group->nodes);
    free(group);
}

// Free the memory allocated for the hash table
void free_table(HashTable *table) {
    for (size_t i = 0; i < table->size; i++) {
        if (table->groups[i] != NULL) {
            free_group(table->groups[i]);
        }
    }
    free(table->groups);
    free(table);
}

/*int main() {
    HashTable *table = create_table(INITIAL_GROUP_SIZE);

    add_group(table, "group1");
    add_group(table, "group2");

    add_node_to_group(table->groups[hash("group1", table->size)], "node1", "value1");
    add_node_to_group(table->groups[hash("group1", table->size)], "node2", "value2");
    add_node_to_group(table->groups[hash("group2", table->size)], "node3", "value3");

    Node *found_node = search_node_in_table(table, "group1", "node1");
    if (found_node) {
        printf("Found node: key = %s, value = %s\n", found_node->key, found_node->value);
    } else {
        printf("Node not found\n");
    }

    free_table(table);
    return 0;
}*/

