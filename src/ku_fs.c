
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#define BLOCK_SIZE (4096)


typedef struct inode{
    unsigned int fsize;
    unsigned int blocks;
    unsigned int pointer[12];
    unsigned int unused[64];
} inode;

typedef struct fileEntry {
    struct {
        char inum;
        char Name[3];
    };
} fileEntry;

typedef struct iblock{
    inode inodes[16];
} iblock;

typedef struct dblock{
    char status[4096];
} dblock;

typedef struct iBlocks{
    iblock iblocks[5];
} iBlocks;

typedef struct dBlocks{

    dblock dblocks[55];

} dBlocks;

typedef struct superBlock{
    char status[4096];
} superBlock;

typedef struct i_bmapBlock{
    char status[4096];
} i_bmapBlock;

typedef struct d_bmapBlock{
    char status[4096];
} d_bmapBlock;

typedef struct rootBlock{
    fileEntry fileEntries[1024];
} rootBlock;

typedef struct disk{
    superBlock super_Block;
    i_bmapBlock i_BmapBlock;
    d_bmapBlock d_BmapBlock;
    iBlocks i_Blocks;
    rootBlock root_Block;
    dBlocks d_Blocks;
} disk;

disk disks;
int roothead;
int ihead;
int dhead;

void init(){
    memset(&disks, 0, sizeof(disk));


    disks.i_BmapBlock.status[0] = 1;
    disks.i_BmapBlock.status[1] = 1;
    disks.i_BmapBlock.status[2] = 1;

    disks.d_BmapBlock.status[0] = 1;
    
    disks.i_Blocks.iblocks[0].inodes[2].fsize = 4*80;
    disks.i_Blocks.iblocks[0].inodes[2].blocks = 1;
    
    roothead = 0;
    ihead = 3;
    dhead = 1;
    
    
    printf("init success\n");
}

void print_EOF(){
    char *block = (char*)&disks; // disk의 시작주소
    for(int i = 0; i < 4096 * 64; i++){ // 4kb (4096 bytes) * 64 blocks
        // fprintf(fp, "%.2x ", *((unsigned char*)(block+i)));
        printf("%.2x ", *((unsigned int*)(block+i)));
    }
}

int isRepeat(const char *name){
    for(int i = 0;i<1024;i++){
        if (disks.root_Block.fileEntries[i].inum == 1) {
            if(disks.root_Block.fileEntries[i].Name[0] == name[0] && disks.root_Block.fileEntries[i].Name[1] == name[1]){
                return disks.root_Block.fileEntries[i].inum;
            }
        }
    }
    return 0;
}

void writeFile(const char *name, const unsigned int fsize){
    
    if(isRepeat(name) != 0){
        printf("Already exists\n");
        return;
    }
    
    int a = fsize / BLOCK_SIZE;
    int b = fsize % BLOCK_SIZE;
    int needBlock = 0;
    if (b > 0) {
        needBlock = a+1;
    }
    
    if(55<needBlock){
        printf("No Space\n");
        return;
    }
   
    if (dhead + needBlock >= 58) {
        printf("No Space\n");
        return;;
    }
    else{
        printf("%d번째줄 입력 중..\n",roothead+1);
    }
    
    disks.root_Block.fileEntries[roothead].inum = ihead;
    disks.root_Block.fileEntries[roothead].Name[0] = name[0];
    disks.root_Block.fileEntries[roothead].Name[1] = name[1];
    
    disks.i_BmapBlock.status[ihead] = 1;
    for (int i =0; i<needBlock; i++) {
        disks.d_BmapBlock.status[dhead+i] = 1;
    }
    int temp1 = -1;
    int temp2 = 0;
    
    temp2 = (ihead+1)%16;
    temp1 = (ihead+1)/16;
    
    if(temp2 != 0){
        disks.i_Blocks.iblocks[temp1].inodes[temp2-1].fsize = fsize;
        disks.i_Blocks.iblocks[temp1].inodes[temp2-1].blocks = needBlock;
        for (int i =0; i<needBlock; i++) {
            disks.i_Blocks.iblocks[temp1].inodes[temp2-1].pointer[i] = dhead+i;
        }
        memset(&disks.d_Blocks.dblocks[dhead-1], name[0], fsize);
    }
    else if (temp2 == 0){
        disks.i_Blocks.iblocks[temp1-1].inodes[15].fsize = fsize;
        disks.i_Blocks.iblocks[temp1-1].inodes[15].blocks = needBlock;
        for (int i =0; i<needBlock; i++) {
            disks.i_Blocks.iblocks[temp1-1].inodes[15].pointer[i] = dhead+i;
        }
        memset(&disks.d_Blocks.dblocks[dhead-1], name[0], fsize);
    }
    
    printf("%d번째줄 입력완료 \n",roothead+1);
    roothead++;
    ihead++;
    dhead = dhead + needBlock;
    
}



int main(int argc, char* argv[]) {
    // FILE *fp;
    // fp = fopen("/workspace/os2/src/result.txt","w");
    
    char line[1024];
    char fileName[3];
    char command;
    unsigned int size = 0;
    
    FILE *fd;
   if (argc != 2) {
           printf("ku_fs: Wrong number of arguments\n");
           return 1;
       }

       fd = fopen(argv[1], "r");
       if (!fd) {
           printf("ku_fs: Fail to open the input file\n");
           return 1;
       }
	
    // fd = fopen("/workspace/os2/src/a.txt","r");
    init();
    
    while (fgets(line, sizeof(line), fd) != NULL) {
        sscanf(line, "%s %c %ud", fileName, &command, &size);
        switch (command) {
            case 'w':
                writeFile(fileName, size);
                break;
           
           default:
               goto printEOF;
               break;
        }
    }
	printEOF:
    print_EOF();
	
	fclose(fd);
    
    
    return 0;


}
