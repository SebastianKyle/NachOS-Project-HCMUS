#include "syscall.h"
#define MAX_LENGTH 32

int main() {
    char fileName[MAX_LENGTH];
    PrintString("\n\t\t\t --------TAO FILE-------- \n\n");

    PrintString(" - Nhap ten file: ");
    ReadString(fileName, MAX_LENGTH);

    if (strlen(fileName) == 0 || strlen(fileName) == 1) {
        strcpy(fileName, "text.txt");
    }

    if (Create(fileName) == 0) {
        PrintString("\n Tao file thanh cong.");
    }
    else {
        PrintString("\n Loi trong qua trinh tao file.");
    }
}
