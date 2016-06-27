/*
 * ELF executable loading
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.29 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/ktypes.h>
#include <geekos/screen.h>  /* for debug Print() statements */
#include <geekos/pfat.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/elf.h>

int g_flag = 0;

/**
 * From the data of an ELF executable, determine how its segments
 * need to be loaded into memory.
 * @param exeFileData buffer containing the executable file
 * @param exeFileLength length of the executable file in bytes
 * @param exeFormat structure describing the executable's segments
 *   and entry address; to be filled in
 * @return 0 if successful, < 0 on error
 */
int Parse_ELF_Executable(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat)
{
	int i = 0;
	int j = 0;
	struct Exe_Segment* segment;

    elfHeader* eHeader = (elfHeader*)exeFileData;
	programHeader* pheader = (programHeader*)(exeFileData + eHeader->phoff);
	exeFormat->numSegments = eHeader->phnum;
	exeFormat->entryAddr = eHeader->entry;

	for(; i < exeFormat->numSegments; i++)
	{
		segment = &exeFormat->segmentList[i];
		segment->lengthInFile = pheader->fileSize;
		segment->offsetInFile = pheader->offset;
		segment->protFlags = pheader->flags;
		segment->sizeInMemory = pheader->memSize;
		segment->startAddress = pheader->vaddr;

        if (g_flag) {
            Print("%x\n", (int)(segment->lengthInFile));
    		Print("%x\n", (int)(segment->offsetInFile));
    		Print("%x\n", (int)(segment->protFlags));
    		Print("%x\n", (int)(segment->sizeInMemory));
    		Print("%x\n", (int)(segment->startAddress));
        }

		pheader = (programHeader*)((char*)pheader + eHeader->phentsize);
	}

    //TODO("Parse an ELF executable image");
    return 0;
}
