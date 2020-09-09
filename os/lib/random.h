#ifndef RANDOM_H_
#define RANDOM_H_

void random_init(unsigned short seed);

unsigned short random_rand(void);

#define RANDOM_RAND_MAX 65535U

#endif /* RANDOM_H_ */
