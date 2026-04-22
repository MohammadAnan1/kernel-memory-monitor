#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/mutex.h>

#define DEVICE_NAME "container_monitor"
#define IOCTL_ADD_PROCESS _IOW('a', 'a', int*)

MODULE_LICENSE("GPL");

struct process_info {
    pid_t pid;
    unsigned long soft_limit;
    unsigned long hard_limit;
    struct list_head list;
};

static LIST_HEAD(process_list);
static DEFINE_MUTEX(list_lock);
static struct timer_list monitor_timer;
static int major;

/* Get memory usage */
unsigned long get_rss(struct task_struct *task) {
    if (!task->mm)
        return 0;
    return get_mm_rss(task->mm) << PAGE_SHIFT;
}

/* Timer function (NEW API FORMAT) */
void monitor_memory(struct timer_list *t) {
    struct process_info *p, *tmp;
    struct task_struct *task;

    mutex_lock(&list_lock);

    list_for_each_entry_safe(p, tmp, &process_list, list) {
        task = pid_task(find_vpid(p->pid), PIDTYPE_PID);

        if (!task) {
            list_del(&p->list);
            kfree(p);
            continue;
        }

        unsigned long rss = get_rss(task);

        if (rss > p->soft_limit) {
            printk(KERN_INFO "[SOFT LIMIT] PID %d exceeded\n", p->pid);
        }

        if (rss > p->hard_limit) {
            printk(KERN_INFO "[HARD LIMIT] Killing PID %d\n", p->pid);
            send_sig(SIGKILL, task, 0);
            list_del(&p->list);
            kfree(p);
        }
    }

    mutex_unlock(&list_lock);

    /* Restart timer */
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(2000));
}

/* IOCTL handler */
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int data[3];

    if (copy_from_user(data, (int *)arg, sizeof(data)))
        return -EFAULT;

    struct process_info *p = kmalloc(sizeof(*p), GFP_KERNEL);
    if (!p)
        return -ENOMEM;

    p->pid = data[0];
    p->soft_limit = data[1];
    p->hard_limit = data[2];

    mutex_lock(&list_lock);
    list_add(&p->list, &process_list);
    mutex_unlock(&list_lock);

    printk(KERN_INFO "Process %d added\n", p->pid);
    return 0;
}

/* File operations */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = device_ioctl,
};

/* Init */
static int __init monitor_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0)
        return major;

    /* NEW timer setup */
    timer_setup(&monitor_timer, monitor_memory, 0);

    /* Start timer */
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(2000));

    printk(KERN_INFO "Memory Monitor Loaded\n");
    return 0;
}

/* Exit */
static void __exit monitor_exit(void) {
  //del_timer_sync(&monitor_timer);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Memory Monitor Unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
