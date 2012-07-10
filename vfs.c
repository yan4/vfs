#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include "build_time.h"

static struct dentry *root;
static struct dentry* control;

static ssize_t d_read(struct file *file, char __user* buf,
		size_t len, loff_t* off)
{
	return 0;
}


struct file_operations d_ops;
struct file_operations i_ops;

static int traverse (struct dentry* den){
	struct dentry *parent;
	struct dentry *r;
	
	do {
		parent = debugfs_create_dir(den->d_name.name, root);
		if (IS_ERR(parent))
			return PTR_ERR(parent);

		r = debugfs_create_file ("dentry", 0666, parent, den, &d_ops);
		if (IS_ERR(r))
			return PTR_ERR(r);

		r = debugfs_create_file("indoe", 0666, parent, den->d_inode, &i_ops);
		if (IS_ERR(r))
			return PTR_ERR(r);
	
		den = den->d_parent;		
	} while (!IS_ROOT(den));

	return 0;
}


static ssize_t d_write(struct file *file, const char __user* buf,
		size_t len, loff_t* off)
{
	char kbuf[128];
	unsigned long ret;
	struct path path;

	ret = copy_from_user(kbuf, buf, len);
	
	if (ret)
		return ret;
	
	kbuf[len-1] = '\0';

	printk ("Read %s (%ld bytes) from user\n", kbuf, len);

	ret = kern_path(kbuf, 0, &path);	
	if (ret)
		return ret;

	ret = traverse(path.dentry);
	if (ret)
		return ret;

	return len;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = d_read,
	.write = d_write,
};

int __init vfs_init(void){
	printk ("*****BUILD %s*********\n", BUILD_TIME);
	
//	root = debugfs_create_dir("den", NULL);
	root = debugfs_create_file("den", 0x666, NULL, NULL, &fops);
	return 0;
	if (IS_ERR(root))
		return PTR_ERR(root);

	printk ("***********%p\n", &fops.owner);
	control = debugfs_create_file("control", 0666, root, NULL, &fops);
	if (IS_ERR(control))
		return PTR_ERR(control);

	return 0;
}

void __exit vfs_exit(void){
	debugfs_remove(root);
}

module_init(vfs_init);
module_exit(vfs_exit);
MODULE_LICENSE("GPL");
