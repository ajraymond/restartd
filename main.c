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

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <time.h>
#include <syslog.h>
#include <signal.h>
#include <stdarg.h>

#include "config.h"

// SIGTERM & SIGHUP handler
void
got_signal (int sig)
{

  if (sig == SIGTERM)
    {
      syslog (LOG_INFO, "Got SIGTERM. Exiting.");
      unlink ("/var/run/restartd.pid");
      exit (0);
    }
  if (sig == SIGHUP)
    {
      syslog (LOG_INFO, "Got SIGHUP. Reloading config file.");
      read_config ();
      syslog (LOG_INFO,
	      "Config file has been read. Found %d process to check.",
	      config_process_number);
    }

}


int
main (int argc, char *argv[])
{

  DIR *procdir_id;
  struct dirent *procdir_dirent;
  int proc_cmdline;
  char *proc_cmdline_str;
  ssize_t proc_cmdline_str_length;
  char *proc_cmdline_name;
  int i, f;
  FILE *out_proc;
  regex_t *regc;
  size_t r_nmatch = 0;
  regmatch_t r_match[256];
  pid_t child;
  time_t now;

  debug = 0;
  check_interval = 20;
  foreground = 0;

  // Options

  config_file = strdup (DEFAULT_CONFIG);

  for (i = 0; i < argc; i++)
    {

      if (!strcmp (argv[i], "-c") || !strcmp (argv[i], "--config"))
	config_file = strdup (argv[i + 1]);

      if (!strcmp (argv[i], "-v") || !strcmp (argv[i], "--version"))
	{
	  printf
	    ("restard %s - Copyright 2000-2002 Tibor Koleszar <oldw@debian.org>\n"
	     "                Copyright 2006 Aurélien GÉRÔME <ag@roxor.cx>\n",
	     VERSION);
	  exit (0);
	}

      if (!strcmp (argv[i], "-d") || !strcmp (argv[i], "--debug"))
	debug = 1;

      if (!strcmp (argv[i], "-f") || !strcmp (argv[i], "--foreground"))
	foreground = 1;

      if (!strcmp (argv[i], "-i") || !strcmp (argv[i], "--interval"))
	{
	  if (atoi (argv[i + 1]) > 0)
	    {
	      check_interval = atoi (argv[i + 1]);
	    }
	  else
	    {
	      fprintf (stderr,
		       "Error at parameter -i: %s is not an interval number\n.",
		       argv[i + 1]);
	      exit (0);
	    }
	}

      if (!strcmp (argv[i], "-h") || !strcmp (argv[i], "--help"))
	{
	  printf ("restartd %s - process restarter and checker daemon\n"
		  "Copyright 2000-2002 Tibor Koleszar <oldw@debian.org>\n"
		  "Copyright 2006 Aurélien GÉRÔME <ag@roxor.cx>\n"
		  "restartd is GPL'ed software.\n"
		  "Parameters:\n"
		  "  -c <config_file> : config file (default is /etc/restartd.conf)\n"
		  "  -d               : set debug on (poor)\n"
		  "  -f               : run in foreground\n"
		  "  -i <interval_sec>: the check interval in second\n"
		  "  -h               : help\n\n", VERSION);
	}

    }

  config_process = malloc (sizeof (struct config_process_type) * 128);

  read_config ();

  syslog (LOG_INFO, "Config file has been read. Found %d process to check.",
	  config_process_number);

  procdir_dirent = malloc (sizeof (struct dirent));
  proc_cmdline_str = (char *) malloc (1024);
  proc_cmdline_name = (char *) malloc (1024);
  regc = malloc (1024);

  // Catch signals
  signal (SIGTERM, got_signal);
  signal (SIGHUP, got_signal);

  // Fork
  if (foreground == 1 || !(child = fork ()))
    {

      // generic daemon stuff from http://www.aarnet.edu.au/~gdt/articles/2003-09-nowhere-daemon/daemon.html
      // added by cwarden@postica.com - 041216
      if (!foreground)
	{
	  if (setsid () == (pid_t) (-1))
	    {
	      fprintf (stderr, "setsid() failed\n");
	      syslog (LOG_ERR, "setsid() failed");
	      return -1;
	    }
	}
      if ((chdir ("/")) < 0)
	{
	  syslog (LOG_ERR, "Cannot chdir to /");
	  return -1;
	}
      f = open ("/dev/null", O_RDONLY);
      if (f == -1)
	{
	  syslog (LOG_ERR, "Failed to open /dev/null");
	  return -1;
	}
      else
	{
	  if (dup2 (f, fileno (stdin)) == -1)
	    {
	      syslog (LOG_ERR, "Failed to set stdin to /dev/null");
	      return -1;
	    }
	  (void) close (f);
	}

      f = open ("/dev/null", O_WRONLY);
      if (f == -1)
	{
	  syslog (LOG_ERR, "Failed to open /dev/null");
	  return -1;
	}
      else
	{
	  if (dup2 (f, fileno (stdout)) == -1)
	    {
	      syslog (LOG_ERR, "Failed to set stdout to /dev/null");
	      return -1;
	    }
	  if (dup2 (f, fileno (stderr)) == -1)
	    {
	      syslog (LOG_ERR, "Failed to set stderr to /dev/null");
	      return -1;
	    }
	  (void) close (f);
	}

      out_proc = fopen ("/var/run/restartd.pid", "wt");
      fprintf (out_proc, "%d", getpid ());
      fclose (out_proc);

      while (1)
	{

	  if ((procdir_id = opendir ("/proc")) == NULL)
	    {
	      syslog (LOG_ERR, "Cannot open /proc dir");
	      return -1;
	    }

	  for (i = 0; i < config_process_number; i++)
	    config_process[i].processes[0] = 0;

	  while ((procdir_dirent = readdir (procdir_id)) != NULL)

	    if (atoi (procdir_dirent->d_name) > 0)
	      {

		sprintf (proc_cmdline_name, "/proc/%s/cmdline",
			 procdir_dirent->d_name);
		proc_cmdline = open (proc_cmdline_name, O_RDONLY);

		proc_cmdline_str_length =
		  read (proc_cmdline, proc_cmdline_str, 1024);

		for (i = 0; i < proc_cmdline_str_length; i++)
		  if (!proc_cmdline_str[i])
		    proc_cmdline_str[i] = 32;

		proc_cmdline_str[proc_cmdline_str_length] = 0;

		// Check process
		for (i = 0; i < config_process_number; i++)
		  {
		    regcomp (regc, config_process[i].regexp, REG_EXTENDED
			     || REG_ICASE);
		    if (!regexec
			(regc, proc_cmdline_str, r_nmatch, r_match, 0))
		      sprintf (config_process[i].processes, "%s %s",
			       config_process[i].processes,
			       procdir_dirent->d_name);
		    regfree (regc);
		  }

		close (proc_cmdline);

	      }

	  closedir (procdir_id);

	  now = time (NULL);

	  out_proc = fopen ("/var/run/restartd", "wt");

	  fprintf (out_proc, "%s\n", ctime (&now));
	  for (i = 0; i < config_process_number; i++)
	    {

	      if (strlen (config_process[i].processes) > 0)
		{
		  if (strlen (config_process[i].running) > 0)
		    {
		      strcpy (config_process[i].status, "running");
		      syslog (LOG_INFO, "%s is running, executing '%s'",
			      config_process[i].name,
			      config_process[i].running);
		      system (config_process[i].running);
		    }
		  else
		    {
		      strcpy (config_process[i].status, "running");
		    }
		}

	      if (strlen (config_process[i].processes) == 0
		  && strlen (config_process[i].not_running) > 0)
		{
		  strcpy (config_process[i].status, "restarting");
		  syslog (LOG_ERR, "%s is NOT running, executing '%s'",
			  config_process[i].name,
			  config_process[i].not_running);
		  system (config_process[i].not_running);
		}

	      if (strlen (config_process[i].processes) == 0
		  && strlen (config_process[i].not_running) == 0)
		strcpy (config_process[i].status, "not running");

	      fprintf (out_proc, "%-12s %-12s      %s\n",
		       config_process[i].name, config_process[i].status,
		       config_process[i].processes);
	    }

	  fclose (out_proc);

	  sleep (check_interval);

	}
    }

  free (procdir_dirent);
  free (proc_cmdline_str);
  free (proc_cmdline_name);
  free (config_process);
  free (regc);
  return 0;

}
