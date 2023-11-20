/* help.c
 *  Test program to print out group information
 *  and describe sort and ascii program
 */

#include "syscall.h"

int main() {
    PrintString("\n\t\t\t --------HELP-------- \n\n");

    PrintString("\n Thanh vien: ");
    PrintString("\n - Tran Y Van: 21120360");
    PrintString("\n - Ho Phuc: 21120311");
    PrintString("\n - Doan Manh Tan: 21120326");
    PrintString("\n - Nguyen Anh Thai: 21120331");

    PrintString("\n\n Cac chuong trinh: ");
    PrintString("\n +) sort: cho phep nguoi dung nhap vao 1 mang n so nguyen sau do sap xep mang");
    PrintString("\n +) ascii: in ra bang ma ASCII");
}