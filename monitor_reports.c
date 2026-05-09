#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
    keep_running = 0;
    const char *msg = "[STOP] SIGINT received. Shutting down...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

void handle_sigusr1(int sig) {
    const char *msg = "[ALERT] A new report has been added!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    FILE *check_f = fopen(".monitor_pid", "r");
    if (check_f) {
        int existing_pid;
        if (fscanf(check_f, "%d", &existing_pid) == 1) {
            // Write the error to stdout so the hub's pipe catches it
            printf("[ERROR] Monitor already running with PID %d\n", existing_pid);
            fclose(check_f);
            return 1;
        }
        fclose(check_f);
    }

    struct sigaction sa_int, sa_usr;

    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    sa_usr.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr.sa_mask);
    sa_usr.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr, NULL);

    FILE *f = fopen(".monitor_pid", "w");
    if (!f) {
        printf("[ERROR] Failed to create .monitor_pid\n");
        return 1;
    }
    fprintf(f, "%d\n", getpid());
    fclose(f);

    printf("[START] Running in background with PID %d.\n", getpid());
    fflush(stdout); 

    while (keep_running) {
        pause(); 
    }

    unlink(".monitor_pid");
    return 0;
}