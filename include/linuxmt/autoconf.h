/*
 * Automatically generated by make menuconfig: don't edit
 */
#define AUTOCONF_INCLUDED

/*
 * Advanced - for Developers and Hackers only
 */
#undef  CONFIG_EXPERIMENTAL
#undef  CONFIG_OBSOLETE
#undef  CONFIG_NOT_YET

/*
 * Architecture
 */
#define CONFIG_ARCH_AUTO 1
#undef  CONFIG_ARCH_PC
#undef  CONFIG_ARCH_SIBO
#define CONFIG_ARCH_PC_AUTO 1
#undef  CONFIG_ARCH_PC_XT
#undef  CONFIG_ARCH_PC_AT
#undef  CONFIG_ARCH_PC_MCA
#undef  CONFIG_ROMCODE

/*
 * 286 Protected Mode Support
 */
#undef  CONFIG_286PMODE

/*
 * Driver Support
 */

/*
 * Character device drivers
 */
#define CONFIG_CONSOLE_DIRECT 1
#undef  CONFIG_CONSOLE_BIOS
#undef  CONFIG_CONSOLE_SERIAL
#define CONFIG_DCON_VT52 1
#define CONFIG_DCON_ANSI 1
#undef  CONFIG_DCON_ANSI_PRINTK
#define CONFIG_US_KEYMAP 1
#undef  CONFIG_BE_KEYMAP
#undef  CONFIG_UK_KEYMAP
#undef  CONFIG_DE_KEYMAP
#undef  CONFIG_DV_KEYMAP
#undef  CONFIG_ES_KEYMAP
#undef  CONFIG_FR_KEYMAP
#undef  CONFIG_SE_KEYMAP
#define CONFIG_DCON_KRAW 1
#define CONFIG_CHAR_DEV_RS 1
#define CONFIG_CHAR_DEV_LP 1
#define CONFIG_CHAR_DEV_MEM 1
#define CONFIG_PSEUDO_TTY 1
#undef  CONFIG_DEV_META

/*
 * Block device drivers
 */
#define CONFIG_BLK_DEV_BIOS 1
#define CONFIG_BLK_DEV_BFD 1
#define CONFIG_BLK_DEV_BHD 1
#undef  CONFIG_BLK_DEV_FD
#undef  CONFIG_BLK_DEV_HD
#undef  CONFIG_BLK_DEV_XD
#undef  CONFIG_DMA
#define CONFIG_GENDISK 1
#define CONFIG_BLK_DEV_RAM 1
#define CONFIG_BLK_DEV_CHAR 1

/*
 * Filesystems
 */
#define CONFIG_MINIX_FS 1
#undef  CONFIG_ROMFS_FS
#undef  CONFIG_ELKSFS_FS
#undef  CONFIG_FULL_VFS
#define CONFIG_FS_EXTERNAL_BUFFER 1
#define CONFIG_PIPE 1
#define CONFIG_EXEC_MINIX 1
#undef  CONFIG_EXEC_MSDOS

/*
 * Network Support
 */

/*
 * Networking
 */
#define CONFIG_SOCKET 1
#undef  CONFIG_UNIX
#undef  CONFIG_NANO
#define CONFIG_INET 1
#define CONFIG_INET_STATUS 1
#undef  CONFIG_SOCK_CLIENTONLY
#define CONFIG_SOCK_STREAMONLY 1

/*
 * Kernel hacking
 */
#undef  CONFIG_STRACE
#undef  CONFIG_OPT_SMALL
