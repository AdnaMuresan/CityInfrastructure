//
// Created by adna on 4/19/26.
//

#ifndef CITYINFRASTRUCTURE_REPORTS_H // clion
#define CITYINFRASTRUCTURE_REPORTS_H // clion

#ifndef REPORTS_H
#define REPORTS_H

#include "models.h"

/* Appends a new report to the end of reports.dat */
void add_report(const char *district_id, Report *new_report);

/* Lists all reports and displays file metadata (size, permissions, mtime) */
void list_reports(const char *district_id);

/* Prints the full details of a specific report by its ID */
void view_report(const char *district_id, int report_id);

/* Manager only: Removes a report by shifting subsequent records and truncating */
void remove_report(const char *district_id, int report_id);

/* Manager only: Updates the district.cfg file if permissions are exactly 640 */
void update_threshold(const char *district_id, int new_threshold);

#endif /* REPORTS_H */

#endif //CITYINFRASTRUCTURE_REPORTS_H // clion
