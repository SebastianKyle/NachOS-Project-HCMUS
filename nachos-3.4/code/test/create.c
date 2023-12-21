#include "syscall.h"
#define MAX_LENGTH 32

int main()
{
    char fileName[MAX_LENGTH];
    PrintString("\n\t\t\t --------TAO FILE-------- \n\n");

    PrintString(" - Nhap ten file: ");
    ReadString(fileName, MAX_LENGTH);

    if (fileName[0] == 0 || fileName[1] == 1)
    {
        fileName[0] = 't';
        fileName[1] = 'e';
        fileName[2] = 's';
        fileName[3] = 't';
        fileName[4] = '.';
        fileName[5] = 't';
        fileName[6] = 'x';
        fileName[7] = 't';
    }

    if (Create(fileName) == 0)
    {
        PrintString("\n Tao file thanh cong.");
    }
    else
    {
        PrintString("\n Loi trong qua trinh tao file.");
    }

    PrintString("\n\n");
    Halt();
}