/******************************************************************/
/**
**  @file errors.cpp
**  @brief Error-handling routines.
**
**  Created Dec. 97 from earlier routine embedded in child.cpp
**  $Id: errors.cpp,v 1.7 2004-01-07 10:53:24 childcvs Exp $
*/
/******************************************************************/

#include "errors.h"

#include <stdlib.h>
#if !defined(HAVE_NO_NAMESPACE)
# include <iostream>
using namespace std;
#else
# include <iostream.h>
#endif

#define CHILD_ABORT_ON_ERROR "CHILD_ABORT_ON_ERROR"
#define CHILD_ABORT_ON_WARNING "CHILD_ABORT_ON_WARNING"

/*****************************************************************************\
**
**  ReportFatalError:  This is an error-handling routine that prints the
**                     message errMsg then halts the program.
**
**      Parameters:     errMsg -- error message
**      Called by:
**      Created: 12/96 GT
**
\*****************************************************************************/
void ReportFatalError( const char *errMsg )
{
  cout << errMsg <<endl;
  cout << "That was a fatal error, my friend!" <<endl;
  if (getenv(CHILD_ABORT_ON_ERROR) != NULL)
    abort();
  cout << "(Set \"" CHILD_ABORT_ON_ERROR "\" to generate a crash.)" <<endl;
  exit(1);
}



/*****************************************************************************\
**
**  ReportWarning:  This is an error-handling routine that prints the
**                     message errMsg but does not halt the program unless
**                     the environment variable CHILD_ABORT_ON_WARNING is set.
**
**      Parameters:     errMsg -- error message
**      Called by:
**      Created: 9/03 SL
**
\*****************************************************************************/
void ReportWarning( const char *errMsg )
{
  cout << "WARNING: " << errMsg <<endl;
  if (getenv(CHILD_ABORT_ON_WARNING) != NULL)
    abort();
  cout << "(Set \"" CHILD_ABORT_ON_WARNING "\" to generate a crash.)" <<endl;
}

