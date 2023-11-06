#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"

unsigned int no_of_prediction = 0;
unsigned int no_of_misprediction = 0;

float misprediction_rate = 0.0;

void bimodal_predictor(bparam parameters, counter_table* counter_tab,unsigned long int addr, char outcome)
{
    // printf("I am here");
    no_of_prediction++;
    int m = parameters.M2;
    bool taken = false;
    

    unsigned int PC = ((addr >> 2)&(counter_tab->total_rows -1));

    unsigned int prediction = counter_tab->counter[PC];

    if(prediction >= 2)
    {
        taken = true;
    }
    
    bool true_outcome_taken = false;
    if(outcome == 't')
    {
        true_outcome_taken = true;
    }

    if(taken!= true_outcome_taken)
    {
        no_of_misprediction++;
    }

    int counter_value = counter_tab->counter[PC];

    if(true_outcome_taken)
    {
        if(counter_tab->counter[PC]<3)
        {
            counter_tab->counter[PC]++;
        }
    }
    else
    {
        if(counter_tab->counter[PC] >0)
        {
            counter_tab->counter[PC]--;
        }
    }
    // counter_value = counter_tab->counter[PC];
}


void gshare_predictor(bparam paramaters, gr *reg,  counter_table* counter_tab,unsigned long int addr, char outcome)
{
    no_of_prediction++;
    int m1 = paramaters.M1;
    int n = paramaters.N;

    int no_bits = ((int)(pow(2, n)));
    int global_history_reg_index = ((reg->reg)&(no_bits -1));

    unsigned int PC = ((addr>>2)&(counter_tab->total_rows-1));

    int pc_upper_bits = (PC>>(m1-n));
    int xor_bits = global_history_reg_index^pc_upper_bits;

    int pc_lower_bits = (PC)&(((int)(pow(2, m1-n))) -1);
    int index = (xor_bits << (m1-n)) | pc_lower_bits;

    unsigned int prediction = counter_tab->counter[index];

    bool taken = false;
    bool actual_outcome_taken = false;

    if(prediction>=2)
    {
        taken = true;
    }

    if(outcome == 't')
    {
        actual_outcome_taken = true;
    }

    if(taken != actual_outcome_taken)
    {
        no_of_misprediction++;
    }

    int counter_value = counter_tab->counter[index];

    if(actual_outcome_taken)
    {
        if(counter_tab->counter[index]<3)
        {
            counter_tab->counter[index]++;
        }

        reg->reg = (reg->reg>>1) | (1<<(n-1));
    }
    else
    {
        if(counter_tab->counter[index]>0)
        {
            counter_tab->counter[index]--;
        }

        reg->reg = (reg->reg>>1) | (0<<(n-1));

    }


}
int main (int argc, char *argv[])
{
    FILE *fp;
    char *trace_file;
    bparam params;
    char status;
    unsigned long int addr;

    // counter_table *counter;
    counter_table *counter;

   

    // if (!(argc == 4))
    // {
    //     printf("Error");
    // }

    params.bp_name = argv[1];
    

    if(strcmp(params.bp_name, "bimodal") == 0)
    {
        params.M2 = strtoul(argv[2], NULL, 10);
        trace_file = argv[3];
        counter = new counter_table(params.M2);
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare")==0)
    {
        params.M1 = strtoul(argv[2], NULL, 10);
        params.N = strtoul(argv[3], NULL, 10);
        trace_file = argv[4];
        counter = new counter_table(params.M1);
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }



    fp = fopen(trace_file, "r");
    if(fp == NULL)
    {
        exit(EXIT_FAILURE);
    }

    char str[2];
    int c = 0;
    gr register1;
    while(fscanf(fp, "%lx %s", &addr, str)!= EOF)
    {
        status = str[0];
        // if (status == 't')
        // {
        //     printf("%lx %s\n", addr, "t");
        // }
        // else
        // {
        //     printf("%lx %s\n", addr, "n");
        // }

        if(strcmp(params.bp_name, "bimodal")==0)
        {
            bimodal_predictor(params, counter, addr, status);
        }
        else if(strcmp(params.bp_name, "gshare")==0)
        {
            gshare_predictor(params, &register1, counter, addr, status);
        }
    }

    printf("OUTPUT\n");
    printf("number of predictions: %d\n", no_of_prediction);
    printf("number of mispredictions: %d\n", no_of_misprediction);
    
    misprediction_rate = 100*((float(no_of_misprediction))/(float(no_of_prediction)));
    printf("misprediction rate: %.2f%c\n", misprediction_rate, '%');

    if(strcmp(params.bp_name, "bimodal")==0)
    {
        printf("FINAL BIMODAL CONTENTS");
        for(int i=0; i<counter->total_rows; i++)
        {
            printf("\n%d %d", i, counter->counter[i]);
        }
    }
    else if(strcmp(params.bp_name, "gshare")==0)
    {
        printf("FINAL GSHARE CONTENTS");
        for(int i=0; i<counter->total_rows; i++)
        {
            printf("\n%d %d", i, counter->counter[i]);
        }
    }

    return 0;

}