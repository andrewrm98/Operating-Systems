/* ki2.c
 * Andrew Morrison
 * Peter Christakos */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <asm-generic/current.h>
#include <asm/errno.h>

unsigned long **sys_call_table;
asmlinkage long (*ref_sys_cs3013_syscall2)(void);

struct ancestry 
{
  pid_t ancestors[10];
  pid_t siblings[100];
  pid_t children[100];
};

/************************* Our Code *****************************************/

asmlinkage long new_sys_cs3013_syscall2(unsigned short * target_pid, struct ancestry *response)
{ 

  struct task_struct * tree;
  struct task_struct * task;
  struct list_head * list;
  struct ancestry info;
  pid_t tpid;
  int i = 0;

  /* Gets the current task
   * Use task->comm for task name and task->pid for the task PID */
  //struct task_struct *task = current;
  task = pid_task(find_vpid(*target_pid), PIDTYPE_PID);

  /* Check if copy from user succeeds */
  if(copy_from_user(&tree, response, sizeof(response)))
  {
    return EFAULT;
  }

  // print the target's pid
  tpid = task_pid_nr(tree);
  printk(KERN_INFO "Target PID: %i\n", tpid);

  // get children
  i = 0;
  list_for_each(list, &tree->children)
  {
    task = list_entry(list, struct task_struct, sibling);
    printk(KERN_INFO "Child PID: %d\n", task->pid);
    info.children[i] = task->pid;
    i++;
  }

  // get siblings
  i = 0;
  list_for_each(list, &tree->sibling)
  {
  	task = list_entry(list, struct task_struct, sibling);
  	printk(KERN_INFO "Sibling PID: %d\n", task->pid);
  	info.siblings[i] = task->pid;
  	i++;
  }

  // get ancestors
  i = 0;
  while(task->pid > 1)
  {
  	task = task->real_parent;
  	printk(KERN_INFO "Parent PID: %d\n", task->pid);
  	info.ancestors[i] = task->pid;
  	i++;
  }

  /* Check if copy back to user succeeds */
  if(copy_to_user(response, &tree, sizeof(response)))
  {
  	return EFAULT;
  }

  return 1;
}

/************************* Supplied Code *****************************************/

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
	     (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
  */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
  
  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);