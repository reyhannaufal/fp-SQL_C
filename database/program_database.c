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
#include <time.h>
#include <wait.h>
#include <fcntl.h>
#include <limits.h>
#include <syslog.h>
#include <errno.h>
#include <assert.h>

#define PORT 8080
#define buffSize 256

//DML
char *PATH = "/home/mufis/PS/FP";
char *namaDB = "db_percobaan";
char TBLARR[100][100];
char OUTCLIENT[buffSize][buffSize];
int OCLENGTH = 0;

typedef struct user{
	char username[buffSize],password[buffSize],input[buffSize];
	int sock,isRoot;
} user;
user* client[10];

//DML Fungsi
int findMax_strTbl(char *namaTbl);
char *fpath_tbl(char *nama_tbl);
int ftxt_toArr(char *path);
char *str_withoutq(char *inpt);
char *fpath_tbl(char *nama_tbl);
int ftxt_toArr(char *path);
char *str_withoutq(char *inpt);
char *tokens_toStr(char **arg, int src, int dest, char *delim);
char** str_split(char* a_str, const char a_delim);
char* substr(const char *src, int m, int n);
int is_schar(char x, char *schar);
char *remove_schar(char *str, char *schar);
int find_position(int argc, char **argv, char *str);
int find_index(char *arg, char ichar);
int fline_where(int argc, char **argv, int whereC, int *kolom);
void addStringtoTxt(char *str, char *path, char *type, int tipe_call);
int cek_tipe(char *inpt);
void delete_tbl(int argc, char **argv, int line);
void insert_tbl(int argc, char **argv, char *parameter);
void select_tbl(int argc, char **argv);
void run_command(char *comm);

void* inRoutine(void *arg){
	int i = *(int*) arg-1;
	char buffer[buffSize];
	while (1){
		recv(client[i]->sock,buffer,buffSize,0);
		strcpy(client[i]->input, buffer);
	}
}

void* outRoutine(void *arg){
	int i = *(int*) arg-1;
	char buffer[256];
	char buffer_name[256];
	char path[256];
	while(1){
		//DDL DML below
		if(strlen(client[i]->input)!= 0){
			// send(client[i]->sock,client[i]->input,buffSize,0);
			// send(client[i]->sock,"\n",buffSize,0);
			run_command(client[i]->input);
			if(OCLENGTH != 0){
				for(int j=0; j<OCLENGTH; j++){
					send(client[i]->sock,OUTCLIENT[j],buffSize,0);
					strcpy(OUTCLIENT[j], "\0");
				}
				send(client[i]->sock,"\n",buffSize,0);
			}
			OCLENGTH = 0;

			memset(client[i]->input,0,256);
		}
	}
}


// void killer(int pid);

int main(int argc,char* argv[])
{

	char s[PATH_MAX];
	getcwd(s,sizeof(s));
	int indexClient = 0;
	
	pthread_t clientThread[10][2];
	
	pid_t pid,sid;
	pid = fork();
	
	if(pid <0){
		exit(EXIT_FAILURE);
	}
	if(pid > 0){
		exit(EXIT_SUCCESS);
	}
	umask(0);
	sid = setsid();
	if(sid < 0){
		exit(EXIT_FAILURE);
	}
	if((chdir(s))<0){
		exit(EXIT_FAILURE);
	}
	
	
//	close(STDIN_FILENO);
//	close(STDOUT_FILENO);
//	close(STDERR_FILENO);
	
	// killer((int)getpid());
	while(1){
		int opt = 1;
		int server_fd, valread;
		struct sockaddr_in address;
		int addrlen = sizeof(address);

		  
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		    perror("socket failed");
		    exit(EXIT_FAILURE);
		}
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		    perror("setsockopt");
		    exit(EXIT_FAILURE);
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons( PORT );
		  
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
		    perror("bind failed");
		    exit(EXIT_FAILURE);
		}

		if (listen(server_fd, 10) < 0) {
		    perror("listen");
		    exit(EXIT_FAILURE);
		}
		for(indexClient=0;indexClient<10;indexClient++){
			client[indexClient] = (user*)malloc(sizeof(user));
			if ((client[indexClient]->sock = accept(server_fd,(struct sockaddr*)&address,(socklen_t*)&addrlen))<0){
				perror("accept");
		    	exit(EXIT_FAILURE);
			}
			pthread_create(&clientThread[indexClient][0],NULL,inRoutine,(void*)&indexClient);
			pthread_create(&clientThread[indexClient][1],NULL,outRoutine,(void*)&indexClient);
		}
		for(indexClient=0;indexClient<10;indexClient++){

			pthread_join(clientThread[indexClient][0],NULL);
			pthread_join(clientThread[indexClient][1],NULL);
		}
	}
	//return 0;
}


// mkae killer for easy termination
// void killer(int pid){
// 	FILE *tex;
// 	char* script;
// 	tex = fopen("Killer.sh", "w");
// 	asprintf(&script,"\#!/bin/bash\nkill -9 %d\nrm -- \"$0\"",pid);
// 	fputs(script,tex);
// 	fclose(tex);
// }

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

char *tokens_toStr(char **arg, int src, int dest, char *delim){
	char *output = malloc(200);
	for (int i=src; i<dest; i++){
		if(i==src){
			strcpy(output, arg[i]);
		}else{
			if(i != dest){
				strcat(output, delim);
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

    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    count += last_comma < (a_str + strlen(a_str) - 1);
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
    int len = n - m;
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
    strncpy(dest, (src + m), len);
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

int fline_where(int argc, char **argv, int whereC, int *kolom){
	if(whereC == -1){
		return -1;
	}else{
		int tbl_int = whereC - 1;
		int cek_b = ftxt_toArr(fpath_tbl(argv[tbl_int]));
		if(cek_b == -1){
			return -2; //table tidak ditemukan
		}
		char param_where[100]; //menyatukan tokens jadi string lagi dgn delim
		strcpy(param_where, tokens_toStr(argv, whereC + 1, argc, " "));

		char **token_where;
		token_where = str_split(param_where, '=');

		int kol_n = -3; //mencari kolom pada 
		for (int i=0; i<cek_b; i++){
			char **tblline;
			tblline = str_split(TBLARR[i], ',');
			if(i==0){ //nyari kolom pada urutan ke berapa
				for (int j=0; *(tblline + j); j++){
					if(strcmp(*(tblline + j), *(token_where + 0)) == 0){
						kol_n = j;
						*kolom = j;
						printf("kolom asli: %d\n", kol_n);
						break;
					}
				}
			}
			else if(kol_n == -3) {
				return -3; //tidak ditemukan nama kolom
			}else{ // nyari line
				if(strcmp(str_withoutq(*(token_where + 1)), *(tblline + kol_n)) == 0){
					// printf("namap: %s, namadb: %s\n", str_withoutq(*(token_where + 1)), *(tblline + kol_n));
					return i;
					break;
				}
			}
		}
		return -4; //tidak ditemukan data
	}
	return -1;
}

void addStringtoTxt(char *str, char *path, char *type, int tipe_call){
	FILE *fptr = NULL;
	
	fptr = fopen(path, type);
	if (fptr == NULL)
    {	
		// printf("Data Gagal Dimasukan!\n");
		strcpy(OUTCLIENT[0], "Data Gagal Dimasukan!\n");
		OCLENGTH = 1;
        return;
    }

	fputs(str, fptr);
	fclose(fptr);
	if(tipe_call == 1) { //insert
		// printf("Data Berhasil Dimasukan!\n");
		strcpy(OUTCLIENT[0], "Data Berhasil Dimasukan!\n");
		OCLENGTH = 1;
	}
}

int cek_tipe(char *inpt){
	for(int i=0; i<strlen(inpt); i++) {
		if(inpt[i]<'0' || inpt[i]>'9'){
			return 2;
		}
	}
	return 1;
}

void delete_tbl(int argc, char **argv, int line){
	if(line == -2){
		// printf("Tabel Tidak Ditemukan!\n");
		strcpy(OUTCLIENT[0], "Tabel Tidak Ditemukan!\n");
		OCLENGTH = 1;
		return;
	}
	else if(line == -3){
		// printf("Tidak Ditemukan Nama Kolom!\n");
		strcpy(OUTCLIENT[0], "Tidak Ditemukan Nama Kolom!\n");
		OCLENGTH = 1;
		return;
	}else if(line == -4){
		// printf("Tidak Ditemukan Data!\n");
		strcpy(OUTCLIENT[0], "Tidak Ditemukan Data!\n");
		OCLENGTH = 1;
		return;
	}

	int from_int = find_position(argc, argv, "FROM");
	int tbl_int = from_int + 1;
	int cek_b = ftxt_toArr(fpath_tbl(argv[tbl_int]));

	if(line == -1){ //2 karena delete
		addStringtoTxt(strcat(TBLARR[0], "\n"), fpath_tbl(argv[tbl_int]), "w", 2);
		addStringtoTxt(strcat(TBLARR[1], "\n"), fpath_tbl(argv[tbl_int]), "a+", 2);
		// printf("Data Berhasil Dihapus!\n");
		strcpy(OUTCLIENT[0], "Data Berhasil Dihapus!\n");
		OCLENGTH = 1;
	}else{
		for(int i=0; i<cek_b; i++){
			if(i != line){ //hapus line
				if(i == 0){
					addStringtoTxt(strcat(TBLARR[i], "\n"), fpath_tbl(argv[tbl_int]), "w", 2);
				}else{
					addStringtoTxt(strcat(TBLARR[i], "\n"), fpath_tbl(argv[tbl_int]), "a+", 2);
				}
			}
		}
		// printf("Data Berhasil Dihapus!\n");
		strcpy(OUTCLIENT[0], "Data Berhasil Dihapus!\n");
		OCLENGTH = 1;
	}	
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
		// printf("Tabel Tidak Ditemukan!\n");
		strcpy(OUTCLIENT[0], "Tabel Tidak Ditemukan!\n");
		OCLENGTH = 1;
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
			// printf("Panjang Argumen dengan Kolom Tabel Tidak Sama!\n");
			strcpy(OUTCLIENT[0], "Panjang Argumen dengan Kolom Tabel Tidak Sama!\n");
			OCLENGTH = 1;
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
				// printf("Gagal! Tipe data ke-%d: %s!\n", i+1, *(tokens + i));
				char outc[100];
				sprintf(outc, "Gagal! Tipe data ke-%d: %s!\n", i+1, *(tokens + i));

				strcpy(OUTCLIENT[0], outc);
				OCLENGTH = 1;
				return;
			}
		}
		strcat(saveStr, "\n");
		// printf("save: %s\n", saveStr);
		addStringtoTxt(saveStr, fpath_tbl(argv[tbl_int]), "a+", 1);
	}
}

int findMax_strTbl(char *namaTbl){
	int out = 0;
	int cek = ftxt_toArr(fpath_tbl(namaTbl));
	if(cek == -1){
		return -1;
	}
	for(int i=0; i<cek; i++){
		char** tokens;
		tokens = str_split(TBLARR[i], ',');
		if(tokens){
			for(int j=0; *(tokens + j); j++){
				if(strlen(*(tokens + j)) > out){
					out = strlen(*(tokens + j));
				}
			}
		}
	}
	return out;
}

void select_tbl(int argc, char **argv){
	int from_int = find_position(argc, argv, "FROM");
	int tbl_int = from_int + 1;
	int *kol_int = malloc(100);

	if(from_int == 1){
		// printf("Format Perintah Salah!\n");
		// printf("SELECT [namakolom1, namakolom2...] FROM [namatabel]\n");
		strcpy(OUTCLIENT[0], "Format Perintah Salah!\n");
		strcpy(OUTCLIENT[1], "SELECT [namakolom1, namakolom2...] FROM [namatabel]\n");
		OCLENGTH = 2;
		return;
	}

	int lenTab = findMax_strTbl(argv[tbl_int]) + 1;
	// int lenTab = 10;

	int cek = ftxt_toArr(fpath_tbl(argv[tbl_int]));
	if(cek == -1) {
		// printf("Tabel Tidak Ditemukan!\n");
		strcpy(OUTCLIENT[0], "Tabel Tidak Ditemukan!\n");
		OCLENGTH = 1;
		return;
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
			if(kol_c == 0) {
				strcpy(OUTCLIENT[0], "Kolom Tidak DItemukan!\n");
				OCLENGTH = 1;
				return;
			}

			char outc[100];
			//print nama kolom
			for(int x=0; x<kol_c; x++){
				// printf("%s", *(tokens + kol_int[x]));
				char ctab[50];
				sprintf(ctab, "%*s", -lenTab, *(tokens + kol_int[x]));
				if(x==0){
					strcpy(outc, ctab);
				}else{
					strcat(outc, ctab);
				}
				if(x<kol_c - 1){
					// printf("\t");
					strcat(outc, " ");
				}

			}
			// printf("\n");
			strcat(outc, "\n");
			strcpy(OUTCLIENT[0], outc);
			OCLENGTH = 1;
		}
		// for(int x=0; x<kol_c; x++){
		// 	printf("urutan jal: %d, str: %s, urutan didb: %d\n", 
		// 	x, *(tokens + kol_int[x]), kol_int[x]);
		// }

		//print isi database, panjang sepanjang cek (dari ftxt_arr)
		for(int x=2; x<cek; x++) {
			char** isiDb;
			isiDb = str_split(TBLARR[x], ',');

			char outc[100];
			if(isiDb){
				for(int i=0; i<kol_c; i++){ //print urutan sesuai kol_c dari argv(command)
					// printf("%s", *(isiDb + kol_int[i]));
					char ctab[50];
					sprintf(ctab, "%*s", -lenTab, *(isiDb + kol_int[i]));

					if(i==0){
						strcpy(outc, ctab);
					}else{
						strcat(outc, ctab);
					}
					if(i<kol_c - 1){
						// printf("\t");
						strcat(outc, " ");
					}
				}
				// printf("\n");
				strcat(outc, "\n");
				strcpy(OUTCLIENT[x-1], outc);
				OCLENGTH += 1;			
			}
		}

		// for (int i=2; i<cek; i++) {
		// 	printf("%s\n", TBLARR[i]);
		// }
	}
}

void run_command(char *comm){

	char schar[3] = {',', '.', ';'};
	int i, whereC = -1;

	char *param = malloc(100);
	strcpy(param, comm);

    char** tokens;
    tokens = str_split(comm, ' ');

    if (tokens) {
        for (i = 0; *(tokens + i); i++) {
            // free(*(tokens + i));
			//buang spesialchar(schar)
			strcpy(*(tokens+i), remove_schar(*(tokens + i), schar));
			if(strcmp(*(tokens+i), "WHERE") == 0) {
				whereC = i;
			}
        }
        // free(tokens);
    }

	// i sebagai panjang dari command yang telah ditokens

	if (strcmp(*(tokens + 0), "INSERT") == 0){
		insert_tbl(i, tokens, substr(param, find_index(param, '('), find_index(param, ')') + 1));
	}
	else if(strcmp(*(tokens + 0), "UPDATE") == 0){
		int kolom = -1;
		int baris = fline_where(i, tokens, whereC, &kolom);
		printf("kolom: %d\n", kolom);

	}
	else if(strcmp(*(tokens + 0), "DELETE") == 0){
		int kolom;
		delete_tbl(i, tokens, fline_where(i, tokens, whereC, &kolom));
	}
	else if(strcmp(*(tokens + 0), "SELECT") == 0){
		select_tbl(i, tokens);
	}
}