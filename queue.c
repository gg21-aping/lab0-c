#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Iterate through a list reversely */
#define list_for_each_entry_safe_reverse(entry, safe, head, member)    \
    for (entry = list_entry((head)->prev, typeof(*entry), member),     \
        safe = list_entry(entry->member.prev, typeof(*entry), member); \
         &entry->member != (head); entry = safe,                       \
        safe = list_entry(safe->member.prev, typeof(*entry), member))

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));

    if (!new)
        return NULL;

    INIT_LIST_HEAD(new);

    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    element_t *e, *next;

    if (!head)
        return;

    list_for_each_entry_safe(e, next, head, list)
        q_release_element(e);

    free(head);

    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *e;

    if (!head)
        return false;

    e = malloc(sizeof(element_t));
    if (!e)
        return false;

    e->value = strdup(s);
    if (!e->value) {
        free(e);
        return false;
    }

    INIT_LIST_HEAD(&e->list);
    list_add(&e->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *e;

    if (!head)
        return false;

    e = malloc(sizeof(element_t));
    if (!e)
        return false;

    e->value = strdup(s);
    if (!e->value) {
        free(e);
        return false;
    }

    INIT_LIST_HEAD(&e->list);
    list_add_tail(&e->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *e;

    if (!head || list_empty(head))
        return NULL;

    e = list_first_entry(head, element_t, list);

    list_del(&e->list);

    if (sp && e->value) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return e;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *e;

    if (!head || list_empty(head))
        return NULL;

    e = list_last_entry(head, element_t, list);

    list_del(&e->list);

    if (sp && e->value) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return e;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    int len = 0;
    struct list_head *iter;

    if (!head)
        return 0;

    list_for_each(iter, head)
        len++;

    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    struct list_head *slow, *fast;
    element_t *e;

    if (!head || list_empty(head))
        return false;
    if (list_is_singular(head))
        return true;

    slow = head->next;
    fast = head->next->next;

    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    list_del(slow);

    e = list_entry(slow, element_t, list);
    q_release_element(e);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    // given sorted queue, remove the duplicated elements

    element_t *curr = NULL, *next = NULL;
    bool should_delete = false;

    if (!head || list_empty(head))
        return false;

    list_for_each_entry_safe(curr, next, head, list) {
        bool duplicate =
            (&next->list != head && !strcmp(curr->value, next->value));

        if (duplicate || should_delete) {
            list_del(&curr->list);
            q_release_element(curr);
            should_delete = duplicate;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *curr, *next;

    if (!head || list_empty(head))
        return;

    list_for_each_safe(curr, next, head)
        list_move(curr, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    struct list_head *start;
    int size, i;

    if (!head || list_empty(head) || list_is_singular(head) || k <= 1)
        return;

    size = q_size(head);
    start = head;

    while (size >= k) {
        struct list_head *end = start->next, *iter = start->next;

        for (i = 0; i < k; i++) {
            list_move(iter, start);
            iter = end->next;
        }

        start = end;
        size -= k;
    }
}

static inline bool is_descend(char *a, char *b, bool descend)
{
    int cmp = strcmp(a, b);
    return ((cmp < 0) && !descend) || ((cmp > 0 && descend));
}

static void q_merge_two(struct list_head *list1,
                        struct list_head *list2,
                        bool descend)
{
    struct list_head *iter;

    if (list_empty(list1)) {
        list_splice_init(list2, list1);
        return;
    } else if (list_empty(list2))
        return;

    iter = list1->next;

    while (iter != list1 && !list_empty(list2)) {
        element_t *e1 = list_entry(iter, element_t, list);
        element_t *e2 = list_first_entry(list2, element_t, list);

        if (!is_descend(e1->value, e2->value, descend)) {
            /* e2 is greater than e1 */
            list_move_tail(&e2->list, iter);
        } else {
            /* e1 is greater than e2 */
            iter = iter->next;
        }
    }

    if (!list_empty(list2))
        list_splice_tail_init(list2, list1);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    struct list_head *slow, *fast;
    LIST_HEAD(temp);

    if (!head || list_empty(head) || list_is_singular(head))
        return;

    slow = head->next;
    fast = head->next->next;

    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    /* split list to half */
    list_cut_position(&temp, head, slow);

    q_sort(&temp, descend);
    q_sort(head, descend);

    q_merge_two(head, &temp, descend);
}

/**
 * q_make_monotonic() - Remove nodes to make queue monotonic
 * @head: header of queue
 * @descend: whether to filter for descending (true) or ascending (false)
 *
 * Return: the number of elements in queue after performing operation
 */
static int q_make_monotonic(struct list_head *head, bool descend)
{
    element_t *curr, *next;
    char *extreme = NULL;

    if (!head || list_empty(head) || list_is_singular(head))
        return q_size(head);

    list_for_each_entry_safe_reverse(curr, next, head, list)
    {
        if (!extreme) {
            extreme = curr->value;
            continue;
        }
        if (is_descend(extreme, curr->value, descend)) {
            list_del(&curr->list);
            q_release_element(curr);
        } else {
            extreme = curr->value;
        }
    }

    return q_size(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_make_monotonic(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_make_monotonic(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
