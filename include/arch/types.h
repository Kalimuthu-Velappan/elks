/* arch/i86/include/asm/types.h - Basic Linux/MT data types. */

#ifndef LX86_ARCH_TYPES_H
#define LX86_ARCH_TYPES_H

/*@-namechecks@*/

/* First we define all of the __u and __s types...*/

#define signed

typedef unsigned char			__u8,		*__pu8;
typedef signed char			__s8,		*__ps8;
typedef unsigned short int		__u16,		*__pu16;
typedef signed short int		__s16,		*__ps16;
typedef unsigned long int		__u32,		*__pu32;
typedef signed long int 		__s32,	 	*__ps32;

/* __uint == 16bit here */

typedef unsigned short int		__uint,		*__puint;
typedef signed short int		__sint,		*__psint;

/* Then we define registers, etc... */

struct _registers {
    __u16	ksp, sp, ss, ds, bx, cx, dx,
		 di, si, ax, es, bp, ip, cs, flags;
};

typedef struct _registers		__registers,	*__pregisters;

/* Changed to unsigned short int as that is what it is here.
 */

typedef __u16			__pptr;

struct _mminit {
    __u16	cs, endcs,
		ds, endds,
		ss, endss,
		    lowss;
};

typedef struct _mminit		__arch_mminit, *__parch_mminit;

/*@+namechecks@*/

#ifndef NULL
#define NULL		((void *) 0)
#endif

#endif
