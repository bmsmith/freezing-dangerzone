#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>



    void inc_n(int *n)
    {
        /* increment n to 100 */
        while(++(*n) < 100);
    }


int main()
{
    int x, y = 0;
    // printf("x: %d, y: %d\n", x, y);
    
    pid_t pid = fork();
    if (pid == 0)
    {
        int inc_x = 0;
        // Child process
        inc_n(&inc_x);
        printf("x increment finished\n");
        exit(inc_x);
    }
    else
    {
        inc_n(&y);
        printf("y increment finished\n");
        printf("%d", wait(&x));
        // Parent Process
    }
    
    printf("x: %d, y: %d\n", x, y);
        
        
            return 0;
}

