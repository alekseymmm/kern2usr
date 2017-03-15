# kern2usr
Interaction between kernelspace and userspace

Start testing:

1. Go to project directory
$ cd kern2usr

2. Run userspace part
$ ./Debug/KernUsr

Expected output:

Eventfd created efd=3 pid=5426
Eventfd created efd2=4 pid=5426
Start polling...
timeout in polling

3. Go to kernel part directory and insert module
$ cd kern
$ insmod kern.ko

Expected output in /var/log/messages
kernel: [72254.308388] module started...
kernel: [72254.308393] Char dev with Major = 239 was created
kernel: [72254.308394] Buffer allocated
kernel: [72254.308483] Work queue created.

4. Send process pid  and both eventfd to kernel module pararmeters
$ echo 5426 > /sys/module/kern/parameters/usr_pid
$ echo 3 > /sys/module/kern/parameters/usr_efd
$ echo 4 > /sys/module/kern/parameters/usr_efd2

5. Create char dev
$ ../scripts/create_chdev.sh

6. Send command to find eventfd descriptors
$ echo 1 > /sys/module/kern/parameters/cur_cmd

Output:
kernel: [  119.313422] Received from userspace: usr_pid=5426, efd=3, efd2=4
kernel: [  119.313426] Resolved pointer to the userspace program task struct: ffff880feeff4380
kernel: [  119.313428] Resolved pointer to the userspace program's eventfd's file struct: ffff880ff0d66900 and struct2 : ffff880ff0d66100
kernel: [  119.313431] Resolved pointer to the userspace program's eventfd's context: ffff88101861d240 and context2 : ffff88101861d5c0

7. Send command to perform mmap
$ echo 2 > /sys/module/kern/parameters/cur_cmd

Output:
In handling_polling value in eventfd = 2
Got mmap command in polling eventfd
Char dev /dev/char/mmaptest opened 
Memory from char dev = 5 mmaped to 0x7f75cedb4000
handling_polling done!

8. Send command to start testing
$ echo 3 > /sys/module/kern/parameters/cur_cmd

Output:
kernel: [  242.747208] Timer is started.
kernel: [  242.747812] Timer handler called
kernel: [  242.747814] In handler: buffer_filled = 0, calculation_done = 0
kernel: [  242.747815] Start monitoring for efd2...
kernel: [  242.747861] Buffer filled
kernel: [  242.747946] Time to copy = 82233
kernel: [  242.747947] Exit from __monitor_efd2

9. Send command to stop timer and stop testing
$ echo 4 > /sys/module/kern/parameters/cur_cmd

Output:
kernel: [  260.714262] Timer is deleted.

10. Send command to stop userspace polling and exit 
$ echo 5 > /sys/module/kern/parameters/cur_cmd

Output:
In handling_polling value in eventfd = 5
Stop polling cmd received
handling_polling done!
done!

11. Unload module 
$ rmmod kern.ko
