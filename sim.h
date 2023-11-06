#ifndef SIM_H
#define SIM_H
#include <math.h>

typedef struct bparam{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char* bp_name;
}bparam;

class counter_table
{
    public:
        int total_rows;
        int *counter;

        counter_table(int M2)
        {
            total_rows = (int)(pow(2, M2));
            counter = new int[total_rows]();

            for(int i=0; i<total_rows; i++)
            {
                counter[i]=2;
            }
        }
};

typedef struct global_register
{
    unsigned long int reg = 0;
}gr;

#endif
