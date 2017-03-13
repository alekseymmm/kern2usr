/*
 * fops_chdev.h
 *
 *  Created on: 13 мар. 2017 г.
 *      Author: root
 */

#include "kern.h"

#ifndef KERN_FOPS_CHDEV_H_
#define KERN_FOPS_CHDEV_H_

ssize_t mmaptest_read(struct file *filep, char *buffer, size_t len, loff_t *offset);
int mmaptest_mmap(struct file *filp, struct vm_area_struct *vma);
int mmaptest_open(struct inode *inode, struct file *filp);
int mmaptest_release(struct inode *inode, struct file *filp);
long mmaptest_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);
unsigned int mmaptest_poll(struct file *file, struct poll_table_struct *pwait);

#endif /* KERN_FOPS_CHDEV_H_ */
