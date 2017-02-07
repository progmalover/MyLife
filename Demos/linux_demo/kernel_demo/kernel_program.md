### Reference?

1. <linux kernel development>


### important kernel program points?

1. Locks(details refer to <1>): 
- atomic(int atomic <asm/atomic.h>, bit atomic <asm/bitops.h>);
- spin lock(spinlock <linux/spinlock.h>, rw lock is also spin lock and have try lock, this rw lock is also used in user space), this is kernel feature.
- semaphore(<asm/semaphore.h>, same as ovs and user sapce semaphore, semaphore is sleep lock), semaphore operation is DOWN/UP;
- completion variable(<linux/completion.h>).

For C program in user space, it's basically same. User space ovs lock usage, refer to (https://app.yinxiang.com/shard/s28/nl/5668999/3a09925c-0628-48a9-aa27-d63e2357e407). In ovs, it's mutex, rwlock, atomic, barrier, condition.

For C++/Java, thay have wrappered these and add some new feature like future. And third-party library also added.

2. systemcall ???

3. memory:
- kernel has only 4K run time stack, so do not create large veriable, and do not use large param, refer to <1>. So use dynamic alloc like kmalloc is neccerry.
- kmalloc(<linux/slab.h>), refer to <1>. For flag, refer <linux/slab.h> <linux/gfp.h>.
- kfree(<linux/slab.h>).
- vmalloc(<linux/vmalloc.h>), when you need large memory, and there is no large physical continuious memory, use this, refer to <1>. But vmalloc has side effect.
- vfree <linux/vmalloc.h>.
- slab <slab.h>.

At last, summary:
- If you need continous physical memory, use kmalloc(size, GFP_ATOMIC/GFP_KERNEL).
- If you need continous virtual memory, use vmalloc, but try not use this, this has perfomance lost.
- If you need large memory and frequent need, use slab api.

4. KO module:
refer to <1>.

5. debug:
- printk(KERN_INFO, "") in <linux/kernel.h> .
- Configures while compiling kernel, like CONFIG_DEBUG_KERNEL in Kernel Hacking menu while compiling. More refer to <1>.
- SysRq.

