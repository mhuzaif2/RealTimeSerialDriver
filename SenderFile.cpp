/*
   Project: Real Time Embedded Systems 
   Real Time Linux Kernel Module File for Sender End
   Description:
	1- Defines parameters for running periodic real time task (20 packages of data per second)
	2- Generates Random Bytes for the Data
	3- Puts the Required Header and Tail to the Data
	   i.e. Header: 'A''S''C''I', Tail: 0x01
   Author: Muhammad Umer Huzaifa
   Date: 05/12/2014
*/
#include <linux/kernel.h>	/* decls needed for kernel modules */
#include <linux/module.h>	/* decls needed for kernel modules */
#include <linux/version.h>	/* LINUX_VERSION_CODE, KERNEL_VERSION() */
/*
  Specific header files for RTAI, our flavor of RT Linux
 */
#include "rtai.h"		/* RTAI configuration switches */
#include "rtai_sched.h"		/* rt_set_periodic_mode(), start_rt_timer(),
				   nano2count(), RT_LOWEST_PRIORITY,
				   rt_task_init(), rt_task_make_periodic() */
#include "rtai_serial.h"
#include "rtai_fifos.h"
/*
  Some newer versions define RT_SCHED_LOWEST_PRIORITY instead, so we'll
  get that if necessary
 */
#if ! defined(RT_LOWEST_PRIORITY)
#if defined(RT_SCHED_LOWEST_PRIORITY)
#define RT_LOWEST_PRIORITY RT_SCHED_LOWEST_PRIORITY
#else
#error RT_SCHED_LOWEST_PRIORITY not defined
#endif
#endif
/*
  Some RTAI functions return standard Linux symbolic error codes, so
  we'll include them
 */
#include <linux/errno.h>	/* EINVAL, ENOMEM */
/*
  Linux kernel modules in kernel versions 2.4 and later are asked to
  state their license terms.  "GPL" is the usual, for software
  released under the Gnu General Public License. The Linux kernel
  module loader 'insmod' will complain if it's anything else.
 */
/*
  THIS SOFTWARE WAS PRODUCED BY EMPLOYEES OF THE U.S. GOVERNMENT AS PART
  OF THEIR OFFICIAL DUTIES AND IS IN THE PUBLIC DOMAIN.
  When linked into the Linux kernel the resulting work is GPL. You
  are free to use this work under other licenses if you wish.
*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,0)
MODULE_LICENSE("GPL");
#endif
static RT_TASK simulator_task;	/* we'll fill this in with our task */
static RTIME simulator_period_ns =50000000;   // nanoseconds, 20Hz timer period, in nanoseconds 
#define BAUD 115200  /* 38400 */
#define NUMBITS 8
#define STOPBITS 1
////////////////////////////////////////////
void simulator_function(int arg)
{
  unsigned int data_size = 367;
  unsigned char data_byte[data_size];  /* header : 4 byte, 81 data : 162 bytes, Tail: 1 byte*/
  int sum=0;
  unsigned short data_generator[1];
 printk("Start Simulator\n");
  printk("Data\n");
  data_byte[0] = 'A';  
  data_byte[1] = 'S';  
  data_byte[2] = 'C';  
  data_byte[3] = 'I';  
  data_byte[data_size-1] = 0x01;
 int i;
  int count = 0;
int sensordata=0;
  while(1){
/* Suggested changes */
for (i=4; i<=data_size-2;i=i+2)
		{
get_random_bytes(&data_generator[0],sizeof(unsigned short));
			data_byte[i] = (data_generator[0] >> 8) & 0xFF; 
		}
for(i=0; i<=data_size-1;i=i+1)
		{
			sum=sum+data_byte[i];
			//sensordata= (data_byte[i] << 8) + data_byte[i];
			//printk("Data[%d] :: %d, Package number = %d \n",i,data_byte[i],count);
		}
   printk("Data[0]=%d, Data[1]=%d Data[2]=%d Data[3]=%d Data[data_size-1]=%d \n",data_byte[0],data_byte[1],data_byte[2],data_byte[3],data_byte[data_size-1]);
    /*Write data through serial port*/
    rt_spwrite_timed(0, (void *)data_byte, data_size, DELAY_FOREVER);  /* This line make simulator period slow */
    /* Wait one period */
    rt_task_wait_period();
    count++;
	//if (count==8)
		//rtf_reset(1);
    printk("count = %d\n",count);
  }
  return;
}
///////////////////////////////////////////////////////////
int init_module(void)
{
  RTIME simulator_period_count;	/* requested timer period, in counts */
  RTIME timer_period_count;	/* actual timer period, in counts */
  int retval;			/* we look at our return values */
/* Open a FIFO */

  /* Open serial port */
  rt_spopen(COM1, BAUD, NUMBITS, STOPBITS,RT_SP_PARITY_NONE, 
RT_SP_NO_HAND_SHAKE, RT_SP_FIFO_DISABLE);
  rt_spclear_tx(COM1);
  /*
    Set up the timer to expire in pure periodic mode by calling
    void rt_set_periodic_mode(void);
    This sets the periodic mode for the timer. It consists of a fixed
    frequency timing of the tasks in multiple of the period set with a
    call to start_rt_timer. The resolution is that of the 8254
    frequency (1193180 hz). Any timing request not an integer multiple
    of the period is satisfied at the closest period tick. It is the
    default mode when no call is made to set the oneshot mode.
  */
  rt_set_periodic_mode();
  /*
    Start the periodic timer by calling
    RTIME start_rt_timer(RTIME period);
    This starts the timer with the period 'period' in internal count units.
    It's usually convenient to provide periods in second-like units, so
    we use the nano2count() conversion to convert our period, in nanoseconds,
    to counts. The return value is the actual period set up, which may
    differ from the requested period due to roundoff to the allowable
    chip frequencies.
    Look at the console, or /var/log/messages, to see the printk()
    messages.
  */
  simulator_period_count = nano2count(simulator_period_ns);
  timer_period_count = start_rt_timer(simulator_period_count);
  printk("periodic_task: requested %d counts, got %d counts\n",
	    (int) simulator_period_count, (int) timer_period_count);
  /*
    Initialize the task structure by calling
    rt_task_init(RT_TASK *task, void *rt_thread, int data, int stack_size,
    int priority, int uses_fpu, void *signal);
    This structure will be passed to rt_task_init() later to start the
    task.
    'task' is a pointer to an RT_TASK type structure whose space must
    be provided by the application. It must be kept during the whole
    lifetime of the real time task and cannot be an automatic
    variable.  'rt_thread' is the entry point of the task
    function. The parent task can pass a single integer value data to
    the new task.  'stack_size' is the size of the stack to be used by
    the new task. See the note below on computing stack size.
    'priority' is the priority to be given the task. The highest
    priority is 0, while the lowest is RT_LOWEST_PRIORITY (0x3fffFfff,
    or 1073741823).  'uses_fpu' is a flag. Nonzero value indicates
    that the task will use the floating point unit.  'signal' is a
    function that is called, within the task environment and with
    interrupts disabled, when the task becomes the current running
    task after a context switch.
    The newly created real time task is initially in a suspend
    state. It is can be made active either with
    rt_task_make_periodic(), rt_task_make_periodic_relative_ns() or
    rt_task_resume().
  */
  retval = rt_task_init(&simulator_task,	/* pointer to our task structure */
		 simulator_function, /* the actual timer function */
		 0,		/* initial task parameter; we ignore */
		 1024,		/* 1-K stack is enough for us */
		 RT_LOWEST_PRIORITY, /* lowest is fine for our 1 task */
		 0,		/* uses floating point; we don't */
		 0);		/* signal handler; we don't use signals */
  if (0 != retval) {
    if (-EINVAL == retval) {
      /* task structure is already in use */
      printk("periodic task: task structure already in use\n");
    } else if (-ENOMEM == retval) {
      /* stack could not be allocated */
      printk("periodic task: can't allocate stack\n");
    } else {
      /* unknown error */
      printk("periodic task: error initializing task structure\n");
    }
    return retval;
  }
  /*
    Start the task by calling
    int rt_task_make_periodic (RT_TASK *task, RTIME start_time, RTIME period);
    This marks the task 'task', previously created with
    rt_task_init(), as suitable for a periodic execution, with period
    'period'.  The time of first execution is given by start_time.
    start_time is an absolute value measured in clock
    ticks. After the first task invocation, it should call
    rt_task_wait_period() to reschedule itself.
   */
  retval = rt_task_make_periodic(&simulator_task, /* pointer to our task structure */
			  /* start one cycle from now */
			  rt_get_time() + simulator_period_count, 
			  simulator_period_count); /* recurring period */
  if (0 != retval) {
    if (-EINVAL == retval) {
      /* task structure is already in use */
      printk("periodic task: task structure is invalid\n");
    } else {
      /* unknown error */
      printk("periodic task: error starting task\n");
    }
    return retval;
  }
  return 0;
}
/*
  Every Linux kernel module must define 'cleanup_module', which takes
  no argument and returns nothing.
  The Linux kernel module installer program 'rmmod' will execute this
  function.
 */
void cleanup_module(void)
{
  int retval;
  retval = rt_task_delete(&simulator_task);
  if (0 !=  retval) {
    if (-EINVAL == retval) {
      /* invalid task structure */
      printk("periodic task: task structure is invalid\n");
    } else {
      printk("periodic task: error stopping task\n");
    }
  }
  rt_spclose(COM1);
  return;
}
