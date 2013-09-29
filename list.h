#ifndef _LIST_H
#define _LIST_H

struct o_list_head {
    void * data;
    struct o_list_head * next;
};

typedef struct o_list_head o_list_t;

struct list_head {
    struct list_head * next;
    struct list_head * prev;
};

typedef struct list_head list_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

static inline void __list_add(struct list_head * node,
    struct list_head * prev,
    struct list_head * next)
{
    next->prev = node;
    node->next = next;
    node->prev = prev;
    prev->next = node;
}

static inline void list_add(struct list_head * node, struct list_head * head)
{
    __list_add(node, head, head->next);
}

static inline void list_add_tail(struct list_head * node, struct list_head * head)
{
    __list_add(node, head->prev, head);
}

static inline void __list_del(struct list_head * prev,
                  struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head * entry)
{
    __list_del(entry->prev, entry->next);
}

static inline int list_empty(struct list_head * head)
{
    return head->next == head;
}

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define list_for_eachp(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#endif
