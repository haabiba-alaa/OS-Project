#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
static void* current_position=NULL;
static struct FrameInfo *frames =NULL;
int heapPages[((KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE)] = {0};
uint32 freeSpaceVA = KERNEL_HEAP_START;
int32 heapContent[((KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE)]={0};

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
 {
 	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
 	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
 	//All pages in the given range should be allocated
 	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
 	//Return:
 	//	On success: 0
 	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

 	//Comment the following line(s) before start coding...
 	//panic("not implemented yet");
 	struct FrameInfo * frames=NULL;

	 start = daStart;
	 initSizeToAllocate =ROUNDUP(initSizeToAllocate,PAGE_SIZE);
	 segment_break = ROUNDUP(start + initSizeToAllocate,PAGE_SIZE);
	 limit = ROUNDUP(daLimit, PAGE_SIZE);
	 cprintf("Check\n");
	 if (segment_break > daLimit)
	 {
		 return E_NO_MEM;
	 }
	 cprintf("Valid\n");
	 uint32 numPages =initSizeToAllocate / PAGE_SIZE;

	 for (uint32 i = 0; i < numPages; i++)
	 {
		 uint32 pageAddr = start + (i * PAGE_SIZE);
		 int x = allocate_frame(&frames);
		 if(x==0)
		 {
		  int y =map_frame(ptr_page_directory,frames,pageAddr,PERM_WRITEABLE);
			 if(y!=0)
			 {
				 return E_NO_MEM;
			 }
		 }
	 }
	 cprintf("Finish\n");
	 initialize_dynamic_allocator(start, initSizeToAllocate);
	 cprintf("Finish 2\n");
	 return 0;
}


void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */
	//MS2: COMMENT THIS LINE BEFORE START CODING====
	//return (void*)-1 ;
	//panic("not implemented yet");
	if(increment<0){
			increment=ROUNDDOWN(increment,PAGE_SIZE);
		}
		else if(increment>0){
			increment=ROUNDUP(increment,PAGE_SIZE);
		}
	    uint32 current_break =(uint32) segment_break;
	    uint32 Prev_break = (uint32) current_break;
	    uint32 new_break = (uint32) ROUNDUP((current_break + increment), PAGE_SIZE);
	    segment_break = (uint32)new_break;

	    if (new_break > limit) {
	        panic("segment break exceeds hard limit");
	    }

	   // cprintf("sbrk: Requested increment: %d, Current Break: %x, New Break: %x\n", ROUNDUP(increment,PAGE_SIZE), (void*)current_break, (void*)new_break);

	    // Case: Incrementing the heap size
	    if (increment > 0) {
	    	struct FrameInfo* frames = NULL;
	        while (current_break <= new_break) {
	        	frames=NULL;
	            int frame = allocate_frame(&frames);
	            if (frame != 0) {
	                panic("out of memory");
	            }
	            map_frame(ptr_page_directory, frames, current_break, PERM_WRITEABLE);
	            //cprintf("sbrk: Mapped frame at %p\n", (void*)current_break);
	            current_break = (current_break + PAGE_SIZE);
	        }
	       // cprintf("sbrk: Heap size increased\n");
	        return (void*)Prev_break;
	    } else if (increment < 0) {
	        while (current_break >= new_break) {
	            unmap_frame(ptr_page_directory, (uint32)current_break);
	        //    cprintf("sbrk: Unmapped frame at %p\n", (void*)current_break);
	            current_break = (current_break - PAGE_SIZE);
	        }
	       // cprintf("sbrk: Heap size decreased\n");
	        return (void*)new_break;
	    } else {
	      //  cprintf("sbrk: No change in heap size\n");
	        return (void*)current_break;
	    }
}

void* kmalloc(unsigned int size) {
    if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
        void* result = alloc_block_FF(size);
        cprintf("allocFF: Allocated block at %p, Size: %u\n", result, size);
        return result;
    }

    size = ROUNDUP(size, PAGE_SIZE);
    uint32 numPages = size / PAGE_SIZE;
    cprintf("kmalloc:required Size = %u\n",size);
    uint32 address = 0;
    uint32 addressEnd = 0;
    uint32 finalAddress = 0;
    uint32 hard_limit = limit + PAGE_SIZE;
    uint32 kernPages = (KERNEL_HEAP_MAX - hard_limit) / PAGE_SIZE;
    uint32 startIndex = (hard_limit-KERNEL_HEAP_START)/PAGE_SIZE;
    //startIndex++;
    uint32 pageCounter = 0;

    if (kernPages > numPages && isKHeapPlacementStrategyFIRSTFIT()) {
        for (uint32 i = startIndex; i < kernPages; i++) {
            if (heapPages[i] == 0) {
                pageCounter++;
                if (pageCounter == 1) {
                    address = KERNEL_HEAP_START + (i * PAGE_SIZE);//changed line
                    finalAddress = address;
                }
            } else {
                pageCounter = 0;
            }

            if (pageCounter == numPages) {
                break;
            }
        }

        if (pageCounter == numPages) {
            addressEnd = address + (numPages * PAGE_SIZE);
            for (int i = 0; i < numPages; i++) {
                struct FrameInfo* fptr = NULL;
                int success = allocate_frame(&fptr);
                if (success != 0) {
                    return NULL;
                }
                int map_num = map_frame(ptr_page_directory, fptr, address, PERM_WRITEABLE);
                fptr->va=address;
                heapPages[(address - KERNEL_HEAP_START) / PAGE_SIZE] = numPages;//changed line
                address += PAGE_SIZE;
//                cprintf("kmalloc: Mapped frame at %p\n", (void*)address);
            }
//            cprintf("kmalloc: Allocated block at %p, Size: %u\n", (void*)finalAddress, size);
            return (void*)finalAddress;
        }
        else {
            cprintf("kmalloc: Not enough consecutive free pages. Requested: %u, Available: %u\n", size, pageCounter);
            return NULL;
        }
    }
    else {

        cprintf("kmalloc: Not enough free pages. Requested: %u, Available: %u\n", numPages, kernPages);
        cprintf("kmalloc: Size of block requested: %u, Number of free pages available: %u\n", numPages, kernPages);
        cprintf("kmalloc: Size: %u, kernSize: %u, start: %u, segment_break: %u, limit: %u\n", numPages, kernPages, start, segment_break, hard_limit);

        return NULL;
    }
}
void kfree(void* virtual_address) {
    uint32 hard_limit = limit + PAGE_SIZE;
    uint32 va = (uint32)virtual_address;

    cprintf("kfree: Attempting to free address %p\n", virtual_address);

    if (va >= KERNEL_HEAP_START && va <= limit) {
        // Case: Freeing memory within Block Allocator range
        cprintf("kfree: Freeing block at %p within Block Allocator range\n", virtual_address);
        free_block(virtual_address);
        cprintf("kfree: Freed block at %p\n", virtual_address);
    } else if (va >= hard_limit && va <= KERNEL_HEAP_MAX) {
        // Case: Freeing memory within Page Allocator range
        uint32 page_address = ROUNDDOWN(va, PAGE_SIZE);
        cprintf("kfree: Freeing space at %p within Page Allocator range (Page Address: %p)\n", virtual_address, (void*)page_address);
        uint32 startIndex = (va - KERNEL_HEAP_START) / PAGE_SIZE;

        if(heapPages[startIndex]!=0){
        	uint32 numPages= heapPages[startIndex];
        	for(uint32 i=0;i<numPages;i++){
        		int index = startIndex + i;
				if (heapPages[index] != 0)

				{
					va = KERNEL_HEAP_START + (index * PAGE_SIZE);
					unmap_frame(ptr_page_directory, va);
					heapPages[index] = 0;
				}
        	}
        }


        cprintf("kfree: Freed space at %p\n", virtual_address);
    } else {
        // Case: Invalid address
        // Consider handling this case more gracefully before panicking
        panic("kfree: Invalid address");
    }
}

unsigned int kheap_virtual_address(unsigned int physical_address) {
    // Check if the physical address is within the block allocator limit
	struct FrameInfo* ptr_frame_info =NULL;
	ptr_frame_info = to_frame_info(physical_address);
	if(ptr_frame_info!=NULL){
		cprintf("kfree: Freed space at %p\n",ptr_frame_info->va);
		return ptr_frame_info->va;
	}
   return 0;
}


unsigned int kheap_physical_address(unsigned int virtual_address) {
   return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
