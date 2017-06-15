/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_LIBSTDINCLUDE_H__
#define __SLIB_LIBSTDINCLUDE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include <errno.h>

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <stdint.h>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>

#define stricmp strcasecmp
#define strnicmp strncasecmp

#endif
