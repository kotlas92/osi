#include <regex.h>

#ifndef VALIDATE_H
#define VALIDATE_H

int match(const char *string, char *pattern)
{
    int        status;
    regex_t    re;

    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
      return(0);
    }
    status = regexec(&re, string, (size_t) 0, NULL, 0);
    regfree(&re);
    if (status != 0) {
   	  return(0);
    }
    return(1);
}
#endif //VALIDATE_H