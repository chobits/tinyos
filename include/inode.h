#ifndef __INODE_H
#define __INODE_H

#include <types.h>

struct dirty_block_cache;
struct block;
struct inode;
struct file_stat;
struct dir_stat;

struct inode_operations {
	int (*read)(struct inode *, char *, size_t, off_t);
	int (*write)(struct inode *, char *, size_t, off_t);
	struct inode *(*sub_lookup)(struct inode *, char *, int);
	void (*update_size)(struct inode *, size_t);
	void (*close)(struct inode *);
	void (*sync)(struct inode *);
	int (*getdir)(struct inode *, int, int, struct dir_stat *);
};

struct inode {
	unsigned int i_size;
	unsigned int i_ino;
	unsigned int i_mode;
	int i_refcnt;
	struct inode_operations *i_ops;
	struct super_block *i_sb;
	struct dirty_block_cache *i_dbc;
};

extern struct inode *inode_sub_lookup(struct inode *dir, char *basename, int len);
extern struct inode *path_lookup_dir(char *path, char **basename, int *baselen);
extern struct inode *inode_path_lookup(char *path);
extern struct inode *get_inode_ref(struct inode *inode);
extern int inode_read(struct inode *inode, char *buf, size_t size, off_t off);
extern int inode_write(struct inode *inode, char *buf, size_t size, off_t off);
extern struct inode *inode_open(char *path);
extern void inode_close(struct inode *inode);
extern void inode_sync(struct inode *inode);
extern void inode_stat(struct inode *, struct file_stat *);
/* dirty block cache */
extern void inode_sync_dbc(struct inode *inode);
extern void inode_free_dbc(struct inode *inode);
extern void inode_add_dbc(struct inode *inode, struct block *block);
extern void inode_chdir(struct inode *inode);
extern int inode_getdir(struct inode *inode, int start, int num, struct dir_stat *ds);

#endif	/* inode.h */
