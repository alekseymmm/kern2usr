/*
 * fops_chdev.c
 *
 *  Created on: 13 мар. 2017 г.
 *      Author: root
 */

#include <linux/module.h>
#include <linux/kernel.h>
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

//extern wait_queue_head_t wq_buffer; //wait till the buffer filled;
extern char *buffer;
extern int buffer_filled;
extern int calculation_done;

extern long long int start_time , stop_time ;
extern long long int total_time ;

extern long long int kstart_time , kstop_time ;
extern long long int ktotal_time ;

ssize_t mmaptest_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
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
int mmaptest_mmap(struct file *filp, struct vm_area_struct *vma)
{

	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);

	printk("mmaptest_mmap called...\n");

	printk("vma->vm_pgoff = %lu\n", vma->vm_pgoff);
	printk("vma->vm_start = %lu\n", vma->vm_start);
	printk("vma->vm_end = %lu\n", vma->vm_end);
	printk("mmap size = %lu, BUF_TEST_SIZE = %d\n", size, BUF_TEST_SIZE);

	//if userspace tries to mmap beyond end of the buffer
	if(size > BUF_TEST_SIZE){
		printk("Error: try to mmap beyond buffer length.\n");
		return -EINVAL;
	}

	vma->vm_page_prot.pgprot |= VM_WRITE;
	printk("vma->vm_page_prot = %lu\n", vma->vm_page_prot.pgprot);
//	if(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
//			vma->vm_end - vma->vm_start, vma->vm_page_prot))
	if(remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)buffer) >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start, vma->vm_page_prot ))
		return -EAGAIN;

	return 0;
}

int mmaptest_open(struct inode *inode, struct file *filp){
	try_module_get(THIS_MODULE);
	return 0;
}

int mmaptest_release(struct inode *inode, struct file *filp){
	module_put(THIS_MODULE);
	return 0;
}

long mmaptest_ioctl(struct file *file,
		unsigned int ioctl_num, unsigned long ioctl_param){
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

unsigned int mmaptest_poll(struct file *file, struct poll_table_struct *pwait)
{
	unsigned int mask = 0;

	//poll_wait(file, &wq_buffer, pwait);
	//printk("In poll: buffer_filled = %d, calculation_done = %d\n", buffer_filled, calculation_done);

	if(buffer_filled && !calculation_done){
		printk("Mask assign to POLLIN\n");
		mask |= POLLIN;
	}
	return mask;
}
