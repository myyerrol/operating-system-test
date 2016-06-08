/*
 * Segmentation-based user mode implementation
 * Copyright (c) 2001,2003 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.23 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/defs.h>
#include <geekos/mem.h>
#include <geekos/string.h>
#include <geekos/malloc.h>
#include <geekos/int.h>
#include <geekos/gdt.h>
#include <geekos/segment.h>
#include <geekos/tss.h>
#include <geekos/kthread.h>
#include <geekos/argblock.h>
#include <geekos/user.h>
#include <geekos/errno.h>

/* ----------------------------------------------------------------------
 * Variables
 * ---------------------------------------------------------------------- */

#define DEFAULT_USER_STACK_SIZE 8192


/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */


/*
 * Create a new user context of given size
 */

/* TODO: Implement
static struct User_Context* Create_User_Context(ulong_t size)
*/
static struct User_Context* Create_User_Context(ulong_t size)
{
    struct User_Context* userContext;
    int index;
    // Size must be a multiple of the page size.
    size = Round_Up_To_Page(size);

    // Allocate memory for the user context.
    Disable_Interrupts();
    userContext = (struct User_Context *)Malloc(sizeof(struct User_Context));

    if (userContext != 0) {
	    userContext->memory = Malloc(size);
    }

    Enable_Interrupts();

    if (userContext == 0 || userContext->memory == 0) {
        goto fail;
    }

    // Fill user memory with zeroes, leaving it uninitizlized is a potenital
    // security flaw.
    memset(userContext->memory, '\0', size);
    userContext->size = size;
    // Allocate an LDT descriptor for the uesr context.
    userContext->ldtDescriptor = Allocate_Segment_Descriptor();

    if(userContext->ldtDescriptor == 0) {
        goto fail;
    }

    Init_LDT_Descriptor(userContext->ldtDescriptor, userContext->ldt,
        NUM_USER_LDT_ENTRIES);
    index = Get_Descriptor_Index(userContext->ldtDescriptor);
    userContext->ldtSelector = Selector(KERNEL_PRIVILEGE, true, index);
    // Initialize code and data segments within the LDT.
    Init_Code_Segment_Descriptor(&userContext->ldt[0],
                                (ulong_t)userContext->memory,
	                             size / PAGE_SIZE,
                                 USER_PRIVILEGE);
    Init_Data_Segment_Descriptor(&userContext->ldt[1],
                                (ulong_t)userContext->memory,
	                             size / PAGE_SIZE,
                                 USER_PRIVILEGE);
    userContext->csSelector = Selector(USER_PRIVILEGE, false, 0);
    userContext->dsSelector = Selector(USER_PRIVILEGE, false, 1);
    // Nobody is using this user context yet.
    userContext->refCount = 0;
    // Successfully !
    return userContext;

fail:
    // We failed, release any allocated memory.
    Disable_Interrupts();

    if (userContext != 0) {
    	if (userContext->memory != 0) {
    	    Free(userContext->memory);
        }
        Free(userContext);
    }

    Enable_Interrupts();
    return 0;
}


static bool Validate_User_Memory(struct User_Context* userContext,
    ulong_t userAddr, ulong_t bufSize)
{
    ulong_t avail;

    if (userAddr >= userContext->size)
        return false;

    avail = userContext->size - userAddr;
    if (bufSize > avail)
        return false;

    return true;
}

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * Destroy a User_Context object, including all memory
 * and other resources allocated within it.
 */
void Destroy_User_Context(struct User_Context* userContext)
{
    /*
     * Hints:
     * - you need to free the memory allocated for the user process
     * - don't forget to free the segment descriptor allocated
     *   for the process's LDT
     */
    KASSERT(userContext->refCount == 0);
    // Free the context's LDT descriptor.
    Disable_Interrupts();
    Free_Segment_Descriptor(userContext->ldtDescriptor);
    // Free the context's memory.
    Free(userContext->memory);
    Free(userContext);
    Enable_Interrupts();
}

/*
 * Load a user executable into memory by creating a User_Context
 * data structure.
 * Params:
 * exeFileData - a buffer containing the executable to load
 * exeFileLength - number of bytes in exeFileData
 * exeFormat - parsed ELF segment information describing how to
 *   load the executable's text and data segments, and the
 *   code entry point address
 * command - string containing the complete command to be executed:
 *   this should be used to create the argument block for the
 *   process
 * pUserContext - reference to the pointer where the User_Context
 *   should be stored
 *
 * Returns:
 *   0 if successful, or an error code (< 0) if unsuccessful
 */
int Load_User_Program(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat, const char *command,
    struct User_Context **pUserContext)
{
    /*
     * Hints:
     * - Determine where in memory each executable segment will be placed
     * - Determine size of argument block and where it memory it will
     *   be placed
     * - Copy each executable segment into memory
     * - Format argument block in memory
     * - In the created User_Context object, set code entry point
     *   address, argument block address, and initial kernel stack pointer
     *   address
     */
    int i;
    ulong_t maxva = 0;
    unsigned numArgs;
    ulong_t argBlockSize;
    ulong_t virSize, argBlockAddr;
    struct User_Context *userContext = 0;

    // Find maximun virtual address.
    for (i = 0; i < exeFormat->numSegments; ++i) {
        struct Exe_Segment *segment = &exeFormat->segmentList[i];
        ulong_t topva = segment->startAddress + segment->sizeInMemory;
        if (topva > maxva) {
            maxva = topva;
        }
    }

    // Determine size required for argument bolck.
    Get_Argument_Block_Size(command, &numArgs, &argBlockSize);
    // Now we can determint the size of the memory block needed ro run the
    // process.
    virSize = Round_Up_To_Page(maxva) + DEFAULT_USER_STACK_SIZE;
    // Create User_Context.
    argBlockAddr = virSize;
    virSize += argBlockSize;
    userContext = Create_User_Context(virSize);

    if (userContext == 0) {
        return -1;
    }

    // Load segment data into memory.
    for (i = 0; i < exeFormat->numSegments; ++i) {
        struct Exe_Segment *segment = &exeFormat->segmentList[i];
        memcpy(userContext->memory + segment->startAddress, exeFileData +
            segment->offsetInFile, segment->lengthInFile);
    }

    // Format argument block.
    Format_Argument_Block(userContext->memory + argBlockAddr, numArgs,
        argBlockAddr, command);
    // Fill in code entry point.
    userContext->entryAddr = exeFormat->entryAddr;
    // Fill in addresses of argument block and stack.
    userContext->argBlockAddr = argBlockAddr;
    userContext->stackPointerAddr = argBlockAddr;
    *pUserContext = userContext;

    return 0;
}

/*
 * Copy data from user memory into a kernel buffer.
 * Params:
 * destInKernel - address of kernel buffer
 * srcInUser - address of user buffer
 * bufSize - number of bytes to copy
 *
 * Returns:
 *   true if successful, false if user buffer is invalid (i.e.,
 *   doesn't correspond to memory the process has a right to
 *   access)
 */
bool Copy_From_User(void* destInKernel, ulong_t srcInUser, ulong_t bufSize)
{
    /*
     * Hints:
     * - the User_Context of the current process can be found
     *   from g_currentThread->userContext
     * - the user address is an index relative to the chunk
     *   of memory you allocated for it
     * - make sure the user buffer lies entirely in memory belonging
     *   to the process
     */
    struct User_Context *userContext = g_currentThread->userContext;

    if (!Validate_User_Memory(userContext, srcInUser, bufSize)) {
	   return false;
    }

    memcpy(destInKernel, userContext->memory + srcInUser, bufSize);

    return true;
}

/*
 * Copy data from kernel memory into a user buffer.
 * Params:
 * destInUser - address of user buffer
 * srcInKernel - address of kernel buffer
 * bufSize - number of bytes to copy
 *
 * Returns:
 *   true if successful, false if user buffer is invalid (i.e.,
 *   doesn't correspond to memory the process has a right to
 *   access)
 */
bool Copy_To_User(ulong_t destInUser, void* srcInKernel, ulong_t bufSize)
{
    /*
     * Hints: same as for Copy_From_User()
     */
    struct User_Context *userContext = g_currentThread->userContext;

    if (!Validate_User_Memory(userContext, destInUser, bufSize)) {
	   return false;
    }

    memcpy(userContext->memory + destInUser, srcInKernel, bufSize);

    return true;
}

/*
 * Switch to user address space belonging to given
 * User_Context object.
 * Params:
 * userContext - the User_Context
 */
void Switch_To_Address_Space(struct User_Context *userContext)
{
    /*
     * Hint: you will need to use the lldt assembly language instruction
     * to load the process's LDT by specifying its LDT selector.
     */
    ushort_t ldtSelector;
    // Switch to the LDT of the new user context.
    ldtSelector = userContext->ldtSelector;

    __asm__ __volatile__ (
    "lldt %0"
    :
    :"a" (ldtSelector)
    );
}
