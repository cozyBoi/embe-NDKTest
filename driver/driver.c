#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/ioctl.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/irq.h>
#include <asm/system.h>

#include <mach/gpio.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>


static int inter_major=242, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;
static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg);

static inter_usage=0;
int interruptCount=0;

wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

#define IOM_FND_MAJOR 261        // ioboard fpga device major number
#define IOM_FND_NAME "fpga_fnd"        // ioboard fpga device name

#define IOM_FND_ADDRESS 0x08000004 // pysical address

static int fpga_fnd_port_usage = 0;
static unsigned char *iom_fpga_fnd_addr;
static struct cdev inter_cdev;
static dev_t inter_dev;
static int kernel_timer_usage = 0;

static struct struct_mydata {
    struct timer_list timer;
    int count;
};

struct struct_mydata mydata;
struct struct_mydata mydata2;

static int blink_cnt = 1;
// define functions...
int iom_fpga_driver_open(struct inode *minode, struct file *mfile);
int iom_fpga_driver_release(struct inode *minode, struct file *mfile);
int iom_fpga_driver_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
int fnd_write(unsigned int value[4]);


static struct file_operations inter_fops =
{
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};


static int EXITEXIT = 0;
static unsigned int fnd_value[4];
static int timer_init = 0;
static int first_push = 1;

int fnd_write(unsigned int _value[4]){
    unsigned int value[4];
    int i = 0;
    //print it reverse order
    //because
    //arr -> [0] [1] [2] [3]
    //fnd -> [3] [2] [1] [0]
    for(i = 0; i < 4; i++){
        value[i] = _value[3-i];
    }
    unsigned short int value_short = 0;
    
    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)iom_fpga_fnd_addr);
    
    return 0;
}

static int inter_open(struct inode *minode, struct file *mfile){
	int ret;
	int irq;

    init_timer(&(mydata.timer));
    init_timer(&(mydata2.timer));
    
	printk(KERN_ALERT "Open Module\n");

	// int1
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler1, IRQF_TRIGGER_FALLING, "home", 0);

	// int2
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler2, IRQF_TRIGGER_FALLING, "back", 0);

	// int3
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler3, IRQF_TRIGGER_FALLING, "volup", 0);

    gpio_direction_input(IMX_GPIO_NR(5,14));
    irq = gpio_to_irq(IMX_GPIO_NR(5,14));
    printk(KERN_ALERT "IRQ Number : %d\n",irq);
    request_irq (irq, inter_handler4, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "vol_down", 0);
    
    fpga_fnd_port_usage = 1;
    kernel_timer_usage = 1;
    
	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
    del_timer_sync(&mydata2.timer);
    del_timer_sync(&mydata.timer);
    
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	
    fpga_fnd_port_usage = 0;
    kernel_timer_usage = 0;
    
    EXITEXIT = 0;
    int i = 0;
    for(i = 0; i < 4; i++) fnd_value[i] = 0;
    fnd_write(fnd_value);
    timer_init = 0;
    first_push = 1;
    prev = 1;
    ENDENDEND = 0;
    blinking_cnt = 0;
    
    printk(KERN_ALERT "Release Module\n");
	return 0;
}

static int inter_read(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
    if (push_sw_buff[0] == 1 && prev_push_sw_buff[0] != 1) {
        Text_mode = ~Text_mode;
        push_sw_buff[0] = 0;
    }
    else if (push_sw_buff[1] == 1) {
        Clock_FND_set_to_borad_time();
        push_sw_buff[1] = 0;
    }
    else if (push_sw_buff[2] == 1 && Text_mode) {
        clock_plus_hour();
        printf("plus hour\n");
        push_sw_buff[2] = 0;
    }
    else if (push_sw_buff[3] == 1 && Text_mode) {
        clock_plus_minute();
        printf("plus minute\n");
        push_sw_buff[3] = 0;
    }
	printk("write\n");
	return 0;
}

static int inter_register_cdev(void)
{
	int error;
	if(inter_major) {
		inter_dev = MKDEV(inter_major, inter_minor);
		error = register_chrdev_region(inter_dev,1,"stopwatch");
	}else{
		error = alloc_chrdev_region(&inter_dev,inter_minor,1,"stopwatch");
		inter_major = MAJOR(inter_dev);
	}
	if(error<0) {
		printk(KERN_WARNING "inter: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", inter_major);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error)
	{
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}
	return 0;
}

static int __init inter_init(void) {
	int result;
	if((result = inter_register_cdev()) < 0 )
		return result;
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/inter, Major Num : 242 \n");
    
    ENDENDEND = 0;
    iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	return 0;
}

static void __exit inter_exit(void) {
    iounmap(iom_fpga_fnd_addr);
    cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");
