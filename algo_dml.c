#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <dirent.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <pthread.h>
#include <ctype.h>
#include <stdbool.h>
#include <regex.h>
#include <assert.h>

char *PATH = "/home/mufis/PS/FP";
char *namaDB = "db_percobaan";

char *argument(int argi, char **arg){
	char *output = malloc(200);
	for (int i=1; i<argi; i++){
		if(i==1){
			strcpy(output, arg[i]);
		}else{
			if(i != argi){
				strcat(output, " ");
			}
			strcat(output, arg[i]);
		}
	}
	return output;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int is_schar(char x, char *schar){
	for (int i=0; i<strlen(schar); i++) {
		if(x == schar[i]) {
			return 1;
		}
	}
	return 0;
}

int is_kol(int x, int *kol_int, int tot){
	for(int i=0; i<tot; i++) {
		if(x == kol_int[i]){
			return 1;
		}
	}
	return 0;
}

char *remove_schar(char *str, char *schar){
	
	char *strStripped = malloc(50);
	int c = 0; 
	for(int i=0; i < strlen(str); i++)
	{
		if (is_schar(str[i], schar) == 0)
		{
			strStripped[c] = str[i];
			c++;
		}
	}
	strStripped[c] = '\0';

	return strStripped;
}

int find_position(int argc, char **argv, char *str){
	for(int i=0; i<argc; i++) {
		if(strcmp(argv[i], str)==0){
			return i;
			break;
		}
	}
	return -1;
}

void select_tbl(int argc, char **argv){
	int from_int = find_position(argc, argv, "FROM");
	int tbl_int = from_int + 1;
	int *kol_int = malloc(50);

	char nama_tbl[50];
	strcpy(nama_tbl, argv[tbl_int]);

	char tbl_str[50];
	strcpy(tbl_str, PATH);
	strcat(tbl_str, "/");
	strcat(tbl_str, namaDB);
	strcat(tbl_str, "/");
	strcat(tbl_str, nama_tbl);
	strcat(tbl_str, ".txt");
	// printf("Nama tabel: %s\n", tbl_str);

	// for(int k=0; k<argc; k++){
	// 	printf("%s\n", argv[k]);
	// }
	
	if(strcmp(argv[from_int-1], "*") == 0){
		char line[100][100];
		char fname[20];
		FILE *fptr = NULL; 
		int i = 0;
		int tot = 0;

		fptr = fopen(tbl_str, "r");
		while(fgets(line[i], 100, fptr)) 
		{
			line[i][strlen(line[i]) - 1] = '\0';
			i++;
		}

		tot = i; 
		int *kolt_int = malloc(50);
		int kolt_c = 0;
		for(int k = 0; k < tot; k++)
		{	
			// printf("%s\n", line[i]);

			char** tokens;
			tokens = str_split(line[k], ',');

			if ((tokens) && (k>=2))
			{
				for (int i = 0; *(tokens + i); i++)
				{
					printf("%s", *(tokens + i));
					if((i < argc)) {
						printf("\t");
					}
				}
			}

			if(k<tot && k>=2){
				printf("\n");
			}
		}

		fclose(fptr);
	
	}

	else{
		int kol_tot = 0;
		for(int i=1; i<from_int; i++){
			kol_int[kol_tot] = i;
			kol_tot++;
		}
		
		char line[100][100];
		char fname[20];
		FILE *fptr = NULL; 
		int i = 0;
		int tot = 0;

		fptr = fopen(tbl_str, "r");
		while(fgets(line[i], 100, fptr)) 
		{
			line[i][strlen(line[i]) - 1] = '\0';
			i++;
		}

		tot = i; 
		int *kolt_int = malloc(50);
		int kolt_c = 0;
		for(int k = 0; k < tot; k++)
		{	
			// printf("%s\n", line[i]);

			char** tokens;
			tokens = str_split(line[k], ',');

			if((tokens) && (k == 0)){
				for (int j=0; j<kol_tot; j++) {
					for(int a=0; *(tokens + a); a++){
						if(strcmp(*(tokens + a), argv[kol_int[j]])==0){
							kolt_int[kolt_c] = a;
							kolt_c++;
						}
					}
				}
			}

			if ((tokens) && (k>=2))
			{
				for (int i = 0; *(tokens + i); i++)
				{
					int cek_k = is_kol(i, kolt_int, kolt_c);
					if(cek_k == 1){
						printf("%s", *(tokens + i));
					}
					if((i < argc) && (cek_k == 1)) {
						printf("\t");
					}
				}
			}

			if(k<tot && k>=2){
				printf("\n");
			}
		}

		fclose(fptr);
	}
}

int cek_command(char *comm){
	if(strcmp(comm, "SELECT") == 0){
		return 4;
	}

	return 0;
}

void run_command(char *comm){

	int tc;
	char schar[3] = {',', '.'};


    char** tokens;
    tokens = str_split(comm, ' ');

    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
			// printf("%s\n", remove_schar(*(tokens + i), schar));
            // printf("%s\n", *(tokens + i));
            // free(*(tokens + i));
			strcpy(*(tokens+i), remove_schar(*(tokens + i), schar));
        }
        // printf("i= %d\n", i);
		tc = i;
        // free(tokens);
    }

	int cek = cek_command(*(tokens + 0));
	if(cek == 4){
		select_tbl(tc, tokens);
	}
}

int main(int argc, char **argv){	
	char *command = malloc(200);
	command = argument(argc, argv);

	// printf("haduh\n");

	run_command(command);
}
