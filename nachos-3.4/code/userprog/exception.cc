// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

// Tang Program Counter de tiep tuc nap lenh
void IncreasePC()
{
    int counter = machine->ReadRegister(PCReg);

    machine->WriteRegister(PrevPCReg, counter);
    counter = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, counter);
    machine->WriteRegister(NextPCReg, counter + 4);
}

// Input: Khong gian dia chi User(int) - gioi han cua buffer(int)
// Output: Bo nho dem Buffer(char*)
// Chuc nang: Sao chep vung nho User sang vung nho System
char *User2System(int virtAddr, int limit)
{
    int i;
    int oneChar;
    char *kernelBuff = NULL;
    kernelBuff = new char[limit + 1];
    if (kernelBuff == NULL)
    {
        return kernelBuff;
    }

    memset(kernelBuff, 0, limit + 1);

    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuff[i] = (char)oneChar;
        if (oneChar == 0)
            break;
    }

    return kernelBuff;
}

// Input: Khong gian vung nho User(int) - gioi han cua buffer(int) - bo nho dem buffer(char*)
// Output: So byte da sao chep(int)
// Chuc nang: Sao chep vung nho System sang vung nho User
int System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;

    int i = 0;
    int oneChar = 0;

    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);

    return i;
}

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    // Xu ly exception
    switch (which)
    {
    case NoException:
        return;

    case PageFaultException:
        DEBUG('a', "\n No valid translation found.");
        printf("\n\n No valid translation found.");
        interrupt->Halt();
        break;

    case ReadOnlyException:
        DEBUG('a', "\n  Write attempted to page marked read-only.");
        printf("\n\n Write attempted to page marked read-only.");
        interrupt->Halt();
        break;

    case BusErrorException:
        DEBUG('a', "\n Translation resulted valid physical address.");
        printf("\n\n Translation resulted valid physical address.");
        interrupt->Halt();
        break;

    case AddressErrorException:
        DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space.");
        printf("\n\n Unaligned reference or one that was beyond the end of the address space.");
        interrupt->Halt();
        break;

    case OverflowException:
        DEBUG('a', "\nInteger overflow in add or sub.");
        printf("\n\n Integer overflow in add or sub.");
        interrupt->Halt();
        break;

    case IllegalInstrException:
        DEBUG('a', "\n Unimplemented or reserved instr.");
        printf("\n\n Unimplemented or reserved instr.");
        interrupt->Halt();
        break;

    case NumExceptionTypes:
        DEBUG('a', "\n Number exception types.");
        printf("\n\n Number exception types.");
        interrupt->Halt();
        break;

    case SyscallException:
        char *buffer;
        int virtAddr;
        bool isNegative = false;
        int firstIndex = 0;
        int numBytes;
        int length;

        const int MAX_BUFFER_SIZE = 255;
        const int MAX_FILE_LENGTH = 32;

        switch (type)
        {
        case SC_Halt:
        {
            // Input: khong
            // Output: thong bao tat may
            // Chuc nang: tat he dieu hanh

            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            return;
        }

        case SC_ReadInt:
        {
            // Input: khong
            // Output: so nguyen doc duoc tu console
            // Chuc nang: doc so nguyen tu console

            int result = 0;

            buffer = new char[MAX_BUFFER_SIZE + 1];

            // Doc buffer tu console voi toi da 255 ki tu
            numBytes = gSynchConsole->Read(buffer, MAX_BUFFER_SIZE);

            isNegative = false;
            firstIndex = 0;    // Index cua ki tu dau tien cua so trong buffer
            int lastIndex = 0; // Index cua ki tu cuoi cung cua so trong buffer

            // Bo qua ki tu space
            while (buffer[firstIndex] == ' ')
            {
                firstIndex++;
                lastIndex++;
            }

            if (buffer[firstIndex] == '-')
            {
                isNegative = true;
                firstIndex++;
                lastIndex++;
            }

            for (int i = firstIndex; i < numBytes; i++)
            {
                // Truong hop so 0 sau dau thap phan (vd: 12.0)
                if (buffer[i] == '.')
                {
                    for (int j = i + 1; j < numBytes; j++)
                    {
                        // Toan bo phai la 0 sau dau thap phan
                        if (buffer[j] != '0')
                        {
                            printf("\n\n Invalid integer.");
                            DEBUG('a', "\n Invalid integer.");

                            machine->WriteRegister(2, 0);
                            IncreasePC();

                            delete buffer;
                            return;
                        }
                    }

                    lastIndex = i - 1; // Ngay truoc dau thap phan
                    break;
                }
                else if (buffer[i] < '0' || buffer[i] > '9')
                {
                    printf("\n\n Invalid integer. Take 0 as value.");
                    DEBUG('a', "\n Invalid integer.");

                    machine->WriteRegister(2, 0);
                    IncreasePC();

                    delete buffer;
                    return;
                }

                lastIndex = i;
            }

            // Chuyen chuoi ve so nguyen
            const int maxInt32 = 2147483647; // truong hop tran so
            for (int i = firstIndex; i <= lastIndex; i++)
            {
                result = (result * 10 + (int)(buffer[i] - 48)) % maxInt32;
            }

            if (isNegative)
            {
                result = -result;
            }

            machine->WriteRegister(2, result);
            IncreasePC();

            delete buffer;
            return;
        }

        case SC_PrintInt:
        {
            // Input: 1 so nguyen
            // Output: khong
            // Chuc nang: in so nguyen ra console

            // Doc so nguyen to thanh ghi R4
            int number = machine->ReadRegister(4);

            // Neu la 0
            if (number == 0)
            {
                gSynchConsole->Write("0", 1);
                IncreasePC();
                return;
            }

            isNegative = false;
            int numDigits = 0;
            firstIndex = 0;

            if (number < 0)
            {
                isNegative = true;
                number = -number;
                firstIndex = 1;
            }

            // Dem so chu so
            int temp = number;
            while (temp)
            {
                numDigits++;
                temp /= 10;
            }

            // Tao chuoi de in ra console
            buffer = new char[MAX_BUFFER_SIZE + 1];

            for (int i = firstIndex + numDigits - 1; i >= firstIndex; i--)
            {
                buffer[i] = (char)((number % 10) + 48);
                number /= 10;
            }

            if (isNegative)
            {
                buffer[0] = '-';
                buffer[numDigits + 1] = '\0';
                gSynchConsole->Write(buffer, numDigits + 1);

                delete buffer;
                IncreasePC();

                return;
            }

            buffer[numDigits] = '\0';
            gSynchConsole->Write(buffer, numDigits);

            delete buffer;
            IncreasePC();

            return;
        }

        case SC_ReadChar:
        {
            // Input: khong
            // Output: 1 ky tu nguoi dung nhap vao
            // Chuc nang: doc 1 ky tu do nguoi dung nhap vao

            buffer = new char[MAX_BUFFER_SIZE + 1];
            numBytes = gSynchConsole->Read(buffer, MAX_BUFFER_SIZE);

            if (numBytes > 1)
            { // Nhap nhieu hon 1 ky tu
                printf("\n\n Only 1 character allowed.");
                DEBUG('a', "\n ERROR: Only 1 character allowed.");
                machine->WriteRegister(2, 0);
            }
            else if (numBytes == 0)
            { // ky tu rong
                printf("\n\n Empty buffer.");
                DEBUG('a', "\n ERROR: empty buffer.");
                machine->WriteRegister(2, 0);
            }
            else
            {
                char c = buffer[0];
                machine->WriteRegister(2, c);
            }

            delete buffer;
            break;
        }

        case SC_PrintChar:
        {
            // Input: 1 ki tu
            // Output: 1 ki tu
            // Chuc nang: xuat 1 ki tu ra console

            // Doc ki tu tu thanh ghi R4
            char c = machine->ReadRegister(4);
            gSynchConsole->Write(&c, 1);

            break;
        }

        case SC_ReadString:
        {
            // Input: buffer, do dai chuoi
            // Output: khong
            // Chuc nang: doc mot chuoi ki tu vao trong buffer

            char *buffer;

            virtAddr = machine->ReadRegister(4); // dia chi cua tham so buffer tu thanh ghi R4
            length = machine->ReadRegister(5);   // do dai toi da cua chuoi tu thanh ghi R5

            buffer = User2System(virtAddr, length); // Copy chuoi tu UserSpace sang SystemSpace
            gSynchConsole->Read(buffer, length);    // Doc chuoi tu console
            System2User(virtAddr, length, buffer);  // Copy chuoi tu SystemSpace sang UserSpace

            delete buffer;
            IncreasePC();

            return;
        }

        case SC_PrintString:
        {
            // Input: buffer
            // Output: chuoi doc duoc tu buffer
            // Chuc nang: in chuoi ki tu trong buffer ra man hinh

            virtAddr = machine->ReadRegister(4);             // doc dia chi cua buffer to thanh ghi R4
            buffer = User2System(virtAddr, MAX_BUFFER_SIZE); // copy chuoi to UserSpace sang SystemSpace voi buffer dai max 255 ki tu

            // Do dai that cua chuoi
            length = 0;
            while (buffer[length] != 0)
            {
                length++;
            }

            gSynchConsole->Write(buffer, length + 1);

            delete buffer;
            break;
        }

        case SC_Create:
        {
            // Input: Dia chi vung nho cua ten file
            // Output: -1: Loi | 0: Thanh cong
            // Chuc nang: tao file moi voi tham so la ten file

            char *filename;
            DEBUG('a', "\n SC_Create call ...");
            DEBUG('a', "\n Reading virtual address of filename");

            virtAddr = machine->ReadRegister(4);
            DEBUG('a', "\n Reading filename.");

            filename = User2System(virtAddr, MAX_FILE_LENGTH + 1);
            if (strlen(filename) == 0)
            {
                printf("\n File name is invalid.");
                DEBUG('a', "\n File name is invalid");
                machine->WriteRegister(2, -1); // Loi, tra ve -1 trong thanh ghi R2

                break;
            }

            // Khong du vung nho de doc
            if (filename == NULL)
            {
                printf("\n Not enough memory in system.");
                DEBUG('a', "\n Not enough memory in system.");
                machine->WriteRegister(2, -1); // Tra ve -1 vao thanh ghi R2
                delete filename;

                break;
            }

            DEBUG('a', "\n Finish reading filename.");

            // Tao file moi voi size = 0 su dung ham Create() cua fileSystem
            if (!fileSystem->Create(filename, 0))
            {
                printf("\n Error creating file '%s'", filename);
                machine->WriteRegister(2, -1);
                delete filename;

                break;
            }

            // Tao file thanh cong, tra ve 0 cho thanh ghi R2
            machine->WriteRegister(2, 0);
            delete[] filename;

            break;
        }

        case SC_Open:
        {
            // Input: Dia chi vung nho cua ten file, type
            // Output: -1: Loi | OpenFileID: Thanh cong
            // Chuc nang: mo file voi tham so la ten file va type

            int virtAddr = machine->ReadRegister(4); // Lay dia chi ten file
            int type = machine->ReadRegister(5);     // Lay tham so type
            char *filename;
            filename = User2System(virtAddr, MAX_FILE_LENGTH);

            int freeSlot = fileSystem->FindFreeSlot();
            if (freeSlot != -1) // Tim duoc slot trong
            {
                if (type == 0 || type == 1) // xu li file doc ghi hoac chi doc
                {
                    if (fileSystem->Open(filename, type) != NULL) // Mo file thanh cong
                    {
                        fileSystem->SetOpenFile(freeSlot, fileSystem->Open(filename, type)); // Gan file vao bang mo ta file
                        machine->WriteRegister(2, freeSlot);                                 // tra ve OpenFileID chinh la vi tri cua file trong bang mo ta file
                    }
                    else // Mo file that bai
                    {
                        printf("\nCan not open file.");
                        DEBUG('a', "\nCan not open file.");
                        machine->WriteRegister(2, -1); // Tra ve -1 loi
                    }
                }
                else if (type == 2) // xu li stdin
                {
                    machine->WriteRegister(2, 0);
                }
                else if (type == 3) // xu li stdout
                {
                    machine->WriteRegister(2, 1);
                }
                else
                {
                    printf("\nInvalid type.");
                    DEBUG('a', "\nInvalid type.");
                    machine->WriteRegister(2, -1); // Tra ve -1 loi
                }
                delete[] filename;
                break;
            }
            machine->WriteRegister(2, -1); // Khong mo duoc file return -1

            delete[] filename;
            break;
        }

        case SC_Close:
        {
            // Input: OpenFileID
            // Output: -1: Loi | 0: Thanh cong
            // Chuc nang: dong file voi tham so la id cua file

            int openFileID = machine->ReadRegister(4); // Lay tham so OpenFileID
            if (openFileID == 0 || openFileID == 1)    // Kiem tra file co phai stdin hoac stdout khong
            {
                printf("\nCan not close stdin or stdout.");
                DEBUG('a', "\nCan not close stdin or stdout.");
                machine->WriteRegister(2, -1); // Tra ve -1 loi
                break;
            }

            if (fileSystem->GetOpenFile(openFileID) != NULL) // Kiem tra file co ton tai khong
            {
                fileSystem->CloseOpenFile(openFileID); // Dong file
                machine->WriteRegister(2, 0);          // Tra ve 0 thanh cong
                break;
            }
            machine->WriteRegister(2, -1); // Tra ve -1 loi
            break;
        }

        case SC_Read:
        {
            // Input: buffer, so byte can doc, OpenFileID
            // Output: -1: Loi | so byte doc duoc: Thanh cong | -2: Doc ki tu EOF
            // Chuc nang: doc file voi tham so la buffer, so byte can doc, id cua fle

            int virtAddr = machine->ReadRegister(4);   // Lay dia chi cua tham so buffer
            int numBytes = machine->ReadRegister(5);   // Lay so byte can doc
            int openFileID = machine->ReadRegister(6); // Lay tham so OpenFileID

            // Kiem tra OpenFileID
            if (openFileID < 0 || openFileID > 10)
            {
                printf("\nInvalid OpenFileID.");
                DEBUG('a', "\nInvalid OpenFileID.");
                machine->WriteRegister(2, -1);
                break;
            }

            // Kiem tra file co ton tai khong   
            if (fileSystem->GetOpenFile(openFileID) == NULL)
            {
                printf("\nFile not found.");
                DEBUG('a', "\nFile not found.");
                machine->WriteRegister(2, -1);
                break;
            }

            // Kiem tra file co phai stdout khong
            if (fileSystem->GetOpenFile(openFileID)->getType() == 3)
            {
                printf("\nCan not read from stdout.");
                DEBUG('a', "\nCan not read from stdout.");
                machine->WriteRegister(2, -1);
                break;
            }

            char *buffer = new char[numBytes + 1]; // Tao bo dem de doc file

            // Kiem tra file co phai stdin khong
            if (fileSystem->GetOpenFile(openFileID)->getType() == 2)
            {
                int charCount = gSynchConsole->Read(buffer, numBytes);
                System2User(virtAddr, charCount, buffer);
                if (charCount == -1)
                {
                    machine->WriteRegister(2, -2); // Doc ki tu EOF
                }
                else
                {
                    machine->WriteRegister(2, charCount);
                }
                delete[] buffer;
                break;
            }

            if (fileSystem->GetOpenFile(openFileID)->getType() == 0 || fileSystem->GetOpenFile(openFileID)->getType() == 1)
            {
                int charCount = fileSystem->GetOpenFile(openFileID)->Read(buffer, numBytes);
                if (charCount > 0)
                {
                    System2User(virtAddr, charCount, buffer);
                    machine->WriteRegister(2, charCount);
                }
                else
                {
                    machine->WriteRegister(2, -2); // Doc file rong
                }
            }

            delete[] buffer;
            break;
        }

        case SC_Write:
        {
            // Input: buffer, so byte can ghi, OpenFileID
            // Output: -1: Loi | so byte ghi duoc: Thanh cong
            // Chuc nang: ghi file voi tham so la buffer, so byte can ghi, OpenFileID

            int virtAddr = machine->ReadRegister(4);   // Lay dia chi cua tham so buffer
            int numBytes = machine->ReadRegister(5);   // Lay so byte can ghi
            int openFileID = machine->ReadRegister(6); // Lay tham so OpenFileID

            // Kiem tra OpenFileID
            if (openFileID < 0 || openFileID > 10)
            {
                printf("\nInvalid OpenFileID.");
                DEBUG('a', "\nInvalid OpenFileID.");
                machine->WriteRegister(2, -1);
                break;
            }

            // Kiem tra file co ton tai khong
            if (fileSystem->GetOpenFile(openFileID) == NULL)
            {
                printf("\nFile not found.");
                DEBUG('a', "\nFile not found.");
                machine->WriteRegister(2, -1);
                break;
            }

            // Kiem tra file co phai stdin hoac chi doc (type = 1)
            if (fileSystem->GetOpenFile(openFileID)->getType() == 2 || fileSystem->GetOpenFile(openFileID)->getType() == 1)
            {
                printf("\nCan not write to stdin or read only file.");
                DEBUG('a', "\nCan not write to stdin or read only file.");
                machine->WriteRegister(2, -1);
                break;
            }

            char *buffer = User2System(virtAddr, numBytes); // Copy chuoi tu UserSpace sang SystemSpace

            // Kiem tra file co phai stdout khong
            if (fileSystem->GetOpenFile(openFileID)->getType() == 3)
            {
                int i = 0;
                while (buffer[i] != 0 && buffer[i] != '\n' && i < numBytes) // Vong lap de write den khi gap ky tu '\n'
                {
                    i++;
                }
                i = gSynchConsole->Write(buffer, i);
                buffer[i] = '\n';
                gSynchConsole->Write(buffer + i, 1); // Write ky tu '\n'
                machine->WriteRegister(2, i - 1);    // Tra ve so byte thuc su write duoc
                delete[] buffer;
                return;
            }

            // Kiem tra file co phai read & write (type = 0)
            if (fileSystem->GetOpenFile(openFileID)->getType() == 0)
            {
                int charCount = fileSystem->GetOpenFile(openFileID)->Write(buffer, numBytes);
                if (charCount > 0)
                {
                    machine->WriteRegister(2, charCount);
                }
                else
                {
                    machine->WriteRegister(2, -1); // Ghi file that bai
                }
            }

            delete[] buffer;
            break;
        }

        case SC_Seek:
        {
            // Input: vi tri can chuyen con tro den trong file, id cua file
            // Output: -1: Loi | vi tri thuc su trong file: Thanh cong
            // Chuc nang: chuyen con tro den trong file voi tham so la vi tri can chuyen con tro den trong file, id cua file

            int pos = machine->ReadRegister(4); // Lay vi tri can chuyen con tro den trong file
            int id = machine->ReadRegister(5);  // Lay id cua file
            // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
            if (id < 0 || id > 14)
            {
                printf("\nInvalid OpenFileID.");
                machine->WriteRegister(2, -1);
                break;
            }
            // Kiem tra file co ton tai khong
            if (fileSystem->GetOpenFile(id) == NULL)
            {
                printf("\nFile not found.");
                machine->WriteRegister(2, -1);
                break;
            }

            // Kiem tra stdin va stdout
            if (id == 0 || id == 1)
            {
                printf("\nCan not seek stdin or stdout.");
                machine->WriteRegister(2, -1);
                break;
            }

            // Neu pos = -1 thi gan pos = Length nguoc lai thi giu nguyen pos
            pos = (pos == -1) ? fileSystem->GetOpenFile(id)->Length() : pos;

            if (pos > fileSystem->GetOpenFile(id)->Length() || pos < 0) // Kiem tra lai vi tri pos co hop le khong
            {
                printf("\nInvalid position.");
                machine->WriteRegister(2, -1);
            }
            // Hop le
            else
            {
                fileSystem->GetOpenFile(id)->Seek(pos);
                machine->WriteRegister(2, pos);
            }

            break;
        }
        case SC_Exec:
        {
            // Input: Dia chi vung nho cua ten file
            // Output: -1: Loi | SpaceID: Thanh cong
            // Chuc nang: thuc thi file voi tham so la ten file
            int virtAddr = machine->ReadRegister(4);                    // Lay dia chi cua tham so buffer
            char *processname = User2System(virtAddr, MAX_FILE_LENGTH); // Copy chuoi tu UserSpace sang SystemSpace

            //
            if (processname == NULL)
            {
                DEBUG('a', "\n File name is NULL.");
                printf("\n File name is NULL.");
                machine->WriteRegister(2, -1);
                break;
            }

            // Kiem tra file co ton tai khong
            if (fileSystem->Open(processname, 0) == NULL)
            {
                printf("\nFile not found.");
                DEBUG('a', "\nFile not found.");
                machine->WriteRegister(2, -1);
                delete[] processname;
                break;
            }

            int id = pTab->ExecUpdate(processname);
            machine->WriteRegister(2, id);

            delete[] processname;
            break;
        }

        case SC_Join:
        {
            // Input: SpaceID
            // Output: -1: Loi | exitcode
            // Chuc nang: cho tien trinh voi tham so la SpaceID ket thuc
            int id = machine->ReadRegister(4);   // Lay tham so SpaceID
            int exitCode = pTab->JoinUpdate(id); // Goi ham JoinUpdate de cho tien trinh ket thuc

            machine->WriteRegister(2, exitCode);
            break;
        }

        case SC_Exit:
        {
            // Input: exitcode
            // Output: khong
            // Chuc nang: ket thuc tien trinh voi tham so la exitcode

            int joinExitCode = machine->ReadRegister(4);

            if (joinExitCode != 0)
            {
                break;
            }

            int res = pTab->ExitUpdate(joinExitCode);

            currentThread->FreeSpace();
            currentThread->Finish();
            break;
        }

        case SC_CreateSemaphore:
        {
            // Input: Dia chi vung nho cua ten semaphore, semval
            // Output: -1: Loi | 0: Thanh cong
            // Chuc nang: tao semaphore voi tham so la ten semaphore va

            int virtAddr = machine->ReadRegister(4); // Lay dia chi ten semaphore
            int semval = machine->ReadRegister(5);   // Lay tham so semval

            char *name = User2System(virtAddr, MAX_FILE_LENGTH + 1); // Copy chuoi tu UserSpace sang SystemSpace
            if (name == NULL)
            {
                DEBUG('a', "\n ");
                printf("\n Not enough memory in System");
                machine->WriteRegister(2, -1);
                break;
            }

            int res = semTab->Create(name, semval);

            if (res == -1)
            {
                DEBUG('a', "\n Can not create semaphore.");
                printf("\n Can not create semaphore.");
                machine->WriteRegister(2, -1);
                delete[] name;
                break;
            }

            machine->WriteRegister(2, res);
            delete[] name;
            break;
        }

        case SC_Wait:
        {
            // Input: Dia chi vung nho cua ten semaphore
            // Output: -1: Loi | 0: Thanh cong
            // Chuc nang: thuc thi wait voi tham so la ten semaphore

            int virtAddr = machine->ReadRegister(4);

            char *name = User2System(virtAddr, MAX_FILE_LENGTH + 1);
            if (name == NULL)
            {
                DEBUG('a', "\n Not enough memory in System");
                printf("\n Not enough memory in System");
                machine->WriteRegister(2, -1);
                delete[] name;
                break;
            }

            int res = semTab->Wait(name);

            if (res == -1)
            {
                DEBUG('a', "\n Semaphore not found.");
                printf("\n Semaphore not found.");
                machine->WriteRegister(2, -1);
                delete[] name;
                break;
            }

            machine->WriteRegister(2, res);
            delete[] name;
            break;
        }

        case SC_Signal:
        {
            // Input: Dia chi vung nho cua ten semaphore
            // Output: -1: Loi | 0: Thanh cong
            // Chuc nang: thuc thi signal voi tham so la ten semaphore

            int virtAddr = machine->ReadRegister(4);

            char *name = User2System(virtAddr, MAX_FILE_LENGTH + 1);
            if (name == NULL)
            {
                DEBUG('a', "\n Not enough memory in System");
                printf("\n Not enough memory in System");
                machine->WriteRegister(2, -1);
                delete[] name;
                break;
            }

            int res = semTab->Signal(name);

            if (res == -1)
            {
                DEBUG('a', "\n Semaphore not found.");
                printf("\n Semaphore not found.");
                machine->WriteRegister(2, -1);
                delete[] name;
                break;
            }

            machine->WriteRegister(2, res);
            delete[] name;
            break;
        }

        default:
        {
            printf("Unexpected user mode exception %d %d\n", which, type);

            ASSERT(0);
            interrupt->Halt();

            break;
        }
        }

        IncreasePC();
        break;
    }
}
