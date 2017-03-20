/*
 * kern.c
 *
 *  Created on: 23 марта 2016 г.
 *      Author: root
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include <asm/page.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>

#include <linux/poll.h>
#include <linux/mm.h> // for remap_pfn_range
#include <linux/fs.h> // for file_operations, register_chrder ()

#include <linux/fdtable.h>
#include <linux/rcupdate.h>
#include <linux/eventfd.h>

#include "kern.h"
#include "fops_chdev.h"

char *buffer;
char *usr_buffer = NULL;

int buffer_filled = 0;
int calculation_done = 0;
int testing_started = 0;
static int Major  ;

struct timer_list calc_timer;

long long int start_time = 0, stop_time = 0;
long long int total_time = 0;

long long int kstart_time = 0, kstop_time = 0;
long long int ktotal_time = 0, num_tests = 0;

//Structs from userspace
unsigned long usr_pid = 0;
unsigned long usr_efd = 3;
unsigned long usr_efd2 = 4;
unsigned long cur_cmd = 0;

//Structs to resolve references to fd
struct task_struct *userspace_task = NULL;	//ptr to usr space task struct
struct file *efd_file = NULL;				//ptr to eventfd's file struct
struct file *efd_file2 = NULL;				//ptr to eventfd's file struct
struct eventfd_ctx *efd_ctx = NULL;			//ptr to eventfd context
struct eventfd_ctx *efd_ctx2 = NULL;			//ptr to eventfd context

int parse_usr_param(void);

static int __set_usr_pid(const char *str, struct kernel_param *kp){
	int retval;
	retval = kstrtoul(str, 10, &usr_pid);
	if(retval < 0){
		printk("Error while parsing usr_pid.\n");
		return -1;
	}

	return 0;
}

static int __get_usr_pid(char *buffer, struct kernel_param *kp){
	return scnprintf(buffer, PAGE_SIZE, "%lu", usr_pid);
}

static int __set_usr_efd(const char *str, struct kernel_param *kp){
	int retval;
	retval = kstrtoul(str, 10, &usr_efd);
	if(retval < 0){
		printk("Error while parsing usr_efd.\n");
		return -1;
	}

	return 0;
}

static int __get_usr_efd(char *buffer, struct kernel_param *kp){
	return scnprintf(buffer, PAGE_SIZE, "%lu", usr_efd);
}

static int __set_usr_efd2(const char *str, struct kernel_param *kp){
	int retval;
	retval = kstrtoul(str, 10, &usr_efd2);
	if(retval < 0){
		printk("Error while parsing usr_efd2.\n");
		return -1;
	}

	return 0;
}

static int __get_usr_efd2(char *buffer, struct kernel_param *kp){
	return scnprintf(buffer, PAGE_SIZE, "%lu", usr_efd2);
}

static int __set_cur_cmd(const char *str, struct kernel_param *kp){
	int retval;
	retval = kstrtoul(str, 10, &cur_cmd);
	if(retval < 0){
		printk("Error while parsing mmap_ready param.\n");
		return -1;
	}

	switch (cur_cmd){
	case EFD_FIND:
		parse_usr_param();
		break;

	case EFD_MMAP_CMD:
		eventfd_signal(efd_ctx, EFD_MMAP_CMD);
		break;

	case EFD_START_TEST_CMD:
		testing_started = 1;
		buffer_filled = 0;
		calculation_done = 1;

		add_timer(&calc_timer);
		printk("Timer is started.\n");

		ktotal_time = 0;
		num_tests = 0;
		eventfd_signal(efd_ctx, EFD_START_TEST_CMD);
		break;

	case EFD_STOP_TEST_CMD:
		printk("Avg copy time = %llu\n", ktotal_time / num_tests);

		del_timer_sync(&calc_timer);
		printk("Timer is deleted.\n");

		testing_started = 0;
		buffer_filled = 0;
		calculation_done = 1;

		eventfd_signal(efd_ctx, EFD_STOP_TEST_CMD);

		break;

	case EFD_EXIT_TEST_CMD:
		eventfd_signal(efd_ctx, EFD_EXIT_TEST_CMD);
		break;

	default:
		printk("Unsupported command.\n");
	}

	return 0;
}

static int __get_cur_cmd(char *buffer, struct kernel_param *kp){
	return scnprintf(buffer, PAGE_SIZE, "%lu", cur_cmd);
}


void fill_buffer(char *buf, unsigned long long len)
{
	int i;

	buffer_filled = 0;
	for(i = 0; i < len ; i++)
	{
		buf[i] = i % 255;
	}
	buffer_filled = 1;

	calculation_done = 0;
	kstart_time = ktime_to_ns(ktime_get());

	eventfd_signal(efd_ctx, EFD_MEMORY_READY);
	//wake_up_interruptible(&wq_buffer);
	
	printk("Buffer filled\n");

}

void release_buffer(char *buffer)
{
	kfree(buffer);
	buffer_filled = 0;
	calculation_done = 0;
}

void print_buf(char *buf, int len)
{
	int i;
	printk("Buffer :\n");
	for(i = 0; i < len  ; i++)
	{
		printk("%d, ",buf[i]);
	}
	printk("\n");
}

// vfs methods
static struct file_operations mmaptest_fops = {
		mmap:		mmaptest_mmap,
		open: 		mmaptest_open,
		read:		mmaptest_read,
		release: 	mmaptest_release,
		unlocked_ioctl:		mmaptest_ioctl,
		poll:		mmaptest_poll,
};


static void __timer_handler(unsigned long param)
{
	printk("Timer handler called\n");
	printk("In handler: buffer_filled = %d, calculation_done = %d\n", buffer_filled, calculation_done);

	if(!buffer_filled || calculation_done){
		//start monitoring for completion
		printk("Start monitoring for efd2...\n");

		fill_buffer(buffer, BUF_TEST_SIZE);
	}

	mod_timer(&calc_timer, jiffies + msecs_to_jiffies(TIMER_DELAY));
}

int parse_usr_param(){
	//parse userspace argumetns to find efd
	printk("Received from userspace: usr_pid=%lu, efd=%lu, efd2=%lu\n", usr_pid, usr_efd, usr_efd2);

	userspace_task = pid_task(find_vpid(usr_pid), PIDTYPE_PID);
	printk("Resolved pointer to the userspace program task struct: %p\n",userspace_task);

	rcu_read_lock();
	efd_file = fcheck_files(userspace_task->files, usr_efd);
	efd_file2 = fcheck_files(userspace_task->files, usr_efd2);
	rcu_read_unlock();

	printk("Resolved pointer to the userspace program's eventfd's file struct: %p and struct2 : %p\n",efd_file, efd_file2);

	efd_ctx = eventfd_ctx_fileget(efd_file);
	efd_ctx2 = eventfd_ctx_fileget(efd_file2);
	if(!efd_ctx || !efd_ctx2){
		printk("eventfd_ctx_fileget() failed..\n");
		unregister_chrdev(Major, DEVICE_NAME);
		return -1;
	}
	printk("Resolved pointer to the userspace program's eventfd's context: %p and context2 : %p\n", efd_ctx, efd_ctx2);
	return 0;
}

static int __init_module ( void )
{
	printk("module started...\n");

	Major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &mmaptest_fops);
	if(Major < 0){
		printk("Register char dev failed with %d\n", Major);
		return Major;
	}
	printk("Char dev with Major = %d was created\n", MAJOR_NUM);

	buffer = kmalloc(BUF_TEST_SIZE, GFP_KERNEL);
	if(!buffer){
		printk("Buffer allocation error.\n");
		return -1;
	}
	printk("Buffer allocated\n");
	buffer_filled = 0;
	calculation_done = 0;

	calc_timer.expires = jiffies + msecs_to_jiffies(TIMER_DELAY);
	calc_timer.function = __timer_handler;
	init_timer(&calc_timer);

	return 0;
}

static void __cleanup_module(void)
{
	del_timer_sync(&calc_timer);
	printk("Calculation timer is stoped\n");
	if(efd_ctx != NULL){
		eventfd_signal(efd_ctx, EFD_STOP_TEST_CMD);
		eventfd_ctx_put(efd_ctx);
		printk("Put eventfd context\n");
	}
	if(efd_ctx2 != NULL){
		eventfd_signal(efd_ctx2, EFD_STOP_TEST_CMD);
		eventfd_ctx_put(efd_ctx2);
		printk("Put eventfd context2\n");
	}

	release_buffer(buffer);
	printk("Buffer released.\n");

	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk("Char dev unregistered\n");
	printk("module exit\n");
}

module_init( __init_module);
module_exit( __cleanup_module);

MODULE_PARM_DESC(usr_pid, "Userspace pid to monitor");
module_param_call(usr_pid, __set_usr_pid, __get_usr_pid, NULL, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(usr_efd, "Userspace eventfd to monitor");
module_param_call(usr_efd, __set_usr_efd, __get_usr_efd, NULL, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(usr_efd2, "Userspace eventfd for cope done notification");
module_param_call(usr_efd2, __set_usr_efd2, __get_usr_efd2, NULL, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(cur_cmd, "Cmd to execute");
module_param_call(cur_cmd, __set_cur_cmd, __get_cur_cmd, NULL, S_IRUGO | S_IWUSR);

MODULE_AUTHOR("AM");

MODULE_LICENSE("GPL");
