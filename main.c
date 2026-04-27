#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>  // Added for kill()
#include <signal.h>     // Added for kill() and SIGUSR1
#include <time.h>
#include <unistd.h>
#include "models.h"
#include "fs_utils.h"
#include "reports.h"
#include "filter.h"

/* Helper to check if a role has specific access based on the simulation rules.
   mode_flag should be e.g., 4 for read (r), 2 for write (w), 1 for execute (x) */
int check_permission(const char *filepath, const char *role, int need_read, int need_write) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        return 0; // File doesn't exist yet, allow creation logic to handle it
    }

    int can_read = 0, can_write = 0;

    // Managers use the OWNER bits
    if (strcmp(role, "manager") == 0) {
        can_read = (st.st_mode & S_IRUSR) ? 1 : 0;
        can_write = (st.st_mode & S_IWUSR) ? 1 : 0;
    }
    // Inspectors use the GROUP bits
    else if (strcmp(role, "inspector") == 0) {
        can_read = (st.st_mode & S_IRGRP) ? 1 : 0;
        can_write = (st.st_mode & S_IWGRP) ? 1 : 0;
    }

    if (need_read && !can_read) return 0;
    if (need_write && !can_write) return 0;

    return 1;
}

/* Attempts to log the action to logged_district */
void log_action(const char *district, const char *role, const char *user, const char *action) {
    char path[512];
    snprintf(path, sizeof(path), "%s/logged_district", district);

    // Explicitly check if the role is allowed to write to the log (Inspector should fail here)
    if (!check_permission(path, role, 0, 1)) {
        printf("[Log Blocked] Detected restriction: '%s' role cannot write to %s. Refusing to log.\n", role, path);
        return;
    }

    FILE *log = fopen(path, "a");
    if (log) {
        time_t now = time(NULL);
        // Strip the newline from ctime
        char *time_str = ctime(&now);
        time_str[strlen(time_str) - 1] = '\0';

        fprintf(log, "[%s] User: %s | Role: %s | Action: %s\n", time_str, user, role, action);
        fclose(log);
    }
}

/* Helper to log if the monitor was successfully notified or not */
void log_monitor_status(const char *district, const char *role, const char *user, int success) {
    char path[512];
    snprintf(path, sizeof(path), "%s/logged_district", district);
    FILE *log = fopen(path, "a");
    if (log) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strlen(time_str) - 1] = '\0';

        if (success) {
            fprintf(log, "[%s] User: %s | Role: %s | Action: monitor_notified_successfully\n", time_str, user, role);
        } else {
            fprintf(log, "[%s] User: %s | Role: %s | Action: ERROR_monitor_could_not_be_informed\n", time_str, user, role);
        }
        fclose(log);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s --role <manager|inspector> [--user <name>] --<command> <district_id> [args...]\n", argv[0]);
        return 1;
    }

    char *role = NULL;
    char *user = "Unknown";
    char *command = NULL;
    char *district = NULL;
    int cmd_index = -1;

    // 1. Parse Command Line Arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (strncmp(argv[i], "--", 2) == 0 && i + 1 < argc) {
            command = argv[i];
            district = argv[++i];
            cmd_index = i;
            break; // Stop parsing, the rest are arguments for the command
        }
    }

    if (!role || !command || !district) {
        printf("Error: Missing required arguments.\n");
        return 1;
    }

    if (strcmp(role, "manager") != 0 && strcmp(role, "inspector") != 0) {
        printf("Error: Invalid role. Must be 'manager' or 'inspector'.\n");
        return 1;
    }

    // 2. Initialize the district if it doesn't exist yet
    struct stat st;
    if (stat(district, &st) == -1) {
        init_district(district);
    } else {
        // Run the symlink health check every time we interact with an existing district
        check_symlink_health(district);
    }

    // 3. Dispatch the Command
    log_action(district, role, user, command);

    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if (strcmp(command, "--add") == 0) {
        if (!check_permission(path, role, 0, 1)) {
            printf("Error: Role '%s' does not have write access to reports.dat\n", role);
            return 1;
        }

        Report new_report = {0};
        new_report.report_id = (int)time(NULL) % 10000;
        strncpy(new_report.inspector_name, user, MAX_NAME_LEN - 1);
        new_report.latitude = 45.7489;
        new_report.longitude = 21.2087;
        strncpy(new_report.category, "road", MAX_CATEGORY_LEN - 1);
        new_report.severity = 2;
        new_report.timestamp = time(NULL);
        strncpy(new_report.description, "Pothole simulation", MAX_DESC_LEN - 1);

        add_report(district, &new_report);

        // --- PHASE 2: Notify Monitor ---
        FILE *pid_file = fopen(".monitor_pid", "r");
        int monitor_pid = -1;
        int signal_sent = 0;

        if (pid_file) {
            if (fscanf(pid_file, "%d", &monitor_pid) == 1) {
                // Attempt to send SIGUSR1
                if (kill(monitor_pid, SIGUSR1) == 0) {
                    signal_sent = 1;
                    printf("Monitor (PID %d) successfully notified.\n", monitor_pid);
                } else {
                    printf("Error: Failed to send signal to monitor.\n");
                }
            }
            fclose(pid_file);
        } else {
            printf("Notice: .monitor_pid not found. Monitor is not running.\n");
        }

        // Log the result of the notification attempt
        log_monitor_status(district, role, user, signal_sent);
    }
    else if (strcmp(command, "--list") == 0) {
        if (!check_permission(path, role, 1, 0)) {
            printf("Error: Role '%s' cannot read reports.dat\n", role);
            return 1;
        }
        list_reports(district);
    }
    else if (strcmp(command, "--view") == 0 && cmd_index + 1 < argc) {
        int report_id = atoi(argv[cmd_index + 1]);
        view_report(district, report_id);
    }
    else if (strcmp(command, "--remove_report") == 0 && cmd_index + 1 < argc) {
        if (strcmp(role, "manager") != 0) {
            printf("Error: Only managers can remove reports.\n");
            return 1;
        }
        int report_id = atoi(argv[cmd_index + 1]);
        remove_report(district, report_id);
    }
    else if (strcmp(command, "--remove_district") == 0) {
        if (strcmp(role, "manager") != 0) {
            printf("Error: Only managers can remove districts.\n");
            return 1;
        }
        remove_district(district);
    }
    else if (strcmp(command, "--update_threshold") == 0 && cmd_index + 1 < argc) {
        if (strcmp(role, "manager") != 0) {
            printf("Error: Only managers can update thresholds.\n");
            return 1;
        }
        int new_thresh = atoi(argv[cmd_index + 1]);
        update_threshold(district, new_thresh);
    }
    else if (strcmp(command, "--filter") == 0 && cmd_index + 1 < argc) {
        // Collect all remaining arguments as conditions
        int num_conditions = argc - (cmd_index + 1);
        char **conditions = &argv[cmd_index + 1];
        filter_reports(district, conditions, num_conditions);
    }
    else {
        printf("Unknown command or missing arguments for %s.\n", command);
    }

    return 0;
}