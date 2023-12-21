#include "syscall.h"
#define MAX_LENGTH 32
#define MAX_BUFFER 255

int main() {
    char fileName[MAX_LENGTH + 1];
    char content[MAX_BUFFER + 1];
    OpenFileId fileId;
    int len;
    
    PrintString("\n\t\t\t --------TEST FILE-------- \n");

    // Tao file
    PrintString("\n - Nhap ten file: ");
    ReadString(fileName, MAX_LENGTH);

    if (fileName[0] == 0 || fileName[1] == 1) {
        fileName[0] = 't';
        fileName[1] = 'e';
        fileName[2] = 's';
        fileName[3] = 't';
        fileName[4] = '.';
        fileName[5] = 't';
        fileName[6] = 'x';
        fileName[7] = 't';
    }

    if (Create(fileName) == 0) {
        PrintString("\n Tao file thanh cong.");
    }
    else {
        PrintString("\n Loi trong qua trinh tao file.");
    }

    // Mo file stdout
    fileId = Open(fileName, 3);
    if (fileId == -1)
    {
        PrintString("\n Loi trong qua trinh mo file stdout.");
        Halt();
    }

    // Mo file stdin
    fileId = Open(fileName, 2);
    if (fileId == -1) {
        PrintString("\n Loi trong qua trinh mo file stdin.");
        Halt();
    }

    // Doc file stdin
    PrintString("\n - Nhap noi dung file: ");
    len = Read(content, MAX_BUFFER, fileId);

    // Mo file doc ghi
    fileId = Open(fileName, 0);
    if (fileId == -1) {
        PrintString("\n Loi trong qua trinh mo file.");
        Halt();
    }

    // Ghi file
    len = Write(content, len, fileId);
    if(len >= 0) {
        PrintString("\n Ghi file thanh cong.");
    }
    else {
        PrintString("\n Loi trong qua trinh ghi file.");
    }

    // Dong file
    Close(fileId);

    // Mo file chi doc
    fileId = Open(fileName, 1);
    len = Read(content, MAX_BUFFER, fileId);
    if(len >= 0) {
        PrintString("\n - Noi dung file vua nhap: ");
        PrintString(content);
    }
    else {
        PrintString("\n Loi trong qua trinh doc file.");
    }

    // Dong file
    Close(fileId);
    PrintChar('\n');

    Halt();
}
