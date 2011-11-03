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
		/* skip slash */
		while (*path == '/')
			path++;
	} else {
		start = ctask->fs.current_dir;
	}
	prev = dir = start;
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
					dir = prev;
					if (dir != start)
						prev = NULL;
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
		if (!ddff)
			prev = dir;
		dir = inode_sub_lookup(dir, base, len);
		if (!dir) {
			len = 0;
			break;
		}
	}
	if (len == 0)
		base = NULL;
	if (basename)
		*basename = base;
	if (baselen)
		*baselen = len;
	return dir;
}

struct inode *inode_path_lookup(char *path)
{
	struct inode *dir;
	char *basename;
	int len;
	dir = path_lookup_dir(path, &basename, &len);
	if (!dir || len == 0)
		return NULL;
	return inode_sub_lookup(dir, basename, len);
}

