/*
 * GeekOS C code entry point
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2004, Iulian Neamtiu <neamtiu@cs.umd.edu>
 * $Revision: 1.51 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/bootinfo.h>
#include <geekos/string.h>
#include <geekos/screen.h>
#include <geekos/mem.h>
#include <geekos/crc32.h>
#include <geekos/tss.h>
#include <geekos/int.h>
#include <geekos/kthread.h>
#include <geekos/trap.h>
#include <geekos/timer.h>
#include <geekos/keyboard.h>

#define USERNAME_STRING_SIZE 6
#define PASSWORD_STRING_SIZE 6
#define LOGIN_COMMENT_SIZE   27
#define SHELL_COMMENT_SIZE   24
#define SHELL_COMMAND_SIZE   9

#define PRINT_KERNEL_SHELL                         \
do {                                               \
    Set_Current_Attr(ATTRIB(BLACK, GRAY));         \
    Print("[");                                    \
    Set_Current_Attr(ATTRIB(BLACK, AMBER|BRIGHT)); \
    Print("geekos");                               \
    Set_Current_Attr(ATTRIB(BLACK, RED|BRIGHT));   \
    Print("@");                                    \
    Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT)); \
    Print("geekos-bochs");                         \
    Set_Current_Attr(ATTRIB(BLACK, GRAY));         \
    Print("] > ");                                 \
} while (0)

#define PRINT_LOGIN_ERROR(message)                       \
do {                                                     \
    Set_Current_Attr(ATTRIB(BLACK, RED|BRIGHT));         \
    Print("\nError %s! Please try again!\n", (message)); \
    Set_Current_Attr(ATTRIB(BLACK, GRAY));               \
} while (0)

#define PRINT_ROBOT(message)      \
do {                              \
    Print("             \n");     \
    Print("   _ _ _ _   \n");     \
    Print("  | O V O |  \n");     \
    Print("  |_ _-_ _|  \n");     \
    Print("=============\n");     \
    Print("||    |    ||\n");     \
    Print("||    |    ||\n");     \
    Print("||    |    ||\n");     \
    Print("      |      \n");     \
    Print("=============\n");     \
    Print("   WW   WW   \n");     \
    Print("\"%s\"\n", (message)); \
} while (0)

typedef struct LoginInformation {
    char *username;
    char *password;
    char *type;
} LoginInformation;

int g_thread_a = 1;
int g_thread_b = 0;

static void printCharacterA(ulong_t arg)
{
    while (1) {
        if (g_thread_a == 1 && g_thread_b == 0) {
            Print("Thread A\n");
            g_thread_a = 0;
            g_thread_b = 1;
        }
        Print("\0");
    }
}

static void printCharacterB(ulong_t arg)
{
    while (1) {
        if (g_thread_a == 0 && g_thread_b == 1) {
            Print("Thread B\n");
            g_thread_a = 1;
            g_thread_b = 0;
        }
        Print("\0");
    }
}

static void printKeyboardCharacter(ulong_t arg)
{
    int input_char;
    int finish_char;

    while (1) {
        input_char = Wait_For_Key();
        finish_char = ('d' | KEY_CTRL_FLAG);
        if (input_char == finish_char) {
            Print("\nFinish input!\n");
            Exit(0);
        }
        else {
            Print("%c", input_char);
        }
    }
}

static void printCurrentKernelThreadPid()
{
    struct Kernel_Thread *current_kernel_thread;
    current_kernel_thread = Get_Current();

    Print("Current Kernel Thread Pid: %d\n", current_kernel_thread->pid);
}

static void delayMilliSeconds(int milliseconds)
{
    Micro_Delay(1000 * milliseconds);
}

static void delaySeconds(int seconds)
{
    Micro_Delay(1000000 * seconds);
}

static void runKernelShell(ulong_t arg)
{
    int  i;
    int  row, col;
    int  input_char;
    int  count = 0;
    char input_string[10];

    PRINT_KERNEL_SHELL;

    while (1) {
        input_char = Wait_For_Key();
        if (input_char >= 'a' && input_char <= 'z') {
            Print("%c", input_char);
            if (count <  SHELL_COMMAND_SIZE) {
                input_string[count] = input_char;
                count++;
            }
        }
        else if (input_char == KEY_DELETE_FLAG) {
            Get_Cursor(&row, &col);
            if (col > SHELL_COMMENT_SIZE) {
                Put_Cursor(row, col - 1);
                Put_Char(KEY_SPACE_FLAG);
                Put_Cursor(row, col - 1);
                if (count > 0) {
                    count--;
                }
            }
        }
        else if (input_char == KEY_ENTER_FLAG) {
            input_string[count] = '\0';
            count = 0;
            if (strcmp(input_string, "clear") == 0) {
                Print("A");
                Clear_Screen();
                Put_Cursor(0, 0);
            }
            else if (strcmp(input_string, "robot") == 0) {
                PRINT_ROBOT("Let's start to learn ROS!");
            }
            else {
                PRINT_LOGIN_ERROR("command");
            }
            PRINT_KERNEL_SHELL;
        }
    }
}

static void loginToOperatingSystem(ulong_t arg)
{
    int  row, col;
    int  count = 0;
    int  input_char;
    char username_string[7];
    char password_string[7];
    LoginInformation login_infomation;
    login_infomation.username = "geekos";
    login_infomation.password = "geekos";
    login_infomation.type     = "username";

    while (1) {
        input_char = Wait_For_Key();
        if (input_char == KEY_ENTER_FLAG) {
            if (strcmp(login_infomation.type, "username") == 0) {
                username_string[6] = '\0';
                if (strcmp(login_infomation.username, username_string) == 0) {
                    Print("\npassword(default: geekos): ");
                    login_infomation.type = "password";
                    count = 0;
                }
                else {
                    PRINT_LOGIN_ERROR("username");
                    Print("username(default: geekos): ");
                    count = 0;
                }
            }
            else if (strcmp(login_infomation.type, "password") == 0) {
                password_string[6] = '\0';
                if (strcmp(login_infomation.password, password_string) == 0) {
                    Print("\n\n");
                    Start_Kernel_Thread(runKernelShell, 0, PRIORITY_NORMAL,
                                        true);
                    Exit(0);
                }
                else {
                    PRINT_LOGIN_ERROR("password");
                    Print("password(default: geekos): ");
                    count = 0;
                }
            }
        }
        else if (input_char == KEY_DELETE_FLAG) {
            Get_Cursor(&row, &col);
            if (col > LOGIN_COMMENT_SIZE) {
                Put_Cursor(row, col - 1);
                Put_Char(KEY_SPACE_FLAG);
                Put_Cursor(row, col - 1);
                if (count > 0) {
                    count--;
                }
            }
        }
        else {
            Print("%c", input_char);
        }
        if (strcmp(login_infomation.type, "username") == 0) {
            if (count >= USERNAME_STRING_SIZE) {
                continue ;
            }
            if (input_char >= 'a' && input_char <= 'z') {
                username_string[count] = input_char;
                count++;
            }
        }
        else if (strcmp(login_infomation.type, "password") == 0) {
            if (count >= PASSWORD_STRING_SIZE) {
                continue ;
            }
            if (input_char >= 'a' && input_char <= 'z') {
                password_string[count] = input_char;
                count++;
            }
        }
        else {
            Print("\nError, please root initializes the correct informations ");
            Print("of login!\n");
            Exit(0);
        }
    }
}
/*
 * Kernel C code entry point.
 * Initializes kernel subsystems, mounts filesystems,
 * and spawns init process.
 */
void Main(struct Boot_Info* bootInfo)
{
    Init_BSS();
    Init_Screen();
    Init_Mem(bootInfo);
    Init_CRC32();
    Init_TSS();
    Init_Interrupts();
    Init_Scheduler();
    Init_Traps();
    Init_Timer();
    Init_Keyboard();

    Clear_Screen();
    Put_Cursor(0, 0);
    Set_Current_Attr(ATTRIB(BLACK, CYAN|BRIGHT));
    Print("Welcome to GeekOS!\n");
    Set_Current_Attr(ATTRIB(BLACK, GRAY));
    Print("**************************************************************\n");
    Print("*The project0 of geekos is modified by myyerrol for finishing*\n");
    Print("*the first part of os's experiment. It can implement a simple*\n");
    Print("*login operation and shell thread to complete two commands!  *\n");
    Print("**************************************************************\n");
    Set_Current_Attr(ATTRIB(BLACK, CYAN|BRIGHT));
    Print("Login\n");
    Set_Current_Attr(ATTRIB(BLACK, GRAY));
    Print("username(default: geekos): ");

    Start_Kernel_Thread(loginToOperatingSystem, 0, PRIORITY_NORMAL, true);
    // Start_Kernel_Thread(printKeyboardCharacter, 0, PRIORITY_NORMAL, true);
    // Start_Kernel_Thread(printCharacterA, 0, PRIORITY_NORMAL, true);
    // Start_Kernel_Thread(printCharacterB, 0, PRIORITY_NORMAL, true);
    // TODO("Start a kernel thread to echo pressed keys and print counts");

    /* Now this thread is done. */
    Exit(0);
}
