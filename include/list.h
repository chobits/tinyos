#ifndef __LIST_H
#define __LIST_H

#include <compile.h>
#include <types.h>

#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define list_first_entry(ptr, type, member) list_entry((ptr)->next, type, member)

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

#define LIST_HEAD(name) struct list_head name = INIT_LIST_HEAD(name)
#define INIT_LIST_HEAD(head) { &(head), &(head) }
#define HOLE_LIST ((struct list_head *)0xffffffff)
#define list_empty(l) ((l)->next == (l))

static _inline void list_init(struct list_head *head)
{
	head->prev = head->next = head;
}

static _inline void __list_add(struct list_head *list, struct list_head *prev,
							struct list_head *next)
{
	prev->next = list;
	list->prev = prev;
	list->next = next;
	next->prev = list;
}

static _inline void list_add(struct list_head *list, struct list_head *head)
{
	__list_add(list, head, head->next);
}

static _inline void list_add_tail(struct list_head *list, struct list_head *head)
{
	__list_add(list, head->prev, head);
}

static _inline void __list_del(struct list_head *prev, struct list_head *next)
{
	prev->next = next;
	next->prev = prev;
}

static _inline void list_del(struct list_head *list)
{
	__list_del(list->prev, list->next);
	list->prev = list->next = HOLE_LIST;
}

static _inline void list_del_init(struct list_head *list)
{
	__list_del(list->prev, list->next);
	list->prev = list->next = list;
}

static _inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del(list->prev, list->next);
	__list_add(list, head, head->next);
}

/* merge result: dprev <-> (shead <-> ... <-> stail) <-> dnext */
static _inline void __list_merge(struct list_head *dprev, struct list_head *shead,
				struct list_head *stail, struct list_head *dnext)
{
	dprev->next = shead;
	shead->prev= dprev;
	stail->next = dnext;
	dnext->prev = stail;
}

static _inline void list_merge(struct list_head *dest, struct list_head *src)
{
	__list_merge(dest, src->next, src->prev, dest->next);
}

static _inline void list_merge_tail(struct list_head *dest, struct list_head *src)
{
	__list_merge(dest->prev, src->next, src->prev, dest);
}

#define list_for_each(head, list)\
	for (list = (head)->next; list != (head); list = list->next)

#define list_for_each_entry(entry, head, member) \
	for (entry = list_entry((head)->next, typeof(*entry), member);\
				&(entry->member) != (head);\
		entry = list_entry(entry->member.next, typeof(*entry), member))

#define list_for_each_entry_safe(entry, head, member, n) \
	for (entry = list_entry((head)->next, typeof(*entry), member),\
		n = entry->member.next;\
		&entry->member != (head);\
		entry = list_entry(n, typeof(*entry), member),\
		n = entry->member.next)

/*
 * Hash list
 */
/* hash list head */
struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next;
	struct hlist_node **pprev;
};

static _inline int hlist_unhashed(struct hlist_node *node)
{
	return !node->pprev;
}

static _inline int hlist_empty(struct hlist_head *head)
{
	return !head->first;
}

static _inline void hlist_head_init(struct hlist_head *head)
{
	head->first = NULL;
}

static _inline void hlist_node_init(struct hlist_node *node)
{
	node->next = NULL;
	node->pprev = NULL;
}

static _inline void __hlist_del(struct hlist_node *n)
{
	*n->pprev = n->next;
	if (n->next)
		n->next->pprev = n->pprev;
}

static _inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = NULL;
	n->pprev = NULL;
}

static _inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	n->next = h->first;
	n->pprev = &h->first;
	if (h->first)
		h->first->pprev = &n->next;
	h->first = n;
}

/* add @n before @next */
static _inline void hlist_add_before(struct hlist_node *n, struct hlist_node *next)
{
	n->next = next;
	n->pprev = next->pprev;
	*next->pprev = n;
	next->pprev = &n->next;
}

/* add @next after @n */
static _inline void hlist_add_after(struct hlist_node *n, struct hlist_node *next)
{
	next->next = n->next;
	next->pprev = &n->next;
	if (n->next)
		n->next->pprev = &next->next;
	n->next = next;
}

#define hlist_entry(ptr, type, member) list_entry(ptr, type, member)
/* non-node implementation */
#define hlist_for_each_entry2(entry, head, member)\
	for (entry = ((head)->first) ? hlist_entry((head)->first, typeof(*entry), member) : NULL;\
		entry;\
		entry = (entry->member.next) ? hlist_entry(entry->member.next, typeof(*entry), member) : NULL)

#define hlist_for_each_entry(entry, node, head, member)\
	for (node = (head)->first;\
		node && (entry = hlist_entry(node, typeof(*entry), member));\
		node = node->next)

#endif	/* list.h */
