#include "lib/random.h"

#include <stdlib.h>

void random_init(unsigned short seed)
{
    srand(seed);
}

unsigned short random_rand(void)
{
    return (unsigned short)rand();
}
