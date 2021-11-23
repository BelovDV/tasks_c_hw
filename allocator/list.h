#include <stdlib.h>
#include <stdio.h>

/*
struct Node_;
struct Node_
{
	struct Node_ *left;
	struct Node_ *right;

	struct Data data;
};
typedef struct Node_ Node;
*/

#ifndef HEADER_LIST
#define HEADER_LIST

// initialize: Node *root = calloc(sizeof(*root) + sizeof(Data))

/**
 * @brief list - root; begin - root->right, end - root
 * @param root - Node*
 */
void list_initialize(void *root);

/**
 * @brief destructor for list
 * @param root - Node*
 * @param deallocate function to deallocate node (free, for example)
 */
void list_destruct(void *root, void (*deallocate)(void *));

/**
 * @brief check if root is list - cycle
 * @param root - Node*
 * @return is cycle - 1, isn't - 0
 */
int list_check_cycle(void *root, size_t max_size);

/**
 * @brief left->right=right, right->left=left
 * @param left - Node*
 * @param right - Node*
 */
void list_link(void *left, void *right);

/**
 * @brief insert new before iter
 * @param iter - Node*
 * @param new - Node*
 */
void list_insert_before(void *iter, void *new);

/**
 * @brief insert new after iter
 * @param iter - Node*
 * @param new - Node*
 */
void list_insert_after(void *iter, void *new);

/**
 * @brief remove links: iter->left, iter->right
 * @param iter - Node*
 * @exception user should free memory
 */
void list_fetch(void *iter);

/**
 * @brief fetch and deallocate
 * @param iter - Node*
 * @param deallocate function to deallocate node (free, for example)
 */
void list_remove(void *iter, void (*deallocate)(void *));

#endif