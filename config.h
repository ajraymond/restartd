// restartd - Process checker and/or restarter daemon
// Copyright (C) 2000-2002 Tibor Koleszar <oldw@debian.org>
// Copyright (C) 2006 Aurélien GÉRÔME <ag@roxor.cx>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Tibor Koleszar <oldw@debian.org>
// H-7400 Kaposvar, Somssich Pal str. 4.

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEFAULT_CONFIG "/etc/restartd.conf"
#define VERSION "0.2.1"

int debug;			// Debug
int config_process_number;
int check_interval;
int foreground;
struct config_process_type *config_process;
char *config_file;

typedef struct config_process_type
{

  char name[64];
  char regexp[128];
  char not_running[256];
  char running[256];
  char processes[256];
  char status[32];

} config_process_type;

int read_config ( /* char *config_file */ );

#endif
