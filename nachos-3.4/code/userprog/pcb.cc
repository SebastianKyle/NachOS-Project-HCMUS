#include "pcb.h"
#include "utility.h"
#include "system.h"
#include "thread.h"
#include "addrspace.h"

extern void StartProcess_2(int id);

PCB::PCB(int id)
{
    // nếu id = 0 thì đây là tiến trình gốc, nếu không thì đây là tiến trình con.
    if (id == 0)
        this->parentID = -1;
    else
        this->parentID = currentThread->processID;

    this->numwait = this->exitcode = 0;
    this->thread = NULL;

    this->joinsem = new Semaphore("joinsem", 0);
    this->exitsem = new Semaphore("exitsem", 0);
    this->multex = new Semaphore("multex", 1);
}

PCB::~PCB()
{
    if (joinsem != NULL)
        delete this->joinsem;
    if (exitsem != NULL)
        delete this->exitsem;
    if (multex != NULL)
        delete this->multex;
    if (thread != NULL)
    {
        thread->FreeSpace();
        thread->Finish();
    }
}

int PCB::Exec(char *filename, int id)
{
    // Gọi mutex->P(); để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
    multex->P();

    this->thread = new Thread(filename);

    if (this->thread == NULL)
    {
        printf("\nPCB::Exec:: Not enough memory..!\n");
        multex->V();
        return -1;
    }

    this->thread->processID = id;
    this->parentID = currentThread->processID;
    // Gọi thực thi Fork(StartProcess_2,id) => Ta cast thread thành kiểu int, sau đó khi xử ký hàm StartProcess ta cast Thread về đúng kiểu của nó.
    this->thread->Fork(StartProcess_2, id);

    multex->V();

    return id;
}

int PCB::GetID() { return this->thread->processID; }

int PCB::GetNumWait() { return this->numwait; }

int PCB::GetExitCode() { return this->exitcode; }

void PCB::SetExitCode(int ec) { this->exitcode = ec; }


void PCB::JoinWait()
{
    // Gọi joinsem-->P() để tiến trình chuyển sang trạng thái block và ngừng lại, chờ JoinRelease để thực hiện tiếp.
    joinsem->P();
}

void PCB::JoinRelease()
{
    // Gọi joinsem-->V() để giải phóng tiến trình gọi JoinWait().
    joinsem->V();
}

void PCB::ExitWait()
{
    // Gọi exitsem-->P() để tiến trình chuyển sang trạng thái block và ngừng lại, chờ ExitRelease để thực hiện tiếp.
    exitsem->P();
}

void PCB::ExitRelease()
{
    // Gọi exitsem->V() để giải phóng tiến trình đang chờ.
    exitsem->V();
}

void PCB::IncNumWait()
{
    multex->P();
    ++numwait;
    multex->V();
}

void PCB::DecNumWait()
{
    multex->P();
    if (numwait > 0)
        --numwait;
    multex->V();
}

void PCB::SetFileName(char *fn) { strcpy(fileName, fn); }

char *PCB::GetFileName() { return this->fileName; }

