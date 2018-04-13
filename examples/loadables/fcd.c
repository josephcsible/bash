/* fcd - change the shell working directory to the specified file descriptor */

/* See Makefile for compilation details. */

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include "bashansi.h"
#include <stdio.h>
#include <errno.h>

#include "loadables.h"

#if !defined (errno)
extern int errno;
#endif

extern char *strerror ();

extern int array_needs_making;

static int
setpwd (dirname)
     char *dirname;
{
  int old_anm;
  SHELL_VAR *tvar;

  old_anm = array_needs_making;
  tvar = bind_variable ("PWD", dirname ? dirname : "", 0);
  if (tvar && readonly_p (tvar))
    return EXECUTION_FAILURE;
  if (tvar && old_anm == 0 && array_needs_making && exported_p (tvar))
    {
      update_export_env_inplace ("PWD=", 4, dirname ? dirname : "");
      array_needs_making = 0;
    }
  return EXECUTION_SUCCESS;
}

static int
bindpwd ()
{
  char *dirname, *pwdvar;
  int old_anm, r;
  SHELL_VAR *tvar;

  r = sh_chkwrite (EXECUTION_SUCCESS);

  dirname = get_working_directory ("fcd");

  old_anm = array_needs_making;
  pwdvar = get_string_value ("PWD");

  tvar = bind_variable ("OLDPWD", pwdvar, 0);
  if (tvar && readonly_p (tvar))
    r = EXECUTION_FAILURE;

  if (old_anm == 0 && array_needs_making && exported_p (tvar))
    {
      update_export_env_inplace ("OLDPWD=", 7, pwdvar);
      array_needs_making = 0;
    }

  if (setpwd (dirname) == EXECUTION_FAILURE)
    r = EXECUTION_FAILURE;

  if (dirname && dirname != the_current_working_directory)
    free (dirname);

  return (r);
}

int
fcd_builtin (list)
     WORD_LIST *list;
{
  intmax_t fd;

  CHECK_HELPOPT (list);

  if (get_numeric_arg (list, 0, &fd) == 0)
    return (EXECUTION_FAILURE);

  if (fd < 0 || fd > INT_MAX)
    {
      sh_erange (list ? list->word->word : NULL, "file descriptor");
      return (EXECUTION_FAILURE);
    }

  if (fchdir ((int) fd))
    {
      builtin_error ("%d: %s", (int) fd, strerror (errno));
      return (EXECUTION_FAILURE);
    }

  FREE (the_current_working_directory);
  the_current_working_directory = (char *)NULL;

  return (bindpwd ());
}

char *fcd_doc[] = {
	"Change the shell working directory.",
	"",
	"Change the current directory to that referenced by the file descriptor",
	"FD.",
	"",
	"Exit Status:",
	"Returns 0 if the directory is changed; non-zero otherwise.",
	(char *)NULL
};

struct builtin fcd_struct = {
	"fcd",			/* builtin name */
	fcd_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,		/* initial flags for builtin */
	fcd_doc,			/* array of long documentation strings. */
	"fcd [fd]",			/* usage synopsis; becomes short_doc */
	0				/* reserved for internal use */
};
