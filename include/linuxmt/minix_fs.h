#ifndef LX86_LINUXMT_MINIX_FS_H
#define LX86_LINUXMT_MINIX_FS_H

/* The minix filesystem constants/structures
 *
 * Thanks to Kees J Bot for sending me the definitions of the new minix
 * filesystem (aka V2) with bigger inodes and 32-bit block pointers.
 */

#define MINIX_ROOT_INO 1

/* Not the same as the bogus LINK_MAX in <linux/limits.h>. Oh well. */
#define MINIX_LINK_MAX	250

#define MINIX_I_MAP_SLOTS	8
#define MINIX_Z_MAP_SLOTS	64
#define MINIX_SUPER_MAGIC	0x137F	/* original minix fs */
#define MINIX_SUPER_MAGIC2	0x138F	/* minix fs, 30 char names */
#define MINIX2_SUPER_MAGIC	0x2468	/* minix V2 fs */
#define MINIX2_SUPER_MAGIC2	0x2478	/* minix V2 fs, 30 char names */
#define MINIX_VALID_FS		0x0001	/* Clean fs. */
#define MINIX_ERROR_FS		0x0002	/* fs has errors. */

#define MINIX_INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct minix_inode)))
#define MINIX2_INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct minix2_inode)))

#define MINIX_V1		0x0001	/* original minix fs */
#define MINIX_V2		0x0002	/* minix V2 fs */

#define INODE_VERSION(inode)	inode->i_sb->u.minix_sb.s_version

/*
 * This is the original minix inode layout on disk.
 * Note the 8-bit gid and atime and ctime.
 */
struct minix_inode {
    __u16 i_mode;
    __u16 i_uid;
    __u32 i_size;
    __u32 i_time;
    __u8 i_gid;
    __u8 i_nlinks;
    __u16 i_zone[9];
};

/*
 * minix super-block data on disk
 */
struct minix_super_block {
    __u16 s_ninodes;
    __u16 s_nzones;
    __u16 s_imap_blocks;
    __u16 s_zmap_blocks;
    __u16 s_firstdatazone;
    __u16 s_log_zone_size;
    __u32 s_max_size;
    __u16 s_magic;
    __u16 s_state;
    __u32 s_zones;
};

struct minix_dir_entry {
    __u16 inode;
    char name[0];
};

#ifdef __KERNEL__

extern int minix_lookup(register struct inode *dir, char *name, int len,
			register struct inode **result);
extern int minix_create(register struct inode *dir, char *name, int len,
			int mode, struct inode **result);
extern int minix_mkdir(register struct inode *dir, char *name, int len,
		       int mode);
extern int minix_rmdir(register struct inode *dir, char *name, int len);
extern int minix_unlink(struct inode *dir, char *name, int len);
extern int minix_symlink(struct inode *dir, char *name, int len,
			 char *symname);
extern int minix_link(register struct inode *oldinode,
		      register struct inode *dir, char *name, int len);
extern int minix_mknod(register struct inode *dir, char *name, int len,
		       int mode, int rdev);
extern int minix_rename();
extern struct inode *minix_new_inode(struct inode *dir);
extern void minix_free_inode(register struct inode *inode);
extern unsigned long minix_count_free_inodes(register struct super_block
					     *sb);
extern unsigned int minix_new_block(register struct super_block *sb);
extern void minix_free_block(register struct super_block *sb,
			     unsigned short block);
extern unsigned long minix_count_free_blocks(register struct super_block
					     *sb);
extern int minix_bmap();

extern struct buffer_head *minix_getblk(register struct inode *inode,
					unsigned short block, int create);
extern struct buffer_head *minix_bread(struct inode *inode,
				       unsigned short block, int create);

extern void minix_truncate(register struct inode *inode);
extern void minix_put_super(register struct super_block *sb);
extern struct super_block *minix_read_super(register struct super_block *s,
					    char *data, int silent);
extern int init_minix_fs(void);
extern void minix_write_super(register struct super_block *sb);
extern int minix_remount(register struct super_block *sb, int *flags,
			 char *data);
extern void minix_read_inode(register struct inode *inode);
extern void minix_write_inode(register struct inode *inode);
extern void minix_put_inode(register struct inode *inode);
extern void minix_statfs(register struct super_block *sb,
			 struct statfs *buf, int bufsize);
extern int minix_sync_inode(register struct inode *inode);
extern int minix_sync_file();

extern struct inode_operations minix_file_inode_operations;
extern struct inode_operations minix_dir_inode_operations;
extern struct inode_operations minix_symlink_inode_operations;

#endif

#endif
