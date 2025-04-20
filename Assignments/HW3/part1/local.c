#include <stdio.h>
#include <omp.h>

int main(){
    int i;
    #pragma omp parallel
    {
        #pragma omp for ordered
        for (i = 0 ; i < 5 ; i++)
            // #pragma omp ordered
            printf("%d\n", i);
    }
}