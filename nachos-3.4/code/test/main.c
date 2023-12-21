#include "syscall.h"
#include "copyright.h"

#define MAX_LENGTH 32

int main()
{
    int success;                                                          // success flag for opening, creating file
    SpaceId inputId, outputId, sinhvienId, voinuocResId;                     // file ids
    int numTime;                                                          // number of time
    char readChar;                                                        // charater read from file
    int i;

    // Create semaphores
    success = CreateSemaphore("main", 0);
    if (success == -1)
        return 1;
    success = CreateSemaphore("sinhvien", 0);
    if (success == -1)
        return 1;
    success = CreateSemaphore("voinuoc", 0);
    if (success == -1)
        return 1;
    success = CreateSemaphore("voinuocRes", 0);
    if (success == -1)
        return 1;

    // Create file output.txt
    success = Create("output.txt");
    if (success == -1)
        return 1;

    // Open file input.txt for read only
    inputId = Open("input.txt", 1);
    if (inputId == -1){
        Close(inputId);
        return 1;
    }

    // Open output.txt for read & write
    outputId = Open("output.txt", 0);
    if (outputId == -1){
        Close(inputId);
        Close(outputId);
        return 1;
    }

    // Read number of time
    // the pointer of file input.txt is at line 1 after the code below
    numTime = 0;
    while (1)
    {
        Read(&readChar, 1, inputId);
        if (readChar != '\n')
        {
            // Convert char to int
            if (readChar >= '0' && readChar <= '9')
                numTime = numTime * 10 + (readChar - '0');
        }
        else
            break;
    }

    // Call sinhvien
    success = Exec("./test/sinhvien");
    if (success == -1)
    {
        Close(inputId);
        Close(outputId);
        return 1;
    }

    // Call voinuoc
    success = Exec("./test/voinuoc");
    if (success == -1)
    {
        Close(inputId);
        Close(outputId);
        return 1;
    }

    // Loop numTime
    for(i = 0; i < numTime; i++)
    {
        // Create file currentSinhVien.txt
        // contains current sinhvien in one time (in a line)
        success = Create("sinhvien.txt");
        if (success == -1)
        {
            Close(inputId);
            Close(outputId);
            return 1;
        }

        // Open file sinhvien.txt for read & write
        sinhvienId = Open("sinhvien.txt", 0);
        if (sinhvienId == -1)
        {
            Close(inputId);
            Close(outputId);
            return 1;
        }

        // Read each sinhvien's volumes in line and write to sinhvien.txt 
        while (1)
        {
            if (Read(&readChar, 1, inputId) < 1)
            {
                break;
            }
            if (readChar != '\n')
            {
                Write(&readChar, 1, sinhvienId);
            }
            else
                break;
        }

        Close(sinhvienId);

        // signal to sinhvien
        Signal("sinhvien");

        // wait for main
        Wait("main");

        // Read file from voinuoc.txt and write to output.txt
        // voinuocRes.txt contains result of voinuoc that each sinhvien inline has used
        voinuocResId = Open("voinuocRes.txt", 1);
        if (voinuocResId == -1)
        {
            Close(inputId);
            Close(outputId);
            return 1;
        }

        PrintString("\n Thoi diem thu "); PrintInt(i); PrintString(":\n");

        // Read file voinuocRes.txt and write to output.txt 
        while (1)
        {
            if (Read(&readChar, 1, voinuocResId) < 1)
            {
                Write("\r\n", 2, outputId);
                Close(voinuocResId);
                break;
            }
            Write(&readChar, 1, outputId);
            PrintChar(readChar);
        }
    }
    PrintChar('\n');

    Close(inputId);
    Close(outputId);
    return 0;
}