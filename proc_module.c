#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/sched/cputime.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 2048
#define PROCFS_NAME "lab2"

static struct proc_dir_entry *our_proc_file;
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static int pid = 0;
static int struct_id = 0;

static struct dentry *dentry;

struct user_dentry
{
    unsigned int d_flags;
    unsigned long d_time;
    const unsigned char *d_name;
    const unsigned char *parent_name;
    unsigned long s_blocksize;
};

static int write_dentry(char __user *buffer, loff_t *offset, size_t buffer_length)
{
    /* Declare structures */
    struct user_dentry info;
    int len = 0;

    info = (struct user_dentry){
        .d_flags = dentry->d_flags,
        .d_time = dentry->d_time,
        .s_blocksize = dentry->d_sb->s_blocksize,
        .parent_name = dentry->d_parent->d_name.name,
        .d_name = dentry->d_name.name};

    /* String to input */
    len += sprintf(procfs_buffer,
                   "Flags:%d\nDentry name: %s\nParent name: %s\n"
                   "Superblock size: %lu\nd_time: %lu\n",
                   info.d_flags, info.d_name, info.parent_name, info.s_blocksize, info.d_time);
    pr_info("%s", procfs_buffer);
    /* Copy string to user space */
    if (*offset >= len || copy_to_user(buffer, procfs_buffer, len))
    {
        pr_info("Can't copy to user space\n");
        return -EFAULT;
    }

    /* Return value */
    *offset += len;
    return len;
}

int write_task_cputime(struct task_struct *task, char __user *buffer, loff_t *offset, size_t buffer_length)
{
    /* Declare structures */
    int len = 0;
    u64 utime, stime;

    task_cputime(task, &utime, &stime);

    /* String to input */
    len += sprintf(procfs_buffer,
                   "Utime:%lld\nStime name: %lld\nsum_exec_runtime: %lld\n",
                   utime, stime, utime + stime);
    pr_info("%s", procfs_buffer);
    /* Copy string to user space */
    if (*offset >= len || copy_to_user(buffer, procfs_buffer, len))
    {
        pr_info("Can't copy to user space\n");
        return -EFAULT;
    }

    /* Return value */
    *offset += len;
    return len;
}

static ssize_t procfs_read(struct file *filePointer, char __user *buffer,
                           size_t buffer_length, loff_t *offset)
{
    pr_info("procfs_read: START\n");

    if (buffer_length < PROCFS_MAX_SIZE)
    {
        pr_info("Not enough space in buffer\n");
        return -EFAULT;
    }

    if (pid)
    {
        struct task_struct *task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
        if (task == NULL)
        {
            pr_info("Can't get task struct for this pid\n");
            return -EFAULT;
        }
        if (struct_id == 0)
        {
            return write_dentry(buffer, offset, buffer_length);
        }
        if (struct_id == 1)
        {
            return write_task_cputime(task, buffer, offset, buffer_length);
        }
    }
    return -EFAULT;
}

static ssize_t procfs_write(struct file *file, const char __user *buffer,
                            size_t len, loff_t *off)
{
    int cnt, pId, struct_Id;

    pr_info("procfs_write: start\n");
    if (len > PROCFS_MAX_SIZE)
        procfs_buffer_size = PROCFS_MAX_SIZE;
    else
        procfs_buffer_size = len;

    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
        return -EFAULT;

    cnt = sscanf(procfs_buffer, "%d %d", &pId, &struct_Id);
    if (cnt != 2)
    {
        pr_info("Should be 2 args");
        return -EFAULT;
    }

    struct_id = struct_Id;
    pid = pId;
    dentry = file->f_path.dentry;

    pr_info("Pid is: %d\n", pid);
    pr_info("Struct id is: %d\n", struct_id);

    return procfs_buffer_size;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read = procfs_read,
    .proc_write = procfs_write,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfs_read,
    .write = procfs_write,
};
#endif

static int __init procfs1_init(void)
{
    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops);
    if (NULL == our_proc_file)
    {
        proc_remove(our_proc_file);
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit procfs1_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(procfs1_init);
module_exit(procfs1_exit);

MODULE_LICENSE("GPL");