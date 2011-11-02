#ifndef __WAIT_QUEUE_H
#define __WAIT_QUEUE_H

#include <list.h>
#include <types.h>
struct task;

struct wait_queue {
	int num;
	struct list_head queue;
};

struct wait_entry {
	void (*func)(struct wait_entry *);
	struct list_head entry;
	struct task *task;
};

#define INIT_WAIT_QUEUE(name) { 0, INIT_LIST_HEAD(name.queue) }
#define WAIT_QUEUE(name) struct wait_queue name = INIT_WAIT_QUEUE(name)

static _inline struct wait_entry *wait_dequeue(struct wait_queue *wq)
{
	struct wait_entry *wait = NULL;
	if (wq->num > 0) {
		wq->num--;
		wait = list_first_entry(&wq->queue, struct wait_entry, entry);
		list_del(&wait->entry);
	}
	return wait;
}

static _inline void wait_enqueue(struct wait_queue *wq, struct wait_entry *wait)
{
	wq->num++;
	list_add(&wait->entry, &wq->queue);
}

extern void sleep_on(struct wait_queue *wq);
extern void wake_up(struct wait_queue *wq);

#endif
