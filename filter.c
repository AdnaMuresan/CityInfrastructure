//
// Created by adna on 4/19/26.
//

#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* --- AI GENERATED CODE START --- */
int parse_condition(const char *input, char *field, char *op, char *value) {
    // Find the first colon
    const char *colon1 = strchr(input, ':');
    if (!colon1) return 0; // Parsing failed

    // Find the second colon
    const char *colon2 = strchr(colon1 + 1, ':');
    if (!colon2) return 0; // Parsing failed

    // Extract field
    int field_len = colon1 - input;
    strncpy(field, input, field_len);
    field[field_len] = '\0';

    // Extract operator
    int op_len = colon2 - (colon1 + 1);
    strncpy(op, colon1 + 1, op_len);
    op[op_len] = '\0';

    // Extract value (everything after the second colon)
    strcpy(value, colon2 + 1);

    return 1; // Success
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int sev_val = atoi(value); // Convert string to integer
        if (strcmp(op, "==") == 0) return r->severity == sev_val;
        if (strcmp(op, "!=") == 0) return r->severity != sev_val;
        if (strcmp(op, ">") == 0) return r->severity > sev_val;
        if (strcmp(op, ">=") == 0) return r->severity >= sev_val;
        if (strcmp(op, "<") == 0) return r->severity < sev_val;
        if (strcmp(op, "<=") == 0) return r->severity <= sev_val;
    }
    else if (strcmp(field, "category") == 0) {
        // Strings usually only use == or !=
        if (strcmp(op, "==") == 0) return strcmp(r->category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->category, value) != 0;
    }
    else if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->inspector_name, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->inspector_name, value) != 0;
    }
    else if (strcmp(field, "timestamp") == 0) {
        time_t time_val = (time_t)atol(value); // Convert string to long integer
        if (strcmp(op, "==") == 0) return r->timestamp == time_val;
        if (strcmp(op, "!=") == 0) return r->timestamp != time_val;
        if (strcmp(op, ">") == 0) return r->timestamp > time_val;
        if (strcmp(op, ">=") == 0) return r->timestamp >= time_val;
        if (strcmp(op, "<") == 0) return r->timestamp < time_val;
        if (strcmp(op, "<=") == 0) return r->timestamp <= time_val;
    }
    return 0; // Unknown field or unhandled operator
}
/* --- AI GENERATED CODE END --- */

/* --- HUMAN WRITTEN CODE START --- */
void filter_reports(const char *district_id, char *conditions[], int num_conditions) {
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open reports.dat for filtering");
        return;
    }

    Report r;
    int found_any = 0;

    // Read records one by one
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int all_matched = 1;

        // Test the current record against EVERY condition passed in
        for (int i = 0; i < num_conditions; i++) {
            char field[32], op[8], value[256];

            if (!parse_condition(conditions[i], field, op, value)) {
                printf("Error: Invalid condition format '%s'. Expected field:operator:value\n", conditions[i]);
                all_matched = 0;
                break;
            }

            if (!match_condition(&r, field, op, value)) {
                all_matched = 0; // Record failed this condition
                break;
            }
        }

        // Implicit AND logic: only print if it survived all condition checks
        if (all_matched) {
            printf("ID: %d | Inspector: %s | Category: %s | Severity: %d\n",
                   r.report_id, r.inspector_name, r.category, r.severity);
            found_any = 1;
        }
    }

    if (!found_any) {
        printf("No reports matched the given conditions.\n");
    }

    close(fd);
}
/* --- HUMAN WRITTEN CODE END --- */