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
char TBLARR[100][100];

// char *argument(int argi, char **arg){
// 	char *output = malloc(200);
// 	for (int i=1; i<argi; i++){
// 		if(i==1){
// 			strcpy(output, arg[i]);
// 		}else{
// 			if(i != argi){
// 				strcat(output, " ");
// 			}
// 			strcat(output, arg[i]);
// 		}
// 	}
// 	return output;
// }

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

char* substr(const char *src, int m, int n) {
    // get the length of the destination string
    int len = n - m;
 
    // allocate (len + 1) chars for destination (+1 for extra null character)
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
 
    // start with m'th char and copy `len` chars into the destination
    strncpy(dest, (src + m), len);
 
    // return the destination string
    return dest;
}

int is_schar(char x, char *schar){
	for (int i=0; i<strlen(schar); i++) {
		if(x == schar[i]) {
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

int find_index(char *arg, char ichar){
	for(int i = 0; i<strlen(arg); i++) {
		if(arg[i]==ichar){
			return i;
		}
	}
	return -1;
}

int ftxt_toArr(char *path){
	char fname[20];
	FILE *fptr = NULL; 
	int i = 0;
	int tot = 0;

	fptr = fopen(path, "r");
	if (fptr == NULL)
    {
        return -1;
    }

	while(fgets(TBLARR[i], 100, fptr)) 
	{
		TBLARR[i][strlen(TBLARR[i]) - 1] = '\0';
		i++;
	}

	fclose(fptr);

	return i;
}

void addStringtoTxt(char *str, char *path){
	FILE *fptr = NULL;
	
	fptr = fopen(path, "a+");
	if (fptr == NULL)
    {	
		printf("Data Gagal Dimasukan!\n");
        return;
    }

	fputs(str, fptr);
	fclose(fptr);
	printf("Data Berhasil Dimasukan!\n");
}

char *fpath_tbl(char *nama_tbl){
	char *tbl_str = malloc(50);
	strcpy(tbl_str, PATH);
	strcat(tbl_str, "/");
	strcat(tbl_str, namaDB);
	strcat(tbl_str, "/");
	strcat(tbl_str, nama_tbl);
	strcat(tbl_str, ".txt");

	return tbl_str;
}

int cek_tipe(char *inpt){
	for(int i=0; i<strlen(inpt); i++) {
		if(inpt[i]<'0' || inpt[i]>'9'){
			return 2;
		}
	}
	return 1;
}

char *str_withoutq(char *inpt){
	char *out = malloc(100);
	int c = 0;
	for(int i=0; i<strlen(inpt); i++){
		if((inpt[i]>='a' && inpt[i]<='z') || (inpt[i]>='A' && inpt[i]<='Z') || 
			(inpt[i]>='0' && inpt[i]<='9') || (inpt[i]==' ' && i != 0)){
			out[c] = inpt[i];
			c++;
		}
	}
	out[c] = '\0';
	return out;
}

void insert_tbl(int argc, char **argv, char *parameter){
	int into_int = find_position(argc, argv, "INTO");
	int tbl_int = into_int + 1;
	int data_int = tbl_int + 1;

	// printf("%s\n", parameter);


	int cek_b = ftxt_toArr(fpath_tbl(argv[tbl_int]));
	int cek_k = 0, param_c = 0; //panjang kolom dari database (cek_k), 
								//panjang kolom parameter command(param_c)

	if(cek_b == -1) {
		printf("Tabel Tidak Ditemukan!\n");
		return;
	}else{
		// isi di dalam database tipenya
		char** tokens;
		tokens = str_split(TBLARR[1], ',');
		for(int i =0; *(tokens + i); i++){
			cek_k++;
		}
		//parameter dari command inseret
		char** params;
		params = str_split(parameter, ',');
		for(int i =0; *(params + i); i++){
			param_c++;
		}

		if(param_c != cek_k){
			printf("Panjang Argumen dengan Kolom Tabel Tidak Sama!\n");
			return;
		}
		char schar[4] = {'(', ')', ' '};
		char saveStr[100];
		for(int i = 0; i<param_c; i++) {
			// printf("string: %s tipe: %d\n", remove_schar(argv[i], schar), 
			//		cek_tipe(remove_schar(argv[i], schar)));

			if((strcmp(*(tokens + i), "int") == 0) && 
				(cek_tipe(remove_schar(*(params + i), schar)) == 1)) {
				if(i == 0){
					strcpy(saveStr, remove_schar(*(params + i), schar));
				}else{
					strcat(saveStr, remove_schar(*(params + i), schar));
				}
				if(i < param_c - 1)
					strcat(saveStr, ",");
			}
			else if((strcmp(*(tokens + i), "string") == 0) && 
				(cek_tipe(remove_schar(*(params + i), schar)) == 2)){
				if(i == 0){
					strcpy(saveStr, str_withoutq(*(params + i)));
				}else{
					strcat(saveStr, str_withoutq(*(params + i)));
				}
				if(i < param_c - 1)
					strcat(saveStr, ",");
			}
			
			else{
				printf("Gagal! Tipe data ke-%d: %s!\n", i+1, *(tokens + i));
				return;
			}
		}
		strcat(saveStr, "\n");
		// printf("save: %s\n", saveStr);
		addStringtoTxt(saveStr, fpath_tbl(argv[tbl_int]));
	}
}

void select_tbl(int argc, char **argv){
	int from_int = find_position(argc, argv, "FROM");
	int tbl_int = from_int + 1;
	int *kol_int = malloc(100);

	if(from_int == 1){
		printf("Format Perintah Salah!\n");
		printf("SELECT [namakolom1, namakolom2...] FROM [namatabel]\n");
		return;
	}

	int cek = ftxt_toArr(fpath_tbl(argv[tbl_int]));
	if(cek == -1) {
		printf("Tabel Tidak Ditemukan!\n");
	}else{

		char** tokens;
		tokens = str_split(TBLARR[0], ',');

		int kol_c = 0;
		if (tokens)
		{
			// nama kolom (sebelum from (fromint - 1)) dicek apakah bintang?
			if(strcmp(argv[from_int - 1], "*") == 0){
				for (int i = 0; *(tokens + i); i++){
					kol_int[kol_c] = i;
					kol_c++;
				}
			}else { //mulai dari command(argv[j]) nested loop database(tokens + i)
				for (int j=1; j<from_int; j++) { //kol_c nya ngikut j urutannya, 
					for(int i = 0; *(tokens + i); i++) { // dan isi dari tokens + i
						if(strcmp(*(tokens + i), argv[j]) == 0) {
							kol_int[kol_c] = i;
							kol_c++;
						}
					}
					
				}
			}
			//print nama kolom
			for(int x=0; x<kol_c; x++){
				printf("%s", *(tokens + kol_int[x]));
				if(x<kol_c - 1)
					printf("\t");
			}
			printf("\n");
		}
		// for(int x=0; x<kol_c; x++){
		// 	printf("urutan jal: %d, str: %s, urutan didb: %d\n", 
		// 	x, *(tokens + kol_int[x]), kol_int[x]);
		// }

		//print isi database, panjang sepanjang cek (dari ftxt_arr)
		for(int x=2; x<cek; x++) {
			char** isiDb;
			isiDb = str_split(TBLARR[x], ',');

			if(isiDb){
				for(int i=0; i<kol_c; i++){ //print urutan sesuai kol_c dari argv(command)
					printf("%s", *(isiDb + kol_int[i]));
					if(i<kol_c - 1)
						printf("\t");
				}
				printf("\n");				
			}
		}

		// for (int i=2; i<cek; i++) {
		// 	printf("%s\n", TBLARR[i]);
		// }
	}
}

void run_command(char *comm){

	char schar[3] = {',', '.', ';'};
	int i;

	char *param = malloc(100);
	strcpy(param, comm);

    char** tokens;
    tokens = str_split(comm, ' ');

    if (tokens) {
        for (i = 0; *(tokens + i); i++) {
            // free(*(tokens + i));
			//buang spesialchar(schar)
			strcpy(*(tokens+i), remove_schar(*(tokens + i), schar));
        }
        // free(tokens);
    }

	// i sebagai panjang dari command yang telah ditokens

	if (strcmp(*(tokens + 0), "INSERT") == 0){
		insert_tbl(i, tokens, substr(param, find_index(param, '('), find_index(param, ')') + 1));
	}
	else if(strcmp(*(tokens + 0), "SELECT") == 0){
		select_tbl(i, tokens);
	}
}

int main(int argc, char **argv){	
	// char *command = malloc(200);
	// command = argument(argc, argv);

	// char command[200] = "SELECT kolom1, nama, nomor FROM table1;";
	// char command[200] = "SELECT nomor, kolom1, nama FROM table1;";
	// char command[200] = "SELECT * FROM table1;";

	// printf("haduh\n");
	// char command[200] = "INSERT INTO table1 (‘kita coba’, ‘value04’, 04);";
	char command[200] = "INSERT INTO table1 (‘kita coba’, 'wwaas', 87);";
	run_command(command);
}