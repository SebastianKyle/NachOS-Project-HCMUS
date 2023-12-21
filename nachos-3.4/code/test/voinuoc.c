#include "syscall.h"
#include "copyright.h"

void main()
{
    int success;                                // success flag for opening, creating file
    SpaceId voinuocId, voinuocResId;            // file ids
    char readChar;                              // charater read from file
    int v1, v2;                                 // Voi 1, Voi 2
    int volume;                                 // volume of water bottle of each student
    int flag_done_result;                       // flag to signal result thread

    //-----------------------------------------------------------

    while (1)
    {
        v1 = v2 = 0;

        Wait("voinuocRes");
        // Open file voinuocRes.txt to write
        voinuocResId = Open("voinuocRes.txt", 0);
        if (voinuocResId == -1)
        {
            Signal("sinhvien");
            return;
        }

        while (1)
        {
            Wait("voinuoc");
            readChar = 0;

            // Open file voinuoc.txt to read volume 
            voinuocId = Open("voinuoc.txt", 1);
            if (voinuocId == -1)
            {
                Close(voinuocResId);
                Signal("sinhvien");
                return;
            }

            volume = 0;
            flag_done_result = 0;

            // read volume in file voinuoc.txt
            while (1)
            {
                if (Read(&readChar, 1, voinuocId) == -2)
                {
                    Close(voinuocId);
                    break;
                }
                if (readChar != '*' && readChar <= '9' && readChar >= '0')
                {
                    Write(&readChar, 1, voinuocResId);
                    volume = volume * 10 + (readChar - '0');
                }
                else
                {
                    flag_done_result = 1;
                    Close(voinuocId);
                    break;
                }
            }

            if (volume != 0)
            {
                // Dung voi 1
                if (v1 <= v2)
                {
                    v1 += volume;
                    Write(" 1", 2, voinuocResId);
                }
                else // Dung voi 2
                {
                    v2 += volume;
                    Write(" 2", 2, voinuocResId);
                }
            }

            if (flag_done_result == 1)
            {
                // v1 = v2 = 0;
                Close(voinuocResId);
                Signal("sinhvien");
                break;
            }

            if(volume != 0){
                Write("  ", 2, voinuocResId);
            }
            Signal("sinhvien");
        }
        Close(voinuocResId);
    }
}   