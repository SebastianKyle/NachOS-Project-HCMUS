#include "ptable.h"
#include "system.h"
#include "openfile.h"

PTable::PTable(int size)
{
    if (size < 0 || size > MAX_PROCESS)
        size = MAX_PROCESS;

    psize = size;
    bm = new BitMap(size);
    bmsem = new Semaphore("bmsem", 1);

    for (int i = 0; i < psize; i++)
    {
        pcb[i] = NULL;
    }

    bm->Mark(0);
    pcb[0] = new PCB(0);
    pcb[0]->parentID = -1;
}

PTable::~PTable()
{
    if (bm != NULL){
        delete bm;
        bm = NULL;
    }

    for(int i = 0; i < psize; ++i)
    {
        if (pcb[i] != NULL){
            delete pcb[i];
            pcb[i] = NULL;
        }
    }

    if (bmsem != NULL){
        delete bmsem;
        bmsem = NULL;
    }
}

int PTable::ExecUpdate(char *name)
{
    // Gọi mutex->P(); để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
    bmsem->P();

    // Kiểm tra tính hợp lệ của chương trình “name”.
    if (name == NULL)
    {
        printf("\nPTable::Exec : Cannot execute NULL name.\n");
        bmsem->V();
        return -1;
    }

    // Kiểm tra sự tồn tại của chương trình “name”.
    OpenFile *openFile = fileSystem->Open(name);
    if (openFile == NULL)
    {
        printf("\nPTable::Exec : File %s does not exist.\n", name);
        bmsem->V();
        return -1;
    }

    // So sánh tên chương trình và tên của currentThread để chắc chắn rằng chương trình này không gọi thực thi chính nó.
    if (strcmp(name, currentThread->getName()) == 0)
    {
        printf("\nPTable::Exec : Cannot execute itself.\n");
        bmsem->V();
        return -1;
    }

    // Tìm slot trống trong bảng Ptable.
    int index = this->GetFreeSlot();

    // Check if have free slot
    if (index < 0)
    {
        printf("\nPTable::Exec : There is no free slot.\n");
        bmsem->V();
        return -1;
    }

    // Nếu có slot trống thì khởi tạo một PCB mới với processID chính là index của slot này
    pcb[index] = new PCB(index);
    pcb[index]->SetFileName(name);

    // parentID là processID của currentThread
    pcb[index]->parentID = currentThread->processID;


    // Gọi thực thi phương thức Exec của lớp PCB.
    int pid = pcb[index]->Exec(name, index);

    // Gọi bmsem->V()
    bmsem->V();
    // Trả về kết quả thực thi của PCB->Exec.
    return pid;
}

int PTable::JoinUpdate(int id)
{
    // Ta kiểm tra tính hợp lệ của processID id và kiểm tra tiến trình gọi Join có phải là cha của tiến trình
    // có processID là id hay không. Nếu không thỏa, ta báo lỗi hợp lý và trả về -1.
    if (id < 0 || id >= psize)
    {
        printf("\nPTable::JoinUpdate : invalid id = %d", id);
        return -1;
    }
    if (currentThread->processID != pcb[id]->parentID)
    {
        printf("\nPTable::JoinUpdate Cannot join in process which is not it's parent process.\n");
        return -1;
    }

    // Tăng numwait và gọi JoinWait() để chờ tiến trình con thực hiện.
    pcb[pcb[id]->parentID]->IncNumWait();
    pcb[id]->JoinWait();

    // Xử lý exitcode.
    int ec = pcb[id]->GetExitCode();

    // ExitRelease() để cho phép tiến trình con thoát.
    pcb[id]->ExitRelease();

    return ec;
}

int PTable::ExitUpdate(int exitcode)
{
    // Nếu tiến trình gọi là main process thì gọi Halt().
    int id = currentThread->processID;
    if (id == 0)
    {
        currentThread->FreeSpace();
        interrupt->Halt();
        return 0;
    }

    if (IsExist(id) == false)
    {
        printf("\nPTable::ExitUpdate: This %d is not exist. Try again?", id);
        return -1;
    }

    // Ngược lại gọi SetExitCode để đặt exitcode cho tiến trình gọi.
    pcb[id]->SetExitCode(exitcode);
    pcb[pcb[id]->parentID]->DecNumWait();

    // Gọi JoinRelease để giải phóng tiến trình cha đang đợi nó(nếu có) và ExitWait() để xin tiến trình cha cho phép thoát.
    pcb[id]->JoinRelease();
    pcb[id]->ExitWait();

    Remove(id);
    return exitcode;
}

// Find free slot in order to save the new process information
// mark it as used
int PTable::GetFreeSlot()
{
    return bm->Find();
}

// Check if Process ID exists
bool PTable::IsExist(int pid)
{
    return bm->Test(pid);
}

// Remove proccess ID out of table when it ends
void PTable::Remove(int pid)
{
    bm->Clear(pid);
    if (pcb[pid] != 0)
        delete pcb[pid];
}

char *PTable::GetFileName(int id)
{
    return (pcb[id]->GetFileName());
}

