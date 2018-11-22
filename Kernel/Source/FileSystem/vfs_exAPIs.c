#include "stdio.h"
#include "string.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "Hardware/ahci/ahci.h"

#include "vfs.h"
#include "ext2/ext2_fs.h"
#include "AqFS/Aqfs.h"

int Aqfs_list(char* path)
{
    if(!path)
    {
        Aq_ListEntrys(".");
        //Aq_LoadFile("root");
    }
    else 
    {
        Aq_ListEntrys(path);
    }
    return 0;
}

int Aqfs_makenode(char* path, int type)
{

}

int Aqfs_delnode(char* path)
{

}