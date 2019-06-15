#pragma once

#include "kernel/libk/alloc.h"
#include "kernel/libk/log.h"
#include "kernel/libk/messaging.h"

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} Registers;

typedef struct Task {
    Registers regs;
    char *name;
    MessagePort port;
    struct Task *next;
} Task;

static Task *runningTask;
static Task tasks[20];

void task_empty() {
    while (true) {
        yield();
    }
}

void initTasking() {
    // Get EFLAGS and CR3
    memset(tasks, '\0', sizeof(tasks));
    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(tasks[0].regs.cr3)::"%eax");
    asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(tasks[0].regs.eflags)::"%eax");

    createTask(&tasks[1], task_empty, tasks[0].regs.eflags, (uint32_t*)tasks[0].regs.cr3);
    tasks[0].next = &tasks[1];
    tasks[1].next = &tasks[0];

    tasks[0].name = "init";
    tasks[1].name = "yielder";

    runningTask = &tasks[0];
}

void createTask(Task *task, void (*main)(), uint32_t flags, uint32_t *pagedir) {
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (uint32_t) main;
    task->regs.cr3 = (uint32_t) pagedir;
    task->regs.esp = (uint32_t) kmalloc_forever(1024); // Not implemented here
    task->next = 0;
}

void spawnTask(void (*main)(), char *name) {
    char log[120];
    sprintf(log, "Spawning new task %s", name);
    klog(log);

    Task *t;
    Task *l;

    for (int i = 0; i < sizeof(tasks) / sizeof(Task); i++) {
        if (tasks[i].next == &tasks[0]) {
            t = &tasks[i + 1];
            l = &tasks[i];
            break;
        }
    }

    createTask(t, main, tasks[0].regs.eflags, (uint32_t*)tasks[0].regs.cr3);

    l->next = t;
    t->next = &tasks[0];
    t->name = name;
}

void killTask(Task *t) {
    for (int i = 0; i < sizeof(tasks) / sizeof(Task); i++) {
        if (t == &tasks[i]) {
            tasks[i-1].next = tasks[i].next;
            tasks[i].name = NULL;
            return;
        }
    }
}

void exitTask() {
    killTask(runningTask);
}

Task *findTaskByName(char *name) {
    for (int i = 0; i < sizeof(tasks) / sizeof(Task); i++) {
        Task *t = &tasks[i];

        if (strcmp(name, t->name) == 0) {
            return t;
        }
    }

    return NULL;
}

void sendMessageToTask(char *name, char *message) {
    Task *t = findTaskByName(name);
    if (t == NULL || t->name == NULL) return;
    Message m;
    memset(&m, '\0', sizeof(Message));
    m.message = message;
    message_put(&t->port, &m);
    message_get_response(&m);
}

void yield() {
    Task *last = runningTask;
    runningTask = runningTask->next;
    switchTask(&last->regs, &runningTask->regs);
}
