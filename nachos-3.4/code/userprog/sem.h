#ifndef SEM_H
#define SEM_H
#include "synch.h"

class Sem
{
private:
    char name[50];
    Semaphore *sem; // Tao semaphore de quan ly
public:
    // khởi tạo đối tượng Sem. Gán giá trị ban đầu là null
    // nhớ khởi tạo bm sử dụng
    Sem(char *na, int i);

    ~Sem();

    void wait();

    void signal();

    char *GetName();
};

#endif // SEM_H