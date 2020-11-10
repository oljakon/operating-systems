#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>

#define BUF_SIZE PAGE_SIZE

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kondrashova");
MODULE_DESCRIPTION("lab4_proc");

ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);
int fortune_init(void);
void fortune_exit(void);

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = fortune_read,
    .write = fortune_write,
};


static char *buffer;
static struct proc_dir_entry *proc_file, *dir, *symlink;
int read_index, write_index;

ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
    int len;

    if (write_index == 0 || *f_pos > 0)
        return 0;

    if (read_index >= write_index)
        read_index = 0;

    len = sprintf(buf, "%s\n", &buffer[read_index]);
    read_index += len;
    *f_pos += len;

    return len;
}

ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
    int free_space = (BUF_SIZE - write_index) + 1;

    if (count > free_space)
    {
        printk(KERN_INFO "Buffer is full\n");
        return -ENOSPC;
    }

    if (copy_from_user(&buffer[write_index], buf, count))
    {
        return -EFAULT;
    }

    write_index += count;
    buffer[write_index-1] = 0;

    return count;
}


int fortune_init(void)
{
    buffer = (char*)vmalloc(BUF_SIZE);

    if (!buffer)
    {
        printk(KERN_INFO "Not enough memory\n");
        return -ENOMEM;
    }

    memset(buffer, 0, BUF_SIZE);
    proc_file = proc_create("fortune", 0666, NULL, &fops);

    if (!proc_file)
    {
        vfree(buffer);
        printk(KERN_INFO "Cannot create fortune file.\n");
        return -ENOMEM;
    }

    dir = proc_mkdir("fortune_dir", NULL);
    symlink = proc_symlink("fortune_symlink", NULL, "/proc/fortune_dir");

    read_index = 0;
    write_index = 0;

    printk(KERN_INFO "Fortune module loaded.\n");
    return 0;
}


void fortune_exit(void)
{
    remove_proc_entry("fortune", NULL);
    remove_proc_entry("fortune_dir", NULL);
    remove_proc_entry("fortune_symlink", NULL);

    if (buffer)
        vfree(buffer);

    printk(KERN_INFO "Fortune module unloaded.\n");
}

module_init(fortune_init);
module_exit(fortune_exit);


