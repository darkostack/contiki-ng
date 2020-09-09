#ifndef _CC_GCC_H_
#define _CC_GCC_H_

#ifdef __GNUC__
#ifndef CC_CONF_INLINE
#define CC_CONF_INLINE __inline__
#endif
#define CC_CONF_ALIGN(n) __attribute__((__aligned__(n)))
#define CC_CONF_NORETURN __attribute__((__noreturn__))
#endif /* __GNUC__ */

#endif /* _CC_GCC_H_ */
