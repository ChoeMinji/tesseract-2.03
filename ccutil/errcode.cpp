/**********************************************************************
 * File:        errcode.c  (Formerly error.c)
 * Description: Generic error handler function
 * Author:      Ray Smith
 * Created:     Tue May  1 16:28:39 BST 1990
 *
 * (C) Copyright 1989, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include          "mfcpch.h"     //precompiled headers
#include          <stdio.h>
#include          <stdlib.h>
#include          <stdarg.h>
#include          <string.h>
#ifdef __UNIX__
#include          <signal.h>
#endif
#include          "tprintf.h"
//#include                                      "ipeerr.h"
#include          "errcode.h"

const ERRCODE BADERRACTION = "Illegal error action";
#define MAX_MSG       1024
extern inT16 global_abort_code;

/**********************************************************************
 * error
 *
 * Print an error message and continue, exit or abort according to action.
 * Makes use of error messages and numbers in a common place.
 *
 **********************************************************************/
void
ERRCODE::error (                 //handle error
const char *caller,              //name of caller
inT8 action,                     //action to take
const char *format, ...          //special message
) const
{
  va_list args;                  //variable args
  char msg[MAX_MSG];
  char *msgptr = msg;

  if (caller != NULL)
                                 //name of caller
    msgptr += sprintf (msgptr, "%s:", caller);
                                 //actual message
  msgptr += sprintf (msgptr, "Error:%s", message);
  if (format != NULL) {
    msgptr += sprintf (msgptr, ":");
    va_start(args, format);  //variable list
    #ifdef __MSW32__
                                 //print remainder
    msgptr += _vsnprintf (msgptr, MAX_MSG - 2 - (msgptr - msg), format, args);
    msg[MAX_MSG - 2] = '\0';     //ensure termination
    strcat (msg, "\n");
    #else
                                 //print remainder
    msgptr += vsprintf (msgptr, format, args);
                                 //no specific
    msgptr += sprintf (msgptr, "\n");
    #endif
    va_end(args);
  }
  else
                                 //no specific
    msgptr += sprintf (msgptr, "\n");

  tprintf(msg);
  if ((strstr (message, "File") != NULL) ||
    (strstr (message, "file") != NULL))
    global_abort_code = FILE_ABORT;
  else if ((strstr (message, "List") != NULL) ||
    (strstr (message, "list") != NULL))
    global_abort_code = LIST_ABORT;
  else if ((strstr (message, "Memory") != NULL) ||
    (strstr (message, "memory") != NULL))
    global_abort_code = MEMORY_ABORT;
  else
    global_abort_code = NO_ABORT_CODE;

  switch (action) {
    case DBG:
    case TESSLOG:
      return;                    //report only
    case EXIT:
      err_exit();
    case ABORT:
      abort();
    default:
      BADERRACTION.error ("error", ABORT, NULL);
  }
}
