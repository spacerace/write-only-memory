#ifndef __WOM_H__
#define __WOM_H__

#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <asm/io.h>

#define SUCCESS		0

#define DEVICE_NAME	"wom-25120"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 	192		/* length of the receive/send buffer, max 192 byte = 191 chars + trailing zero */
#define DRIVER_AUTHOR	"2013 Nils Stec \"krumeltee\" <nils.stec@gmail.com>"
#define DRIVER_DESC	"signetics WOM-25120 driver"

static int device_open(struct inode *inode, struct file *file); 
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset);

#endif
