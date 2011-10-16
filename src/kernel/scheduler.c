#include "scheduler.h"
#include "tty.h"
#include "fd.h"
#include "kernel.h"
#include "signals.h"
#include "../software/user_programs.c"
#include "../libs/string.h"
#include "../libs/queue.h"
#include "../monix/monix.h"

int _first_time          = 0;
int thinked              = 0;
int _processes_available = PROCESS_MAX;

unsigned int current_pid = 0;

static int	think_storage_index = 0;
static int	think_storage[PROCESS_HISTORY_SIZE];

static Process				* idle;
static Process 				process_pool[PROCESS_MAX];
static Process				* current_process = NULL;
static Queue 				* ready_queue;
static Queue 				* yield_queue;
static Queue 				* blocked_queue;

///////////// Inicio Funciones Scheduler

int rd_queue_size() {
	return queue_count(ready_queue);
}

void process_setready(Process * p) { 
	queue_enqueue(ready_queue,p);
}

int out = 0;

void scheduler_init() {
	int i = 0;
	char *pool = (char*)	process_pool;
	for(; i < PROCESS_MAX * sizeof(Process); i++) {
		*pool = (char) 0;
	}
	
	i = 0;
	for(; i < PROCESS_MAX; ++i)	{
		process_pool[i].state = -1;
	}
	ready_queue   = queue_init(PROCESS_MAX);
	blocked_queue = queue_init(PROCESS_MAX);
	yield_queue   = queue_init(PROCESS_MAX);
	
	i = 0;
	for(; i < PROCESS_HISTORY_SIZE; ++i) {
		think_storage[i] = -1;
	}
}

unsigned int _pid_seed = 0;

unsigned int process_getnextpid() {
	return _pid_seed++;
}


Process * process_getbypindex(int index) {
	return &process_pool[index];
}

Process * process_getbypid(int pid) {
	int i = 0;
	for(; i < PROCESS_MAX; ++i)	{
		if(process_pool[i].pid == pid)	{
			return &process_pool[i];
		}
	}
	return NULL;
}


Process * process_getfree() {
	if(_processes_available == 0) {
		return NULL; // Big problem.
	}
	int i = _pid_seed;
	if(i >= PROCESS_MAX - 1)	{
		i = 0;
	}
	for(; i < PROCESS_MAX; ++i) {
		if(process_pool[i].state == PROCESS_ZOMBIE || process_pool[i].state == -1) {
			_processes_available--;
			return process_pool + i;
		}
		if(i >= PROCESS_MAX - 1)	{
			i = 0;
		}
	}

	return NULL;
}

int process_getfreefd() {
	if(current_process->open_fds == PROCESS_FD_SIZE) {
		return -1; // Big problem.
	}
	int i = 0;
	for(; i < PROCESS_FD_SIZE; ++i) {
		if(current_process->file_descriptors[i] == -1) {
			return i;
		}
		if(i == PROCESS_FD_SIZE - 1)	{
			i = 0;
		}
	}

	return -1;
}

int current_p_tty() {
	return current_process->tty;
}

Process * getp() {
	return current_process;
}

int ran = 0;

void process_cleanpid(int pid) {

	Process * _curr = process_getbypid(pid);

	while(!queue_isempty(_curr->wait_queue)) {
		Process * p = queue_dequeue(_curr->wait_queue);
		p->state = PROCESS_READY;
		queue_enqueue(ready_queue,p);
	}

	int i = 0;
	for(; i < PROCESS_FD_SIZE; ++i)	{
		fd_close(_curr->file_descriptors[i]);
	}

	_curr->state = PROCESS_ZOMBIE;
	_processes_available++;
	softyield();
}

void process_cleaner() {

	while(!queue_isempty(current_process->wait_queue)) {
		Process * p = queue_dequeue(current_process->wait_queue);
		p->state = PROCESS_READY;
		queue_enqueue(ready_queue,p);
	}

	int i = 0;
	for(; i < PROCESS_FD_SIZE; ++i)	{
		close(i);
	}

	current_process->state = PROCESS_ZOMBIE;
	_processes_available++;
	softyield();
}


int yielded = 0;
int yield_save_cntx = 0;
int soft_yielded = 0;

/*
	Switches cards, allows the process to wait for something to come soon. It does NOT save CPU.
	THe process must be saved somewhere else!!!
*/
void softyield() {
//	queue_enqueue(ready_queue, current_process);
	soft_yielded = 1;
	_yield();
}

/*
	Allows the process to wait for the next tick, it saves CPU.
*/
void yield() {
	queue_enqueue(yield_queue, current_process);
	yield_save_cntx = 1;
	yielded++;
	_yield();
}

int sched_pdup2(int pid, int fd1, int fd2) {
	Process * p = process_getbypid(pid);
	fd_close(p->file_descriptors[fd2]);
	p->file_descriptors[fd2] = current_process->file_descriptors[fd1];
	return 0;
}

int sched_prun(int pid) {
	Process * p = process_getbypid(pid);
	queue_enqueue(ready_queue, p);
	return pid;
}

int sched_waitpid(int pid) {
	Process * p = process_getbypid(pid);
	if(p != NULL)
	{
		if (current_process->is_tty) {
			set_owner_pid(p->pid);
		}
		queue_enqueue(p->wait_queue, current_process);
		current_process->state = PROCESS_BLOCKED;
		return pid;
	} else {
		return -1;
	}
}

int sched_pcreate(char * name, int argc, void * params) {

	void * ptr = sched_ptr_from_string(name);
	if(ptr == NULL)	{
		return -1;
	}
	
	char * _name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
	int i = 0, len = strlen(name);
	for(; i < len; ++i)
	{
		_name[i] = name[i];
	}
	
	

	Process * p = create_process(_name, ptr, current_process->priority, current_process->tty, 0, 
		FD_TTY0 + current_process->tty, FD_TTY0 + current_process->tty, FD_TTY0 + current_process->tty, 
		argc, params, 1);
	return p->pid;
}

// Function names
char* _function_names[] = { "help", "test", "clear", "ssh", "hola", "reader", "writer", 
	"kill", "getc", "putc", "top", "hang", NULL };

// Functions
int ((*_functions[])(int, char**)) = { _printHelp, _test, _clear, _ssh, _hola_main, 
	reader_main, writer_main, _kill, getc_main, putc_main, top_main, _hang, NULL };

main_pointer sched_ptr_from_string(char * string) {
	int index;
	for (index = 0; _function_names[index] != NULL; ++index) {
		int n = 0;
		if (!strcmp(string, _function_names[index]) && strlen(string) >= strlen(_function_names[index])) {
			return _functions[index];
		}
	}
	return NULL;
}



int sched_getpid() {
	return current_process->pid;
}

int	stackf_build(void * stack, main_pointer _main, int argc, void * argv) {

	void * bottom 	= (void *)((int)stack + PROCESS_STACK_SIZE -1);

	StackFrame * f	= (StackFrame *)(bottom - sizeof(StackFrame));

	f->EBP     = 0;
	f->EIP     = (int)_main;
	f->CS      = 0x08;

	f->EFLAGS  = 0x202;
	f->retaddr = process_cleaner;
	f->argc    = argc;
	f->argv    = argv;


	return	(int)f;
}

Process * create_process(char * name, main_pointer _main, int priority, unsigned int tty, 
int is_tty, int stdin, int stdout, int stderr, int argc, void * params, int queue_block) {
	Process * p            = process_getfree();
	p->name                = name;
	p->pid                 = process_getnextpid();
	p->gid                 = 0;
	p->ppid                = (current_process != NULL) ? current_process->pid : 0;
	p->priority            = priority;
	p->esp                 = stackf_build(p->stack, _main, argc, params);
	p->state               = PROCESS_READY;
	p->file_descriptors[0] = fd_open_with_index(stdin,0,0,0);
	p->file_descriptors[1] = fd_open_with_index(stdout,0,0,0);
	p->file_descriptors[2] = fd_open_with_index(stderr,0,0,0);
	p->is_tty              = is_tty;
	p->wait_queue          = queue_init(PROCESS_WAIT_MAX);
	
	sg_set_defaults(p);
	

	int i;
	for(i = 3; i < PROCESS_FD_SIZE; ++i) {
		p->file_descriptors[i] = -1;
	}

	p->tty = tty;


	if(strcmp(name, "idle") == 0) {
		idle = p;
	} else if(!queue_block)	{
		queue_enqueue(ready_queue, p);
	}


	return p;
}


void scheduler_save_esp (int esp)
{

	if (current_process != NULL) {
		current_process->esp = esp;
		if(yield_save_cntx)
		{
			current_process = NULL;
			yield_save_cntx = 0;
		}
	}
}

void * scheduler_get_temp_esp (void) {
	if(idle != NULL)
	{
		return (void*)idle->esp;
	} 
	else
	{
		return NULL;
	}

}

long c = 0;

int atomic = 0;

void make_atomic() {
	atomic++;
}

void release_atomic() {
	atomic--;
}


void scheduler_think (void) {

	if(atomic) {
		return;
	}

	if(yielded == 0) {
		while(!queue_isempty(yield_queue)) {
			queue_enqueue(ready_queue, queue_dequeue(yield_queue));
		}
	} else { 
		yielded--;
	}

	if (current_process != NULL 
		&& current_process->state == PROCESS_RUNNING 
		&& current_process != idle)
	{
		current_process->state = PROCESS_READY;
		queue_enqueue(ready_queue, current_process);
		current_process = NULL;
	}

	
	if (!queue_isempty(ready_queue)) {
		current_process = queue_dequeue(ready_queue);
		if (current_process->state == PROCESS_BLOCKED) {
			current_process->state = PROCESS_READY;
			soft_yielded = 1;
		}
	
		if (current_process->state == PROCESS_ZOMBIE) {
			softyield();
		}
	}
	else {
		current_process = idle;
	}
	
	if (soft_yielded) {
		soft_yielded = 0;
	} else {
		think_storage[think_storage_index++] = current_process->pid;
		if (think_storage_index == PROCESS_HISTORY_SIZE) {
			think_storage_index = 0;
		}
	}

	switch_tty(current_process->tty);
	if (current_process != idle) {
		current_process->state = PROCESS_RUNNING;
	}
}

void * storage_index() {
	return think_storage;
}

int scheduler_load_esp()
{
	return current_process->esp;
}

///////////// Fin Funciones Scheduler
