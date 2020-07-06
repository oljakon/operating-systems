#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define BUF_SIZE PAGE_SIZE

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kondrashova");
MODULE_DESCRIPTION("lab4_proc");

static struct proc_dir_entry *proc_entry, *dir, *symlink;
static char* buffer;
int buffer_index, read_fortune;

struct file_operations fileops =
{
  .read = fortune_read,
  .write = fortune_write
};

ssize_t fortune_write(struct file *filp, const char __user *buf, size_t len, loff_t *offp)
{
  int free_space = (PAGE_SIZE - buffer_index) + 1;

  if (len > free_space)
    return -ENOSPC;

  if (raw_copy_from_user(&buffer[write_index], buf, len))
    return -EFAULT;

  write_index += len;
  buffer[write_index - 1] = '\n';

  return len;
}

ssize_t fortune_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
  int len;
  if (*offp > 0)
    return 0;

  if (read_fortune >= write_index)
    read_fortune = 0;

  len = raw_copy_to_user(buf, &buffer[read_index], count);
  read_fortune += len;
  *offp += len;

  return len;
}



int fortune_module_init(void)
{
  buffer = (char *)vmalloc(BUF_SIZE);
  if (!buffer)
  {
    printk(KERN_INFO "fortune: No memory for create buffer\n");
    return -ENOMEM;
  }
  memset(buffer, 0, BUF_SIZE);

  proc_entry = proc_create("fortune", 0666, NULL, &fileops);
  if (proc_entry == NULL)
  {
    vfree(buffer);
    printk(KERN_INFO "fortune: Couldn't create proc entry\n");
    return -ENOMEM;
  }

  dir = proc_mkdir("proc_dir", NULL);
  symlink = proc_symlink("proc_symlink", NULL, "/proc/proc_dir");
  if ((dir == NULL) || (symlink == NULL))
  {
    vfree(buffer);
    printk(KERN_INFO "fortune: Couldn't create proc dir, symlink\n");
    return -ENOMEM;
  }

  buffer_index = 0;
  read_fortune = 0;

  printk(KERN_INFO "fortune: Module loaded.\n");
  return 0;
}

void fortune_module_exit(void)
{
  remove_proc_entry("fortune", NULL);
  remove_proc_entry("fortune_symlink", NULL);
  remove_proc_entry("fortune_dir", NULL);
  vfree(buffer);
  printk(KERN_INFO "fortune: Module unloaded.\n");
}

module_init(fortune_module_init);
module_exit(fortune_module_exit);
