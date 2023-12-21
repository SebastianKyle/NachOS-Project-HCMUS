#include "syscall.h"
#include "copyright.h"

void main()
{
    int success;                                // success flag for opening, creating file
    SpaceId sinhvienId, voinuocId;              // file ids
    char readChar;                              // charater read from file
    int flag_VN;                                // flag to signal voinuoc thread
    int flag_MAIN;                              // flag to signal main thread
    int lengthFile;                             // length of file
    int pFile;                                  // current pointer of file

    // -----------------------------------------------------------

    while (1)
    {
        lengthFile = 0;

        Wait("sinhvien");

        // Create file voinuocRes.txt
        success = Create("voinuocRes.txt");
        if (success == -1)
        {
            Signal("main");     // return to main thread
            return;
        }
        Signal("voinuocRes");   // signal voinuocRes thread

        // Open file sinhvien.txt to read
        sinhvienId = Open("sinhvien.txt", 1);
        if (sinhvienId == -1)
        {
            Signal("main");     // return to main thread
            return;
        }

        lengthFile = Seek(-1, sinhvienId);
        Seek(0, sinhvienId);
        pFile = 0;

        // Create file voinuoc.txt
        success = Create("voinuoc.txt");
        if (success == -1)
        {
            Close(sinhvienId);
            Signal("main");
            return;
        }

        // Open file voinuoc.txt to write
        voinuocId = Open("voinuoc.txt", 0);
        if (voinuocId == -1)
        {
            Close(sinhvienId);
            Signal("main");
            return;
        }

        // Write volume from sinhvien.txt to voinuoc.txt one by one
        // each volume will signal voinuoc thread to start
        while (pFile < lengthFile)
        {
            flag_VN = 0;
            Read(&readChar, 1, sinhvienId);
            if (readChar != ' ')
            {
                Write(&readChar, 1, voinuocId);
            }
            else
            {
                flag_VN = 1;
            }

            // If end of file, write '*' to signal voinuoc thread
            if (pFile == lengthFile - 1)
            {
                Write("*", 1, voinuocId);
                flag_VN = 1;
            }

            if (flag_VN == 1)
            {
                Close(voinuocId);
                Signal("voinuoc");
                
                // Stop until voinuoc finish
                // we have results in voinuocRes.txt
                Wait("sinhvien");

                // Recreate to empty voinuoc.txt
                success = Create("voinuoc.txt");
                if (success == -1)
                {
                    Close(sinhvienId);
                    Signal("main");
                    return;
                }

                // Open file voinuoc.txt to ready for the next volume
                voinuocId = Open("voinuoc.txt", 0);
                if (voinuocId == -1)
                {
                    Close(sinhvienId);
                    Signal("main");
                    return;
                }
            }   
            pFile++;
        }

        // Finish sinhvien and voinuoc, return to main  
        Close(sinhvienId);
        Signal("main"); 
    }
}