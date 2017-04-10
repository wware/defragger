#ifndef DFLAYER_H_INCLUDED
#define DFLAYER_H_INCLUDED

#include <stdlib.h>

void *dfmalloc(size_t size);
void dffree(void *ptr);
void *dfrealloc(void *ptr, size_t size);
double dfusage(void);
void dfinit(void);

#endif
