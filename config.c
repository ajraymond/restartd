/* restartd - Process checker and/or restarter daemon
 * Copyright (C) 2000-2002 Tibor Koleszar <oldw@debian.org>
 * Copyright (C) 2006 Aurélien GÉRÔME <ag@roxor.cx>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Tibor Koleszar <oldw@debian.org>
 * H-7400 Kaposvar, Somssich Pal str. 4.
 */

#include <syslog.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"

#define MAX_LINE_LENGTH 1024

int read_config(void)
{
    FILE *config_fd;
    char *line1, *line2;
    int i, j, sep, lines;

    if (debug) {
        fprintf(stderr, "Config file: %s\n", config_file);
    }

    /* This is a reload */
    if (config_process_number) {
        for(i=0; i<config_process_number; i++) {
            config_process[i].name[0] = 0;
            config_process[i].regexp[0] = 0;
            config_process[i].running[0] = 0;
            config_process[i].not_running[0] = 0;
        }
    }

    config_process_number = 0;

    line1 = (char *) malloc(MAX_LINE_LENGTH);
    if (!line1)
      oom_failure();
    line2 = (char *) malloc(MAX_LINE_LENGTH);
    if (!line2)
      oom_failure();

    if ((config_fd = fopen(config_file, "rt")) == NULL) {
        fprintf(stderr, "Error at opening config file: %s\n", config_file);
        syslog(LOG_ERR, "Error at opening config file: %s", config_file);
        _exit(0);
    }

    while(fgets(line1, MAX_LINE_LENGTH, config_fd) != NULL) {
        j = 0;

        for(i=0; i<strlen(line1); i++) {
            if (line1[i] == '\n' || line1[i] == '\r' || line1[i] == '\t'
                || line1[i] == ' ') {
                if (line2[j - 1] != ' ' && j > 0) {
                    line2[j] = ' ';
                    j++;
                }
            } else {
                line2[j] = line1[i];
                j++;
            }
        }

        line2[j] = 0;

        if (line2[0] != '#' && strlen(line2) > 0) {
            j = 0;
            i = 0;
            lines = 0;
            sep = 0;

            while(j < strlen(line2)) {
                if (line2[j] == ' ' && sep != 1) {
                    line1[i] = 0;

                    if (debug) {
                        fprintf(stderr, "%u: %s\n", lines, line1);
                    }

                    i = 0;
                    sep = 0;

                    if (lines == 0) {
                        snprintf(config_process[config_process_number].name,
                                 64, "%s", line1);
                    }
                    if (lines == 1) {
                        snprintf(config_process[config_process_number].regexp,
                                 128, "%s", line1);
                    }
                    if (lines == 2) {
                        snprintf(config_process[config_process_number].not_running,
                                 256, "%s", line1);
                    }
                    if (lines == 3) {
                        snprintf(config_process[config_process_number].running,
                                 256, "%s", line1);
                    }

                    lines++;
                }

                if (line2[j] == '"') {
                    sep++;
                }

                if ((line2[j] != ' ' || sep == 1) && line2[j] != '"') {
                    line1[i] = line2[j];
                    i++;
                }

                j++;
            }

            config_process_number++;
        }

    }

    fclose(config_fd);

    free(line1);
    free(line2);

    if (!config_process_number) {
        fprintf(stderr, "No processes defined in config file. Exiting.\n");
        syslog(LOG_INFO, "No processes defined in config file. Exiting.");
        exit(0);
    }

    return 0;
}

void dump_config(void) {
    int i;

    for(i=0; i<config_process_number; i++) {
        printf("ID=%d\n  name=%s\n  regexp=%s\n  running=%s\n  not_running=%s\n", i,
            config_process[i].name,
            config_process[i].regexp,
            config_process[i].running,
            config_process[i].not_running);
    }

}
