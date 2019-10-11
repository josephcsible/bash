/*
 * readwrite -- undoes the effect of readonly
 */

/*
   Copyright (C) 1999-2009 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "loadables.h"
#include "bashintl.h"


#if defined (ARRAY_VARS)
#  define ATTROPTS      "aAf"
#else
#  define ATTROPTS      "f"
#endif

/* For each variable name in LIST, make that variable readwrite. */
int
readwrite_builtin (list)
     register WORD_LIST *list;
{
  register SHELL_VAR *var;
  int any_failed, opt;
  int functions_only, arrays_only, assoc_only;
  char *name;

  functions_only = arrays_only = assoc_only = 0;
  any_failed = 0;
  /* Read arguments from the front of the list. */
  reset_internal_getopt ();
  while ((opt = internal_getopt (list, ATTROPTS)) != -1)
    {
      switch (opt)
        {
          case 'f':
            functions_only = 1;
            break;
#if defined (ARRAY_VARS)
          case 'a':
            arrays_only = 1;
            break;
          case 'A':
            assoc_only = 1;
            break;
#endif
          CASE_HELPOPT;
          default:
            builtin_usage ();
            return (EX_USAGE);
        }
    }
  list = loptend;

  if (list)
    {
      while (list)
        {
          name = list->word->word;

          if (functions_only)           /* xxx -f name */
            {
              var = find_function (name);
              if (var == 0)
                {
                  builtin_error (_("%s: not a function"), name);
                  any_failed++;
                }
              else
                SETVARATTR (var, att_readonly, 1);

              list = list->next;
              continue;
            }

          if (legal_identifier (name) == 0)
            {
              sh_invalidid (name);
              any_failed++;
              list = list->next;
              continue;
            }

		  var = find_variable (name);

		  if (var)
			SETVARATTR (var, att_readonly, 1);

		  if (var && exported_p (var))
			array_needs_making++;       /* XXX */

          list = list->next;
        }
    }

  return ((any_failed == 0) ? EXECUTION_SUCCESS
                            : EXECUTION_FAILURE);
}

char *readwrite_doc[] = {
  "Unmark shell variables as unchangeable.",
  "",
  "Unmark each NAME as read-only; the values of these NAMEs may once",
  "again be changed by subsequent assignment.",
  "",
  "Options:",
  "  -a    refer to indexed array variables",
  "  -A    refer to associative array variables",
  "  -f    refer to shell functions",
  "",
  "An argument of `--' disables further option processing.",
  "",
  "Exit Status:",
  "Returns success unless an invalid option is given or NAME is invalid.",
  (char *)0
};

struct builtin readwrite_struct = {
  "readwrite",
  readwrite_builtin,
  BUILTIN_ENABLED,
  readwrite_doc,
  "readwrite [-aAf] [name ...]",
  0
};
