/*
 * simarm.c
 *
 * Disk arm scheduling simulation.
 *
 * Algorithms implemented:
 * 1. FCFS  - First Come, First Served
 * 2. SSTF  - Shortest Seek Time First
 * 3. SCAN  - Elevator algorithm, moving upward first
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define MIN_CYLINDER 0
#define MAX_CYLINDER 999

long cylinder_distance(int a, int b) {
    long diff = (long)a - (long)b;
    return diff < 0 ? -diff : diff;
}

int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

/* First Come, First Served */
long fcfs(const int requests[], int count, int start) {
    long total = 0;
    int current = start;

    for (int i = 0; i < count; i++) {
        total += cylinder_distance(current, requests[i]);
        current = requests[i];
    }

    return total;
}

/* Shortest Seek Time First */
long sstf(const int requests[], int count, int start) {
    if (count <= 0) {
        return 0;
    }

    long total = 0;
    int current = start;
    int *visited = calloc(count, sizeof(int));

    if (visited == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    for (int serviced = 0; serviced < count; serviced++) {
        int best_index = -1;
        long best_distance = LONG_MAX;

        for (int i = 0; i < count; i++) {
            if (!visited[i]) {
                long d = cylinder_distance(current, requests[i]);

                if (best_index == -1 ||
                    d < best_distance ||
                    (d == best_distance && requests[i] < requests[best_index])) {
                    best_distance = d;
                    best_index = i;
                }
            }
        }

        total += best_distance;
        current = requests[best_index];
        visited[best_index] = 1;
    }

    free(visited);
    return total;
}

/* SCAN, moving upward first */
long scan(const int requests[], int count, int start) {
    if (count <= 0) {
        return 0;
    }

    long total = 0;
    int current = start;
    int *sorted = malloc(count * sizeof(int));

    if (sorted == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    for (int i = 0; i < count; i++) {
        sorted[i] = requests[i];
    }

    qsort(sorted, count, sizeof(int), compare_ints);

    int split = 0;

    while (split < count && sorted[split] < start) {
        split++;
    }

    for (int i = split; i < count; i++) {
        total += cylinder_distance(current, sorted[i]);
        current = sorted[i];
    }

    if (split > 0) {
        total += cylinder_distance(current, MAX_CYLINDER);
        current = MAX_CYLINDER;

        for (int i = split - 1; i >= 0; i--) {
            total += cylinder_distance(current, sorted[i]);
            current = sorted[i];
        }
    }

    free(sorted);
    return total;
}

int parse_length(const char *arg) {
    char *endptr;
    long value = strtol(arg, &endptr, 10);

    if (*endptr != '\0' || value <= 0 || value > INT_MAX) {
        fprintf(stderr, "Usage: simarm <sequence_length>\n");
        exit(1);
    }

    return (int)value;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: simarm <sequence_length>\n");
        return 1;
    }

    int sequence_length = parse_length(argv[1]);
    int *sequence = malloc(sequence_length * sizeof(int));

    if (sequence == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    srand((unsigned int)time(NULL));

    for (int i = 0; i < sequence_length; i++) {
        sequence[i] = rand() % (MAX_CYLINDER + 1);
    }

    int start = sequence[0];
    int *requests = sequence + 1;
    int request_count = sequence_length - 1;

    printf("Disk Arm Scheduling Simulation\n");
    printf("------------------------------\n");
    printf("Cylinder range: %d - %d\n", MIN_CYLINDER, MAX_CYLINDER);
    printf("Sequence length: %d\n", sequence_length);
    printf("Initial arm position: %d\n\n", start);

    printf("FCFS total distance: %ld cylinders\n",
           fcfs(requests, request_count, start));

    printf("SSTF total distance: %ld cylinders\n",
           sstf(requests, request_count, start));

    printf("SCAN total distance: %ld cylinders\n",
           scan(requests, request_count, start));

    free(sequence);
    return 0;
}