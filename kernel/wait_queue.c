#include <wait_queue.h>
#include <task.h>

/*
 * The default wait queue is FIFO queue.
 * The last-in wait entry is always waken up firstly.
 */
void wake_up(struct wait_queue *wq)
{
	struct wait_entry *wait = wait_dequeue(wq);
	if (!wait)
		return;
	wait->func(wait);
}

static void default_wake_up(struct wait_entry *wait)
{
	wait->task->state = TASK_RUNNABLE;
}

void sleep_on(struct wait_queue *wq)
{
	struct wait_entry wait;
	/* init wait */
	wait.func = &default_wake_up;
	wait.task = ctask;
	/* add to wait queue */
	wait_enqueue(wq, &wait);
	ctask->state = TASK_SLEEP;
	schedule();
}

