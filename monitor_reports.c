//
// Created by adna on 4/27/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

// Volatile flag to safely break the loop from the signal handler
volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
    keep_running = 0;
    // write() is async-signal-safe, unlike printf()
    const char *msg = "\n[Monitor] SIGINT received. Shutting down...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

void handle_sigusr1(int sig) {
    const char *msg = "[Monitor] Alert: A new report has been added!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    struct sigaction sa_int, sa_usr;

    // 1. Setup SIGINT (Ctrl+C) handler using sigaction
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    // 2. Setup SIGUSR1 (New Report) handler using sigaction
    sa_usr.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr.sa_mask);
    sa_usr.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr, NULL);

    // 3. Create .monitor_pid file
    FILE *f = fopen(".monitor_pid", "w");
    if (!f) {
        perror("Failed to create .monitor_pid");
        return 1;
    }
    fprintf(f, "%d\n", getpid());
    fclose(f);

    printf("[Monitor] Running in background with PID %d. Waiting for signals...\n", getpid());

    // 4. Wait loop
    while (keep_running) {
        pause(); // Suspends the process until a signal is caught
    }

    // 5. Cleanup
    unlink(".monitor_pid");
    printf("[Monitor] Cleanup complete (.monitor_pid deleted). Exiting.\n");
    return 0;
}
