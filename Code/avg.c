
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {

    FILE* ip;
    FILE* op;

    double sum = 0.0, lines = 0;
    char line[50];

    if ((ip = fopen("server_output_time_aggregated","r")) == NULL){
        printf("Error opening the input file: server_output_time_aggregated.\n");
        return 1;
    }

    if ((op = fopen("server_output_time_averaged","a")) == NULL){
        printf("Error opening the output file: server_output_time_averaged.\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), ip))
    {   
        sum += strtod(line, NULL);
        lines++;
    }

    fprintf(op,"%e\n", sum/lines); 
    fclose(ip);
    fclose(op);
    

    return 0;
}