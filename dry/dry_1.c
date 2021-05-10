#include <stdbool.h>
#include <stdlib.h>

typedef struct node_t
{
  int x;
  struct node_t *next;
} * Node;

typedef enum
{
  SUCCESS = 0,
  MEMORY_ERROR,
  EMPTY_LIST,
  UNSORTED_LIST,
  NULL_ARGUMENT,
} ErrorCode;

int getListLength(Node list);
bool isListSorted(Node list);
Node mergeSortedLists(Node list1, Node list2, ErrorCode *error_code);

/**
 * Get the Smaller value from both nodes and advance the respective node
 * to the next one in the linked list
 * 
 * @param list1 - pointer to Node of first list
 * @param list2 - pointer to Node of second list
 * @return 
 *    The bigger value of both nodes
 */
int getSmallerAndMoveFurther(Node *list1, Node *list2);

/**
 * frees all nodes in the linked list
 * 
 * @param list - first Node to be freed
 */
void free_list(Node list);

/**
 * Macro for handling memory allocation errors
 */
#define RETURN_MEMORY_ERROR() \
  *error_code = MEMORY_ERROR; \
  return NULL

/*
 * Macro for the allocation of a new node in a linked list
 */
#define NEW_NODE(list, top)                           \
  (list)->next = (Node)malloc(sizeof(struct node_t)); \
  if (NULL == (list)->next) {                         \
      free_list(list);                                \
      RETURN_MEMORY_ERROR();                          \
    }                                                 \
  (list) = (list)->next;                              \
  (list)->next = NULL                               



/**
 * Macro for creating a copy of a linked list
 */
#define COPY_LIST(src, dest, top)  \
  while (NULL != (src)) {          \
    NEW_NODE((dest), (top));       \
    (dest)->x = (src)->x;          \
    (src) = (src)->next;           \
  }

Node mergeSortedLists(Node list1, Node list2, ErrorCode *error_code)
{

  // error_code is null, we can't set it
  if (NULL == error_code) {
    return NULL;
  }

  // verify valid lists
  if (NULL == list1 || NULL == list2) {
    *error_code = NULL_ARGUMENT;
    return NULL;
  }

  if (!(isListSorted(list1) && isListSorted(list2))) {
    *error_code = UNSORTED_LIST;
    return NULL;
  }

  Node new_list = (Node)malloc(sizeof(struct node_t));

  if (NULL == new_list) {
    RETURN_MEMORY_ERROR();
  }

  Node head1 = list1, head2 = list2, head_new = new_list;

  head_new->x = getSmallerAndMoveFurther(&head1, &head2);
  while ((NULL != head1) && (NULL != head2)) {
    NEW_NODE(head_new, new_list);
    head_new->x = getSmallerAndMoveFurther(&head1, &head2);
  }

  COPY_LIST(head1, head_new, new_list);
  COPY_LIST(head2, head_new, new_list);
  
  *error_code = SUCCESS;
  return new_list; 
}

int getSmallerAndMoveFurther(Node *list1, Node *list2)
{
  int value;

  if ((*list1)->x <= (*list2)->x) {
    value = (*list1)->x;
    *list1 = (*list1)->next;
  } else {
    value = (*list2)->x;
    *list2 = (*list2)->next;
  }

  return value;
}

void free_list(Node list)
{
  Node next;

  while (NULL != list) {
    next = list->next;
    free(list);
    list = next;
  }
}

// ####################################################################
#include <stdio.h>

void print_list(Node l)
{
  while (NULL != l) {
    printf("%d ", l->x);
    l = l->next;
  }
  printf("\n");
}

bool isListSorted(Node list)
{
  int prev = list->x;
  list = list->next;

  while (NULL != list) {
    if (list->x < prev) return false;
    prev = list->x;
    list = list->next;
  }

  return true;
}

int main()
{
  Node l1 = (Node)malloc(sizeof(struct node_t)), h1 = l1;
  Node l2 = (Node)malloc(sizeof(struct node_t)), h2 = l2;
  int a1[] = {1, 4, 9}, a2[] = {2, 4, 8};

  h1->x = a1[0];
  h2->x = a2[0];
  ErrorCode e, *error_code = &e;

  for (int i = 1; i < 3; i++) {
    NEW_NODE(h1, l1);
    h1->x = a1[i];
    NEW_NODE(h2, l2);
    h2->x = a2[i];
  }

  print_list(l1);
  print_list(l2);

  Node merged = mergeSortedLists(l1, l2, error_code);
  print_list(merged);
  return 0;
}