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
{   struct User_Context * UserContext;
    size = Round_Up_To_Page(size);
    UserContext = (struct User_Context *)Malloc(sizeof(struct User_Context));
    //为用户态进程
    if (UserContext != 0)
       UserContext->memory = Malloc(size);
    //为核心态进程
    else
       goto fail;

    //内存为空
    if (0 == UserContext->memory)
       goto fail;
    memset(UserContext->memory, '\0', size);
    UserContext->size = size;
    //以下为用户态进程创建LDT(段描述符表)
    //新建一个LDT描述符
    UserContext->ldtDescriptor = Allocate_Segment_Descriptor();
    if (0 == UserContext->ldtDescriptor){
	Print("Allocate_Segment_Descriptor_fail  \n");
        goto fail;
}
  
    //初始化段描述符
    Init_LDT_Descriptor(UserContext->ldtDescriptor, UserContext->ldt, NUM_USER_LDT_ENTRIES);
    //新建一个LDT选择子
    UserContext->ldtSelector = Selector(KERNEL_PRIVILEGE, true, Get_Descriptor_Index(UserContext->ldtDescriptor));
    //新建一个文本段描述符
    Init_Code_Segment_Descriptor(
        &UserContext->ldt[0],
        (ulong_t) UserContext->memory,
        size / PAGE_SIZE,
        USER_PRIVILEGE
    );
    //新建一个数据段
    Init_Data_Segment_Descriptor(
        &UserContext->ldt[1],
        (ulong_t) UserContext->memory,
        size / PAGE_SIZE,
        USER_PRIVILEGE
    );
    //新建数据段和文本段选择子
    UserContext->csSelector = Selector(USER_PRIVILEGE, false, 0);
    UserContext->dsSelector = Selector(USER_PRIVILEGE, false, 1);
    //将引用数清0
    UserContext->refCount = 0; 
    return UserContext;
fail:
    if (UserContext != 0){
       if (UserContext->memory != 0){
           Free(UserContext->memory);
       }
       Free(UserContext);
    }
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
    //TODO("Destroy a User_Context");
// KASSERT(userContext->refCount == 0);
/* Free the context's LDT descriptor */
// Free_Segment_Descriptor(userContext->ldtDescriptor);
/* Free the context's memory */
// Disable_Interrupts();
// Free(userContext->memory);
// Free(userContext);
// Enable_Interrupts();

    //释放占用的LDT
    Free_Segment_Descriptor(userContext->ldtDescriptor);
    userContext->ldtDescriptor=0;
 
    //释放内存空间
    Free(userContext->memory);
    userContext->memory=0;
 
    //释放userContext本身占用的内存
    Free(userContext);
    userContext=0;
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
      //TODO("Load a user executable into a user memory space using segmentation");
 int i;
 ulong_t maxva = 0;//要分配的最大内存空间
 unsigned numArgs;//进程数目
 ulong_t argBlockSize;//参数块的大小
 ulong_t size, argBlockAddr;//参数块地址
 struct User_Context *userContext = 0;

 //计算用户态进程所需的最大内存空间
 for (i = 0; i < exeFormat->numSegments; ++i) {
  //elf.h
  struct Exe_Segment *segment = &exeFormat->segmentList[i];
  ulong_t topva = segment->startAddress + segment->sizeInMemory; /* FIXME: range check */
  if (topva > maxva)
   maxva = topva;
  }
 Get_Argument_Block_Size(command, &numArgs, &argBlockSize);//获取参数块信息
 size = Round_Up_To_Page(maxva) + DEFAULT_USER_STACK_SIZE;//用户进程大小=参数块总大小 + 进程堆栈大小(8192)
 argBlockAddr = size;
 size += argBlockSize;

 userContext = Create_User_Context(size);//按相应大小创建一个进程
 if (userContext == 0)//创建是否成功
  return -1;
 for (i = 0; i < exeFormat->numSegments; ++i) {
  struct Exe_Segment *segment = &exeFormat->segmentList[i];
  //根据段信息将用户程序中的各段内容复制到分配的用户内存空间
  memcpy(userContext->memory + segment->startAddress, exeFileData + segment->offsetInFile,segment->lengthInFile);
  }
 //格式化参数块

 Format_Argument_Block(userContext->memory + argBlockAddr, numArgs, argBlockAddr, command);
 //初始化数据段，堆栈段及代码段信息
 userContext->entryAddr = exeFormat->entryAddr;
 userContext->argBlockAddr = argBlockAddr;
 userContext->stackPointerAddr = argBlockAddr;
 //将初始化完毕的User_Context赋给*pUserContext
 *pUserContext = userContext;
 return 0;//成功
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
    //TODO("Copy memory from user buffer to kernel buffer");
  //  Validate_User_Memory(NULL,0,0); /* delete this; keeps gcc happy */
 struct User_Context * UserContext = g_currentThread->userContext;

 //--: check if memory if validated
 if (!Validate_User_Memory(UserContext,srcInUser, bufSize))
  return false;

 //--:user->kernel
 memcpy(destInKernel, UserContext->memory + srcInUser, bufSize);
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
     //TODO("Copy memory from kernel buffer to user buffer")
 struct User_Context * UserContext = g_currentThread->userContext;

 //--: check if memory if validated
 if (!Validate_User_Memory(UserContext, destInUser,  bufSize))
  return false;
 
 //--:kernel->user
 memcpy(UserContext->memory + destInUser, srcInKernel, bufSize);

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
     //TODO("Switch to user address space using segmentation/LDT");
 ushort_t ldtSelector= userContext->ldtSelector;
 /* Switch to the LDT of the new user context */
 __asm__ __volatile__ ("lldt %0"::"a"(ldtSelector));

}


