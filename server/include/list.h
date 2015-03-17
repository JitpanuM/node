/** A generic linked list
 *
 * Linked lists a used for several data structures in the code.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2015, Institute for Automation of Complex Power Systems, EONERC
 * @file
 */

#ifndef _LIST_H_
#define _LIST_H_

#include <pthread.h>

/* Forward declarations */
struct list_elm;

/** Static list initialization */
#define LIST_INIT { \
	.head = NULL, \
	.tail = NULL, \
	.count = 0, \
	.lock = PTHREAD_MUTEX_INITIALIZER \
}

#define FOREACH(list, elm) \
	for ( struct list_elm *elm = (list)->head, ; \
		elm != (list)->tail; elm  = elm->next )

struct list {
	struct list_elm *head, tail;
	int count;

	pthread_mutex_lock_t lock;
};

struct list_elm {
	void *data;

	struct list_elm *prev, next;
};

void list_init(struct list *l);

void list_destroy(struct list *l);

void list_push(struct list *l, void *d);

struct list_elm * list_search(struct list *l, int (*cmp)(void *));

#endif /* _LIST_H_ */