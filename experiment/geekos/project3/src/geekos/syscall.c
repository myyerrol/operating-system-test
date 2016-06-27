/*
 * System call handlers
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003,2004 David Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.59 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/syscall.h>
#include <geekos/errno.h>
#include <geekos/kthread.h>
#include <geekos/int.h>
#include <geekos/elf.h>
#include <geekos/malloc.h>
#include <geekos/screen.h>
#include <geekos/keyboard.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/timer.h>
#include <geekos/vfs.h>
#include <geekos/list.h>
#include <geekos/synch.h>

static struct Semaphore_List s_sphlist;
static int semnub=1;

static int Copy_User_String(ulong_t uaddr, ulong_t len, ulong_t maxLen, char **pStr)
{
    int rc = 0;
    char *str;

    if (len > maxLen) {
       return EINVALID;
    }

    str = (char*) Malloc(len + 1);

    if (0 == str) {
        rc = ENOMEM;
        goto fail;
    }
    if (!Copy_From_User(str, uaddr, len)) {
        rc = EINVALID;
        Free(str);
        goto fail;
    }

    str[len] = '\0';
    *pStr = str;
fail:
    return rc;
}

/*
 * Null system call.
 * Does nothing except immediately return control back
 * to the interrupted user program.
 * Params:
 *  state - processor registers from user mode
 *
 * Returns:
 *   always returns the value 0 (zero)
 */
static int Sys_Null(struct Interrupt_State* state)
{
    return 0;
}

/*
 * Exit system call.
 * The interrupted user process is terminated.
 * Params:
 *   state->ebx - process exit code
 * Returns:
 *   Never returns to user mode!
 */
static int Sys_Exit(struct Interrupt_State* state)
{
    // TODO("Exit system call");
    Exit(state->ebx);
}

/*
 * Print a string to the console.
 * Params:
 *   state->ebx - user pointer of string to be printed
 *   state->ecx - number of characters to print
 * Returns: 0 if successful, -1 if not
 */
static int Sys_PrintString(struct Interrupt_State* state)
{
    // TODO("PrintString system call");
    int rc = 0;
    uint_t length = state->ecx;
    uchar_t* buf = 0;

    if (length > 0) {
        if ((rc = Copy_User_String(state->ebx, length, 1023, (char**) &buf)) != 0)
            goto done;

        Put_Buf(buf, length);
    }

done:
    if (buf != 0)
        Free(buf);

    return rc;
}


/*
 * Get a single key press from the console.
 * Suspends the user process until a key press is available.
 * Params:
 *   state - processor registers from user mode
 * Returns: the key code
 */
static int Sys_GetKey(struct Interrupt_State* state)
{
    // TODO("GetKey system call");
    return Wait_For_Key();
}

/*
 * Set the current text attributes.
 * Params:
 *   state->ebx - character attributes to use
 * Returns: always returns 0
 */
static int Sys_SetAttr(struct Interrupt_State* state)
{
    // TODO("SetAttr system call");
    Set_Current_Attr((uchar_t) state->ebx);
    return 0;
}

/*
 * Get the current cursor position.
 * Params:
 *   state->ebx - pointer to user int where row value should be stored
 *   state->ecx - pointer to user int where column value should be stored
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_GetCursor(struct Interrupt_State* state)
{
    // TODO("GetCursor system call");
    int row, col;
    Get_Cursor(&row, &col);

    if (!Copy_To_User(state->ebx, &row, sizeof(int)) ||!Copy_To_User(state->ecx, &col, sizeof(int)))
        return -1;

    return 0;
}


/*
 * Set the current cursor position.
 * Params:
 *   state->ebx - new row value
 *   state->ecx - new column value
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_PutCursor(struct Interrupt_State* state)
{
    // TODO("PutCursor system call");
    return Put_Cursor(state->ebx, state->ecx) ? 0 : -1;
}

/*
 * Create a new user process.
 * Params:
 *   state->ebx - user address of name of executable
 *   state->ecx - length of executable name
 *   state->edx - user address of command string
 *   state->esi - length of command string
 * Returns: pid of process if successful, error code (< 0) otherwise
 */
static int Sys_Spawn(struct Interrupt_State* state)
{
    // TODO("Spawn system call");
    int rc;
    char *program = 0;
    char *command = 0;
    struct Kernel_Thread *process;
    /* Copy program name and command from user space. */
    if ((rc = Copy_User_String(state->ebx, state->ecx, VFS_MAX_PATH_LEN, &program)) != 0 ||
    (rc = Copy_User_String(state->edx, state->esi, 1023, &command)) != 0)
        goto done;

    Enable_Interrupts();
    rc = Spawn(program, command, &process);

    if (rc == 0) {
        KASSERT(process != 0);
        rc = process->pid;
    }

    Disable_Interrupts();
done:
    if (program != 0)
        Free(program);
    if (command != 0)
        Free(command);

    return rc;
}

/*
 * Wait for a process to exit.
 * Params:
 *   state->ebx - pid of process to wait for
 * Returns: the exit code of the process,
 *   or error code (< 0) on error
 */
static int Sys_Wait(struct Interrupt_State* state)
{
    // TODO("Wait system call");
    int exitCode;
    struct Kernel_Thread *kthread = Lookup_Thread(state->ebx);

    if (kthread == 0)
        return -12;

    Enable_Interrupts();
    exitCode = Join(kthread);
    Disable_Interrupts();
    return exitCode;
}


/*
 * Get pid (process id) of current thread.
 * Params:
 *   state - processor registers from user mode
 * Returns: the pid of the current thread
 */
static int Sys_GetPID(struct Interrupt_State* state)
{
    // TODO("GetPID system call");
    return g_currentThread->pid;

}

/*
 * Set the scheduling policy.
 * Params:
 *   state->ebx - policy,
 *   state->ecx - number of ticks in quantum
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_SetSchedulingPolicy(struct Interrupt_State* state)
{
    if (state->ebx != ROUND_ROBIN && state->ebx != MULTILEVEL_FEEDBACK)
        return -1;

    g_schedulingPolicy = state->ebx;
    g_Quantum = state->ecx;
    return 0;
}

/*
 * Get the time of day.
 * Params:
 *   state - processor registers from user mode
 *
 * Returns: value of the g_numTicks global variable
 */
static int Sys_GetTimeOfDay(struct Interrupt_State* state)
{
    // TODO("GetTimeOfDay system call");
    return g_numTicks;
}

/*
 * Create a semaphore.
 * Params:
 *   state->ebx - user address of name of semaphore
 *   state->ecx - length of semaphore name
 *   state->edx - initial semaphore count
 * Returns: the global semaphore id
 */
static int Sys_CreateSemaphore(struct Interrupt_State* state)
{
    // TODO("CreateSemaphore system call");
    int rc;
    char *name = 0;
    //int exit, id_sem;
    struct Semaphore *s = s_sphlist.head;

    if ((rc = Copy_User_String(state->ebx, state->ecx, VFS_MAX_PATH_LEN, &name)) != 0 )
        goto fail;
    //Print("Copy_User_String_Name =%s\n",name);
    while(s != 0) {
    //Print("whiles->semaphoreName=%s\n",s->semaphoreName);
        if(strcmp(s->semaphoreName, name) == 0) {
            s->registeredThreads[s->registeredThreadCount] = g_currentThread;
            s->registeredThreadCount += 1;
            return s->semaphoreID;
        }
        s = Get_Next_In_Semaphore_List(s);
    }
    s = (struct Semaphore *)Malloc(sizeof(struct Semaphore));
    s->registeredThreads[0] = g_currentThread;
    s->registeredThreadCount = 1;
    //strcpy(s->semaphoreName,name);
    s->semaphoreName = name;
    //Print("s->semaphoreName=name===%s\n",s->semaphoreName);
    Clear_Thread_Queue(&s->waitingThreads);
    s->value = state->edx;
    s->semaphoreID = semnub;
    semnub++;
    Add_To_Back_Of_Semaphore_List(&s_sphlist,s);

    return s->semaphoreID;
fail:
    Print("CreateSemaphore failed!");
    return -1;
}

/*
 * Acquire a semaphore.
 * Assume that the process has permission to access the semaphore,
 * the call will block until the semaphore count is >= 0.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_P(struct Interrupt_State* state)
{
    // TODO("P (semaphore acquire) system call");
    struct Semaphore *s=s_sphlist.head;

    while(s != 0) {
        if(s->semaphoreID == state->ebx)
            break;
        s = Get_Next_In_Semaphore_List(s);
    }
    if(s == 0)
        return -1;

    s->value -= 1;

    if(s->value < 0)
        Wait(&s->waitingThreads);

    return 0;
}

/*
 * Release a semaphore.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_V(struct Interrupt_State* state)
{
    // TODO("V (semaphore release) system call");
    struct Kernel_Thread *kthread;
    struct Semaphore *s=s_sphlist.head;

    while(s != 0) {
        if(s->semaphoreID==state->ebx)
             break;
        s = Get_Next_In_Semaphore_List(s);
    }
    if(s == 0)
        return -1;

    s->value+=1;

    if(s->value>=0) {
        kthread = s->waitingThreads.head;
        if( kthread !=0) {
            //kthread = Get_Front_Of_Thread_Queue(&s->waitingThreads);
            //Remove_Thread(&s->waitingThreads, kthread);
            Wake_Up_One(&s->waitingThreads);
        }
    }

    return 0;
}

/*
 * Destroy a semaphore.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_DestroySemaphore(struct Interrupt_State* state)
{
    // TODO("DestroySemaphore system call");
    struct Semaphore *s = s_sphlist.head;

    while (s) {
        if (s->semaphoreID == state->ebx)
            break;
        s = Get_Next_In_Semaphore_List(s);
    }

    if (s == 0)
        return -1;

    s->registeredThreadCount--;

    if (s->registeredThreadCount == 0) {
        Free(s);
        Remove_From_Semaphore_List(&s_sphlist, s);
    }

    return 0;
}

/*
 * Global table of system call handler functions.
 */
const Syscall g_syscallTable[] = {
    Sys_Null,
    Sys_Exit,
    Sys_PrintString,
    Sys_GetKey,
    Sys_SetAttr,
    Sys_GetCursor,
    Sys_PutCursor,
    Sys_Spawn,
    Sys_Wait,
    Sys_GetPID,
    /* Scheduling and semaphore system calls. */
    Sys_SetSchedulingPolicy,
    Sys_GetTimeOfDay,
    Sys_CreateSemaphore,
    Sys_P,
    Sys_V,
    Sys_DestroySemaphore,
};

/*
 * Number of system calls implemented.
 */
const int g_numSyscalls = sizeof(g_syscallTable) / sizeof(Syscall);
