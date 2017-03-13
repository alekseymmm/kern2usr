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

#include "kern.h"

char *buffer, *tmp_buffer;
int buffer_filled = 0;
int calculation_done = 0;
static int Major  ;

struct timer_list calc_timer;
wait_queue_head_t wq_buffer; //wait till the buffer filled

long long int start_time = 0, stop_time = 0;
long long int total_time = 0;

long long int kstart_time = 0, kstop_time = 0;
long long int ktotal_time = 0;

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
	wake_up_interruptible(&wq_buffer);
	printk("Buffer filled\n");
	start_time = ktime_to_ns(ktime_get());
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
//static char *test_str = "Test string from module\n";

static ssize_t mmaptest_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
//   int error_count = 0;
//   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
//   error_count = copy_to_user(buffer, test_str, strlen(test_str));
//
//   if (error_count == 0){            // if true then have success
//      printk("Char dev:  Sent %d characters to the user\n", (int)strlen(test_str));
//      return 0;  // clear the position to the start and return 0
//   }
//   else {
//      printk("Char dev failed to send %d characters to the user\n", error_count);
//      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
//   }
	return 0;
}
static int mmaptest_mmap(struct file *filp, struct vm_area_struct *vma)
{

	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);

	printk("mmaptest_mmap called...\n");

	//if userspace tries to mmap beyond end of the buffer
	if(size > BUF_SIZE){
		printk("Error: try to mmap beyond buffer length.\n");
		return -EINVAL;
	}

	printk("vma->vm_pgoff = %lu\n", vma->vm_pgoff);
	printk("vma->vm_start = %lu\n", vma->vm_start);
	printk("vma->vm_end = %lu\n", vma->vm_end);

	vma->vm_page_prot.pgprot |= VM_WRITE;
	printk("vma->vm_page_prot = %lu\n", vma->vm_page_prot.pgprot);
//	if(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
//			vma->vm_end - vma->vm_start, vma->vm_page_prot))
	if(remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)buffer) >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start, vma->vm_page_prot ))
		return -EAGAIN;

	return 0;
}

static int mmaptest_open(struct inode *inode, struct file *filp)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int mmaptest_release(struct inode *inode, struct file *filp)
{
	module_put(THIS_MODULE);
	return 0;
}
static long mmaptest_ioctl(struct file *file,
		unsigned int ioctl_num, unsigned long ioctl_param)
{
	switch (ioctl_num){
	case IOCTL_SET_MSG:
		calculation_done = 1;
		stop_time = ktime_to_ns(ktime_get());
		total_time += stop_time - start_time;
		printk("got msg from user space in ioctl. Msg = %lu\n", ioctl_param);

		//print_buf(buffer, BUF_TEST_SIZE);
		printk("Calculcation done!!!\n");
		printk("Time for this calculation(usr)= %lld\n", stop_time - start_time);
		printk("Total time (usr)= %lld\n", total_time);

		//memcpy(tmp_buffer, buffer, BUF_SIZE);
		kstart_time = ktime_to_ns(ktime_get());
		//kernel_fpu_begin();
		//GF8_Calculation_2s(tmp_buffer, &dsc, 0, 0);
		//kernel_fpu_end();
		kstop_time = ktime_to_ns(ktime_get());
		ktotal_time += kstop_time - kstart_time;

		printk("Time for this calculation(kern)= %lld\n", kstop_time - kstart_time);
		printk("Total time (kern)= %lld\n", ktotal_time);

		break;
	case IOCTL_GET_MSG:
		printk("set msg for user space in ioctl. Msg = %lu\n", ioctl_param);
		break;
	}
	return 0;
}

static unsigned int mmaptest_poll(struct file *file, struct poll_table_struct *pwait)
{
	unsigned int mask = 0;

	poll_wait(file, &wq_buffer, pwait);
	//printk("In poll: buffer_filled = %d, calculation_done = %d\n", buffer_filled, calculation_done);

	if(buffer_filled && !calculation_done){
		printk("Mask assign to POLLIN\n");
		mask |= POLLIN;
	}
	return mask;
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

	if(!buffer_filled || calculation_done){
		fill_buffer(buffer, BUF_TEST_SIZE);
		//print_buf(buffer, BUF_TEST_SIZE);
	}
	printk("In handler: buffer_filled = %d, calculation_done = %d\n", buffer_filled, calculation_done);
	mod_timer(&calc_timer, jiffies + msecs_to_jiffies(3000));
}


static int __init_module ( void )
{
	printk("module started...\n");

	Major = register_chrdev(0, DEVICE_NAME, &mmaptest_fops);
	if(Major < 0){
		printk("Register char dev failed with %d\n", Major);
		kfree(buffer);
		return Major;
	}
	printk("Char dev with Major = %d was created\n",Major);

	buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
	tmp_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);

	printk("Buffer allocated\n");
	buffer_filled = 0;

	init_waitqueue_head(&wq_buffer);
	printk("Wait queue initialized.\n");

	calc_timer.expires = jiffies + msecs_to_jiffies(5000);
	calc_timer.function = __timer_handler;
	init_timer(&calc_timer);
//	add_timer(&calc_timer);

//	printk("Calc timer started!\n");

	return 0;
}

static void __cleanup_module(void)
{
	del_timer_sync(&calc_timer);
	printk("Calculation timer is stoped\n");

	release_buffer(buffer);
	kfree(tmp_buffer);
	printk("Buffer released.\n");

	unregister_chrdev(Major, DEVICE_NAME);
	printk("Char dev unregistered\n");
	printk("module exit\n");
}

module_init( __init_module);
module_exit( __cleanup_module);


MODULE_AUTHOR("AM");

MODULE_LICENSE("GPL");
