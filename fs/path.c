#include <print.h>
#include <inode.h>
#include <task.h>
#include <fs.h>

struct inode *inode_sub_lookup(struct inode *dir, char *basename, int len)
{
	struct inode *inode = NULL;
	if (dir->i_ops->sub_lookup)
		inode = dir->i_ops->sub_lookup(dir, basename, len);
	return inode;
}

struct inode *inode_sub_lookup_put(struct inode *dir, char *base, int len)
{
	struct inode *inode = inode_sub_lookup(dir, base, len);
	put_inode(dir);
	return inode;
}

int path_next_entry(char **path)
{
	char *p = *path;
	int len = 0;
	while (*p && *p != '/') {
		p++;
		len++;
	}
	/* skip slash */
	while (*p == '/')
		p++;
	*path = p;
	return len;
}

struct inode *path_lookup_dir(char *path, char **basename, int *baselen)
{
	struct inode *dir, *prev, *start;
	char *base;
	int len, ddff = 0;	/* double dot fast fails */
	/* get start dir inode */
	if (path[0] == '/') {
		start = ctask->fs.root_dir;
		prev = get_inode_ref(start);
		/* skip slash */
		while (*path == '/')
			path++;
	} else {
		start = ctask->fs.current_dir;
		prev = NULL;
	}
	dir = get_inode_ref(start);
	while (1) {
		base = path;
		len = path_next_entry(&path);
		if (path[0] == '\0')
			break;
		/*
		 * Fast path lookup of '.' or '..':
		 *  we dont save all previous dir inode, so '..' fast lookup
		 *  maybe fail when prev equals NULL.
		 */
		ddff = 0;
		if (base[0] == '.') {
			if (len == 1) {
				continue;
			} else if (len == 2 && base[1] == '.') {
				if (prev) {
					put_inode(dir);
					dir = get_inode_ref(prev);
					if (dir != start) {
						put_inode(prev);
						prev = NULL;
					}
					continue;
				}
				ddff = 1;
			}
		}

		if (!S_ISDIR(dir->i_mode)) {
			dir = NULL;
			len = 0;
			break;
		}
		if (!ddff) {
			if (prev)
				put_inode(prev);
			prev = get_inode_ref(dir);
		}
		dir = inode_sub_lookup_put(dir, base, len);
		if (!dir) {
			len = 0;
			break;
		}
	}
	if (prev)
		put_inode(prev);
	if (len == 0)
		base = NULL;
	if (basename)
		*basename = base;
	if (baselen)
		*baselen = len;
	return dir;
}

