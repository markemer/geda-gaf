/* gEDA - GPL Electronic Design Automation
 * gsymcheck - gEDA Symbol Check 
 * Copyright (C) 1998-2010 Ales Hvezda
 * Copyright (C) 1998-2010 gEDA Contributors (see ChangeLog for details)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02111-1301 USA.
 */

#include <config.h>

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <libgeda/libgeda.h>

#include "../include/struct.h"
#include "../include/globals.h"
#include "../include/prototype.h"
#include "../include/gettext.h"

#define OPTIONS "qvh"

#ifndef OPTARG_IN_UNISTD
extern char *optarg;
extern int optind;
#endif

void
usage(char *cmd)
{
	printf(_(
            "Usage: %s [OPTIONS] filename1 ... filenameN\n"
	    "  -h            Print usage\n"
	    "  -q            Quiet mode\n"
	    "  -v            Verbose mode (cumulative: errors, warnings, info)\n"
	    "                Use this to get the actual symbol error messages\n"
	    "\nfilename1 ... filenameN are the symbols to check\n"
	    "\n"), cmd);
	exit(0);
}

int
parse_commandline(int argc, char *argv[])
{
  int ch;

  while ((ch = getopt (argc, argv, OPTIONS)) != -1) {
    switch (ch) {

      case 'v':
        verbose_mode++;
        break;

      case 'q':
        quiet_mode=TRUE;
        break;

#if 0
      case 'f':
        printf("f arg: %s\n", optarg);
        break;
#endif

      case 'h':
        usage(argv[0]);
        break;

      case '?':
      default:
        usage(argv[0]);
        break;
    }
  }

  return(optind);
}
