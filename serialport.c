#include <linux/init.h>
//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
//#include <asm/system.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

int serialport_open(struct inode *inode,struct file *filp);
int serialport_release(struct inode *inode,struct file *filp);
ssize_t serialport_read(struct file *filp,char *buf,size_t count,loff_t *f_pos);
ssize_t serialport_write(struct file *filp,char *buf,size_t count, loff_t *f_pos);
void serialport_exit(void);
int serialport_init(void);

struct file_operations serialport_fops = {
      read: serialport_read,
      write: serialport_write,
      open: serialport_open,
      release: serialport_release
};

//Driver globals
//Major port number - check if it is free before use.
int serialport_major = 65;
//Change the Base Address to the right one provided in the virtual box before use.
int BASE_ADDRESS = 0x379;
int port;

module_init(serialport_init);
module_exit(serialport_exit);

int serialport_init(void) {
  int result;
  //Try to register the driver with the "serialport_major" port number.
  result = register_chrdev(serialport_major,"serialport",&serialport_fops);
  if(result<0) {
    printk("<1>serialport: cannot obtain major number %d\n",serialport_major);
    return result;
  }

  //Check if the region can be reserved. We need only 1 byte from the BASE_ADDRESS.
  port = check_region(0x378,1);
  if(port) {
    printk("<1> serialport: cannot reserve %d\n",BASE_ADDRESS);
    result = port;
    goto fail;
  }
  request_region(0x378,1,"serialport");

fail:
  serialport_exit();
  return result;
}

//Exit Module - Nothing much here. We just un-register the device registered by "register_chrdev" and then release the port
void serialport_exit(void) {
  unregister_chrdev(serialport_major,"serialport");
  if(!port) {
    release_region(BASE_ADDRESS,1);
  }
}

int serialport_open(struct inode *inode,struct file *filp) {

  return 0;
}

int serialport_release(struct inode *inode,struct file *filp) {

  return 0;
}

ssize_t serialport_read(struct file *filp,char *buf,size_t count, loff_t *f_pos) {

  char serialport_buffer;

  //Read from device and store in buffer.
  serialport_buffer = inb(BASE_ADDRESS);
//Copy to user space.
  copy_to_user(buf,&serialport_buffer,1);
  //Change reading position
  if(*f_pos == 0) {
    *f_pos+=1;
    return 1;
  } else {
    return 0;
  }
}

ssize_t serialport_write(struct file *filp,char *buf,size_t count, loff_t *f_pos) {
 char *tmp;
 //Buffer writing to device
 char serialport_buffer;

 tmp = buf+count-1;
 copy_from_user(&serialport_buffer,tmp,1);

 return 1;
}
