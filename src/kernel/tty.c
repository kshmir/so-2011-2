#include "tty.h"
#include "../../include/kernel.h"
#include "../drivers/atadisk.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/mcglib.h"
#include "../libs/queue.h"
#include "scheduler.h"
#include "kernel.h"
#include "fd.h"

static int tty_index = 0;
extern int max_uid;
extern int current_tty;

/** Reads the command received and excecutes the corrisponding process sending it it's arguments and 
also responds to special characters.*/
int process_input(const char * input, int tty_number) { 

	int  piped = 0;
	char file_call_buffer[1024];
	char stdout_call_buffer[128];
	char stdin_call_buffer[128];
	
	int stdout_file = 0;
	int stdin_file = 0;
	int read_ptr = 0;
	int stdin = 0;
	int stdout = 0;
	do {
		int i = 0;
		int write_i = 0;
		for(; i < 128; ++i) {
			stdout_call_buffer[i] = 0;
			stdin_call_buffer[i]  = 0;
		}

		for(i = 0; i < 1024; ++i)	{
			file_call_buffer[i] = 0;
		}
		i = read_ptr;
		
		char * file_call;

		
		int valid_input = 0;
		char * current_buffer = file_call_buffer;
		for(; input[i] && input[i] != '|'; ++i) {
			if(strlen(current_buffer) > 0 ) {
				if(input[i] == '>') {
					current_buffer = stdout_call_buffer;
					valid_input = 0;
					write_i = 0;
					stdout_file = 1;

					continue;
				} else if (input[i] == '<') { 
					current_buffer = stdin_call_buffer;
					valid_input = 0;
					write_i = 0;
					stdin_file = 1;
					continue;
				}
			} else {
				if (input[i] == '>' || input[i] == '<') {
					return 0;
				}
			}
			valid_input = input[i] != ' ';
			if(!(write_i == 0 && !valid_input)) {
				current_buffer[write_i++] = input[i];
			}
		}
		if(i == 0 && input[i] == '|') {
			return 0;
		}
		piped = (input[i] == '|') && !stdin_file && !stdout_file; 
		i++;
		
		if(piped)
		{
			read_ptr = i;
		}
		
		int n = 0;
		char** strs = split_string(file_call_buffer, ' ', &n);
		
		
		int pid = pcreate(strs[0], n, strs);
		
		if(pid != -1)
		{
			if(stdout_file) {
				int _stdout = open(stdout_call_buffer, O_WR | O_NEW);
				if(_stdout >= 0)
				{
					pdup2(pid, _stdout, STDOUT);
 				} else {
					printf("TTY: file %s could not be opened for output, invalid perms?\n", stdout_call_buffer);
				}
			}
			
			if(stdin_file) {
				int _stdin = open(stdin_call_buffer, O_RD);
				if(_stdin >= 0)
				{
					pdup2(pid, _stdin, STDIN);				
				} else {
					printf("TTY: file %s could not be opened for input, invalid perms?\n", stdin_call_buffer);
				}

			}
			
			// For pipe
			if(stdin)	{
				pdup2(pid, stdin, STDIN);
			}
			char cad[20];
			if(piped)
			{

				cad[0] = '.';
				itoa(pid, cad + 1);
				stdout = mkfifo(cad, 0600);
				pdup2(pid,stdout,STDOUT);
				
				stdin = stdout;
				stdout = 0;
			}
			
			prun(pid);
			if(!string_ends_with(file_call_buffer, '&') && !piped) {
				waitpid(pid);
				rm(cad);
			}
		}
	} while(piped);
	return 1;
}
/** Starts a shell, it's a user process, but yes, resides in the kernel's code */
int tty_main (int argc, char ** argv)
{	
	clear_screen();
	char cadena[50];
	
	int status     = 0; // 0: logged out; 1: logged in
	int val        = 0;
	int tty_number = 0;
	val = 0;
	tty_number = atoi(argv[1]) + 1;
	char * input;
	char * username;
	char * password;
	printf("Monix v1 - TTY %d\n", tty_number);
	printf("Marseillan, Pereyra, Videla\n");
	printf("Sistemas Operativos - 2011 - ITBA\n");
	printf("Dennis Ritchie RIP\n");

	if(tty_number == 1)
	{
		fs_init();
		users_init();				// Init the users

		fs_finish();
	}

	int child;
	while(1) {
		switch (status){
			case 1:
				printf("user@tty%d:", tty_number);
				input = (char *) getConsoleString(1);
				process_input(input, tty_number);
				if(getuid(NULL) == -1)
				{
					status = 0;
				}
				break;
			case 0:
				child = pcreate("su", 1, NULL);
				prun(child);
				waitpid(child);
				if(getuid(NULL) != -1)	{
					status = 1;
				}
				break;
		}
	}
	return 0;
}

static char * process_name = "tty";

// Creates TTY process
int tty_init(int tty_num) { 
	char ** _params = (char **)malloc(sizeof(char *) * 2);
	char * _num = (char *)malloc(sizeof(char) * 9);
	_params[0] = process_name;
	_params[1] = _num;
	itoa(tty_num, _num);
	fd_open_with_index(FD_TTY0 + tty_num, _FD_TTY, NULL, 0666);
	create_process("tty", tty_main, 0, tty_num, 1, FD_TTY0 + tty_num, FD_TTY0 + tty_num, FD_TTY0 + tty_num, 1, _params, 0);
	init_context(tty_index);
	tty_index++;
}