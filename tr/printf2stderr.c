/* Sandip Nath Tiwari - 20162032 */
#include "tr.h"

#define MAX_ERR_STRLEN 1024
static char err_str[MAX_ERR_STRLEN+1];

int printf2stderr(const char *fmt, ...)
{
	va_list ap;
	int retval;

	va_start(ap, fmt);
	retval = vsnprintf(err_str, MAX_ERR_STRLEN, fmt, ap);
	va_end(ap);

	write(2, err_str, strlen(err_str));
	return retval;
}
