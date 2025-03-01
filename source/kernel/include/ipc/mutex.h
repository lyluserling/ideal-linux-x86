#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "core/task.h"
#include "tools/list.h"

typedef struct _mutex_t{
    task_t * owner;
    int locked_count;//上锁多少次
    list_t wait_list;
}mutex_t;

void mutex_init(mutex_t * mutex);
void mutex_lock(mutex_t * mutex);
void mutex_unlock(mutex_t *mutex);

#endif