/**
 * savvy_scheduler
 * CS 341 - Fall 2023
 * Collaborated with yifan20
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;
    int arrival;
    int priority;
    double start_time;
    double running_time;
    double remaining_time;
    double rr_time;
} job_info;

size_t count;
double total_waiting;
double total_turnaround;
double total_response;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    job* ja = (job*)a;
    job* jb = (job*)b;
    job_info* ia = ja->metadata;
    job_info* ib = jb->metadata;
    if (ia->arrival == ib->arrival) {
        return 0;
    }
    return (ia->arrival < ib->arrival ? -1 : 1);
    
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    job* ja = (job*)a;
    job* jb = (job*)b;
    job_info* ia = ja->metadata;
    job_info* ib = jb->metadata;
    if (ia->priority == ib->priority) {
        return break_tie(a, b);
    }
    return (ia->priority < ib->priority ? -1 : 1);
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    job* ja = (job*)a;
    job* jb = (job*)b;
    job_info* ia = ja->metadata;
    job_info* ib = jb->metadata;
    if (ia->remaining_time == ib->remaining_time) {
        return break_tie(a, b);
    }
    return (ia->remaining_time < ib->remaining_time ? -1 : 1);
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    job* ja = (job*)a;
    job* jb = (job*)b;
    job_info* ia = ja->metadata;
    job_info* ib = jb->metadata;
    if (ia->rr_time == ib->rr_time) {
        return break_tie(a, b);
    }
    return (ia->rr_time < ib->rr_time ? -1 : 1);
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    job* ja = (job*)a;
    job* jb = (job*)b;
    job_info* ia = ja->metadata;
    job_info* ib = jb->metadata;
    if (ia->running_time == ib->running_time) {
        return break_tie(a, b);
    }
    return (ia->running_time < ib->running_time ? -1 : 1);
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    job_info* temp = calloc(1,sizeof(job_info));
    temp->id = job_number;
    temp->arrival = time;
    temp->priority = sched_data->priority;
    temp->running_time = sched_data->running_time;
    temp->remaining_time = sched_data->running_time;
    temp->rr_time = -1;
    // temp->start = -1;
    temp->start_time = -1;
    newjob->metadata = temp;
    priqueue_offer(&pqueue, newjob);
    count++;
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    if (job_evicted == NULL) {
        return priqueue_poll(&pqueue);
    }
    job_info* j_info = job_evicted->metadata;
    
    j_info->remaining_time -= time - j_info->start_time;
    j_info->rr_time = time;
    if (j_info->start_time == -1) {
        //set start time
        j_info->start_time = time - 1;
    }
    if (pqueue_scheme == PPRI || pqueue_scheme == PSRTF) {
        priqueue_offer(&pqueue, job_evicted);
        job_evicted = priqueue_poll(&pqueue);
    }
    else if (pqueue_scheme == RR) {
        priqueue_offer(&pqueue, job_evicted);
        job_evicted = priqueue_poll(&pqueue);
    }
    return job_evicted;
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
    job_info *temp = job_done->metadata;
    total_turnaround += time - temp->arrival;
    total_waiting += time - temp->arrival - temp->running_time;
    total_response += temp->start_time - temp->arrival;
    free(temp);
    // priqueue_poll(&pqueue);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    return total_waiting / count;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    return total_turnaround / count;
}

double scheduler_average_response_time() {
    return total_response / count;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
