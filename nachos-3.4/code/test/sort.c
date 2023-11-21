/* sort.c
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

int A[101]; /* size of physical memory; with code, we'll run out of space!*/

int 
main()
{
    int i, j, tmp, n;

    PrintString("\n\t\t\t --------SORT-------- \n\n");

    /* User enter size of array */
    PrintString("\n - Enter array size: ");
    n = ReadInt();
    while (n <= 0 || n >= 100) {
        PrintString("\n - Invalid array size, please re-enter: ");
        n = ReadInt();
    }

    /* User enter array elements */
    for (i = 0; i < n; i++) {
        PrintString("\n - Enter number: ");
        A[i] = ReadInt();
    }

    PrintString("\n Before sorting: \n");
    for (i = 0; i < n; i++) {
        PrintChar(' ');
        PrintInt(A[i]);
    }

    /* Sort */
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < (n - i - 1); j++)
            if (A[j] > A[j + 1]) {
                tmp = A[j];
                A[j] = A[j + 1];
                A[j + 1] = tmp;
            }

    PrintString("\n After sorting: \n");
    for (i = 0; i < n; i++) {
        PrintChar(' ');
        PrintInt(A[i]);
    }
    PrintString("\n\n");

    Halt();
    // Exit(0); /* and then we're done -- should be 0! */
}
