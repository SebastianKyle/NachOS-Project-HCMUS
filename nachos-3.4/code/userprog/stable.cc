#include "stable.h"

STable::STable()
{
    bm = new BitMap(MAX_SEMAPHORE);

    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        semTab[i] = NULL;
    }
}

STable::~STable()
{
    if (bm)
    {
        delete bm;
        bm = NULL;
    }
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        if (semTab[i])
        {
            delete semTab[i];
            semTab[i] = NULL;
        }
    }
}

int STable::Create(char *name, int init)
{
    // Kiem tra name da ton tai chua
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        if (bm->Test(i))
        {
            if (strcmp(name, semTab[i]->GetName()) == 0)
            {
                return -1;
            }
        }
    }

    // Tim slot trong
    int id = this->FindFreeSlot();

    // Neu khong tim thay slot trong thi tra ve -1
    if (id < 0) return -1;

    // Neu tim thay slot trong thi tao semaphore moi va nap vao semTab
    this->semTab[id] = new Sem(name, init);
    return 0;
}

int STable::Wait(char *name)
{
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        // Kiem tra o thu i da duoc nap semaphore chua
        if (bm->Test(i))
        {
            if (strcmp(name, semTab[i]->GetName()) == 0)
            {
                // Neu ton tai thi cho semaphore down();
                semTab[i]->wait();
                return 0;
            }
        }
    }

    return -1;
}

int STable::Signal(char *name)
{
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        // Kiem tra o thu i da duoc nap semaphore chua
        if (bm->Test(i))
        {
            if (strcmp(name, semTab[i]->GetName()) == 0)
            {
                // Neu ton tai thi cho semaphore up();
                semTab[i]->signal();
                return 0;
            }
        }
    }
    return -1;
}

int STable::FindFreeSlot()
{
    return bm->Find();
}