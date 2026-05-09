//
// Created by adna on 5/9/26.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "models.h"

typedef struct {
    char name[256];
    int score;
} InspectorScore;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./scorer <district_id>\n");
        return 1;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", argv[1]);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        printf("[%s] Error: Could not open reports.dat\n", argv[1]);
        return 1;
    }

    InspectorScore scores[100];
    int unique_inspectors = 0;
    Report temp_report;

    while (read(fd, &temp_report, sizeof(Report)) == sizeof(Report)) {
        int found = 0;
        for (int i = 0; i < unique_inspectors; i++) {
            if (strcmp(scores[i].name, temp_report.inspector_name) == 0) {
                scores[i].score += temp_report.severity;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(scores[unique_inspectors].name, temp_report.inspector_name);
            scores[unique_inspectors].score = temp_report.severity;
            unique_inspectors++;
        }
    }
    close(fd);

    // the Hub will intercept this using dup2!
    printf("\n--- Workload for District: %s ---\n", argv[1]);
    if (unique_inspectors == 0) {
        printf("No reports found.\n");
    } else {
        for (int i = 0; i < unique_inspectors; i++) {
            printf("  > Inspector '%s': %d severity points\n", scores[i].name, scores[i].score);
        }
    }

    return 0;
}
