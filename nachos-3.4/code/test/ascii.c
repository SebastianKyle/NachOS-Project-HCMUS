/* ascii.c
 *  Test program to print out the ASCII table
 */

#include "syscall.h"

int main() {
    PrintString("\n\t\t\t --------ASCII-------- \n\n");

    PrintString("ASCII Value\tCharacter\n");

    for (int i = 0; i <= 127; i++) {
        PrintInt(i);
        PrintString("\t\t");
        PrintChar((char)i);
        PrintChar('\n');
    }

    return 0;
}