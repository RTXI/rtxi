#ifndef COMPILER_H
#define COMPILER_H

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

#define prefetch_read(x)  __builtin_prefetch((x),0,1)
#define prefetch_write(x) __builtin_prefetch((x),1,1)

#endif // COMPILER_H
