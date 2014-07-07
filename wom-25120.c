/* 
 * wom.c
 * 
 * an implementation of signetics famous write-only-memory as a kernel module
 * 
 * * LAST CHANGE: 15. feb 2013, Nils Stec
 * 
 *  Authors:    Nils Stec, "krumeltee", <nils.stec@gmail.com>, (c) 2013
 *              some module parts are by LKMPG                             - taken from version "2007-05-18 ver 2.6.4"
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <asm/io.h>

#include "wom-25120.h"

static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open? */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/*  Called when a process tries to open the device file */
static int device_open(struct inode *inode, struct file *file) {
	if(Device_Open) return -EBUSY;
	Device_Open++;
	
	sprintf(msg, "write only memory does not support read operations, all your data is lost.\n");

	msg_Ptr = msg;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off) {
	char message_from_user[BUF_LEN];
	
	if(copy_from_user(message_from_user, buff, (len < BUF_LEN) ? len : BUF_LEN)) return -EINVAL;
	message_from_user[4] = '\0';
	printk(KERN_INFO "[signetics wom-25120] discarded %dbytes of data\n", (int)len);	
	return len;
}


/** called when module loaded */
int init_module_wom(void) {
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "[signetics wom-25120] registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "[signetics wom-25120] driver loaded with major %d\n", Major);
	printk(KERN_INFO "[signetics wom-25120] >> $ mknod /dev/%s c %d 0\n", DEVICE_NAME, Major);
	
	return SUCCESS;
}

/** called when module unloaded */
void cleanup_module_wom(void) {
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "[signetics wom-25120] DRIVER UNLOADED\n");
}

/* Called when a process closes the device file. */
static int device_release(struct inode *inode, struct file *file) {
	Device_Open--;		/* We're now ready for our next caller */
				/* Decrement the usage count, or else once you opened the file, you'll never get get rid of the module. */
	module_put(THIS_MODULE);
	return 0;
}

/* Called when a process, which already opened the dev file, attempts to read from it. */
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset) {
	int bytes_read = 0;
	
	if(*msg_Ptr == 0) return 0;
	
	while(length && *msg_Ptr) {
		put_user(*(msg_Ptr++), buffer++);
		length--;
		bytes_read++;
	}
	
	return bytes_read;
}

module_init(init_module_wom);
module_exit(cleanup_module_wom);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("signetics wom-25120");
