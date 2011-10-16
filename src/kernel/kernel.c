#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"

#include "video.h"
#include "kernel.h"
#include "scheduler.h"
#include "fd.h"


///////////// Inicio de Variables del Kernel

/* IDT de 80h entradas*/
DESCR_INT idt[0x81]; 
/* IDTR */
IDTR idtr; 



void clear_kernel_buffer() {
	int i = 0;
	for(i = 0; i < KERNEL_BUFFER_SIZE; ++i)	{
		kernel_buffer[i] = 0;
	}
}

///////////// Fin de Variables del Kernel

///////////// Inicio de funciones auxiliares del Kernel.

/*
 *	setup_IDT_entry
 * 		Inicializa un descriptor de la IDT
 *
 *	Recibe: Puntero a elemento de la IDT
 *	 Selector a cargar en el descriptor de interrupcion
 *	 Puntero a rutina de atencion de interrupcion
 *	 Derechos de acceso del segmento
 *	 Cero
 */
void setup_IDT_entry(DESCR_INT *item, byte selector, dword offset, byte access, byte cero) {
	item->selector = selector;
	item->offset_l = offset & 0xFFFF;
	item->offset_h = offset >> 16;
	item->access = access;
	item->cero = cero;
}

///////////// Fin de funciones auxiliares del kernel.

///////////// Inicio Handlers de interrupciones.

int krn = 0;

void int_09() {
	krn = 1;
	char scancode;
	scancode = _in(0x60);

	// We check if the scancode is a char or a control key.
	int flag = scancode >= 0x02 && scancode <= 0x0d;
	flag = flag || (scancode >= 0x10 && scancode <= 0x1b);
	flag = flag || (scancode >= 0x1E && scancode <= 0x29);
	flag = flag || (scancode >= 0x2b && scancode <= 0x35);
	if (flag)	{
		char sc = scanCodeToChar(scancode);
		if(sc != 0 && sc != EOF)	{
			pushC(sc); //guarda un char en el stack
		}
	}
	else {
		controlKey(scancode); // Envia el scancode al analizador de control keys.
	}
	krn = 0;
	
}

int in_kernel(){
	return krn;
}

void int_80() {
	krn = 1;
	int systemCall = kernel_buffer[0];
	int fd         = kernel_buffer[1];
	int buffer     = kernel_buffer[2];
	int count      = kernel_buffer[3];

	
	int i, j;

	if (systemCall == WRITE) {
		Process * current = getp();
		kernel_buffer[KERNEL_RETURN] = fd_write(current->file_descriptors[fd],(char *)buffer,count);
	} else if (systemCall == READ) {
		Process * current = getp();
		kernel_buffer[KERNEL_RETURN] = fd_read(current->file_descriptors[fd],(char *)buffer,count);
	} else if (systemCall == MKFIFO) {		
		int _fd = process_getfreefd();
		if(_fd != -1)	{
			int fd = fd_open(_FD_FIFO, (void *)kernel_buffer[1],kernel_buffer[2]);
			getp()->file_descriptors[_fd] = fd;
			kernel_buffer[KERNEL_RETURN] = _fd;
		}
		else {
			kernel_buffer[KERNEL_RETURN] = -1;
		}
	} else if (systemCall == CLOSE) {
		kernel_buffer[KERNEL_RETURN] = fd_close(getp()->file_descriptors[fd]);
	}
	else if (systemCall == PCREATE) {
		kernel_buffer[KERNEL_RETURN] = sched_pcreate(kernel_buffer[1],kernel_buffer[2],kernel_buffer[3]);
	}
	else if (systemCall == PRUN) {
		kernel_buffer[KERNEL_RETURN] = sched_prun(kernel_buffer[1]);
	}
	else if (systemCall == PDUP2) {
		kernel_buffer[KERNEL_RETURN] = sched_pdup2(kernel_buffer[1],kernel_buffer[2],kernel_buffer[3]);
	}
	else if (systemCall == GETPID) {
		kernel_buffer[KERNEL_RETURN] = sched_getpid();
	}
	else if (systemCall == WAITPID) {
		kernel_buffer[KERNEL_RETURN] = sched_waitpid(kernel_buffer[1]);
	} else if (systemCall == PTICKS) {
		kernel_buffer[KERNEL_RETURN] = (int) storage_index();
	} else if (systemCall == PNAME) {
		Process * p = process_getbypid(kernel_buffer[1]);
		if(p == NULL)
		{
			kernel_buffer[KERNEL_RETURN] = (int) NULL;
		} else {
			kernel_buffer[KERNEL_RETURN] = (int) p->name;
		}
	} else if (systemCall == PSTATUS) {
		Process * p = process_getbypid(kernel_buffer[1]);
		if(p == NULL)
		{
			kernel_buffer[KERNEL_RETURN] = (int) -1;
		} else {
			kernel_buffer[KERNEL_RETURN] = (int) p->state;
		}
	}	else if (systemCall == PPRIORITY) {
		Process * p = process_getbypid(kernel_buffer[1]);
		if(p == NULL)
		{
			kernel_buffer[KERNEL_RETURN] = (int) -1;
		} else {
			kernel_buffer[KERNEL_RETURN] = (int) p->priority;
		}
	}	else if (systemCall == PGID) {
		Process * p = process_getbypid(kernel_buffer[1]);
		if(p == NULL)
		{
			kernel_buffer[KERNEL_RETURN] = (int) -1;
		} else {
			kernel_buffer[KERNEL_RETURN] = (int) p->gid;
		}
	}	else if (systemCall == PGETPID_AT) {
		Process * p = process_getbypindex(kernel_buffer[1]);
		if (p->state != -1) {
			kernel_buffer[KERNEL_RETURN] = (int) p->pid;
		} else {
			kernel_buffer[KERNEL_RETURN] = -1;
		}
	}	else if (systemCall == KILL) {
		kernel_buffer[KERNEL_RETURN - 1] = kernel_buffer[1];
		kernel_buffer[KERNEL_RETURN - 2] = kernel_buffer[2];
	}
	
	krn = 0;
}


// Fires a signal after a syscall, only if the kernel has been set to do so.
void signal_on_demand() {
	make_atomic();
	if (kernel_buffer[KERNEL_RETURN - 1] != 0) {
		
		int sigcode = kernel_buffer[KERNEL_RETURN - 1];
		int pid = kernel_buffer[KERNEL_RETURN - 2];

		kernel_buffer[KERNEL_RETURN - 1] = 0; // SIGCODE
		kernel_buffer[KERNEL_RETURN - 2] = 0; // PID
		
		sg_handle(sigcode, pid);
	}
	release_atomic();
}

///////////// Fin Handlers de interrupciones.

Process * p1, * idle, * kernel;

int idle_main(int argc, char ** params) {
	while(1) {
		_Halt();
	}
}

///////////// Inicio KMAIN

/**********************************************
 kmain()
 Punto de entrada de código C.
 *************************************************/
kmain() {
	int i, num;


	/* CARGA DE IDT CON LA RUTINA DE ATENCION DE IRQ0    */

	setup_IDT_entry(&idt[0x08], 0x08, (dword) & _int_08_hand, ACS_INT, 0);

	/* CARGA DE IDT CON LA RUTINA DE ATENCION DE IRQ1    */

	setup_IDT_entry(&idt[0x09], 0x08, (dword) & _int_09_hand, ACS_INT, 0);

	/* CARGA DE IDT CON LA RUTINA DE ATENCION DE int80h    */

	setup_IDT_entry(&idt[0x80], 0x08, (dword) & _int_80_hand, ACS_INT, 0);
	
	/* CARGA DE IDT CON LA RUTINA DE ATENCION DE int79h    */

	setup_IDT_entry(&idt[0x79], 0x08, (dword) & _int_79_hand, ACS_INT, 0);

	/* Carga de IDTR */

	idtr.base = 0;
	idtr.base += (dword) & idt;
	idtr.limit = sizeof(idt) - 1;

	_lidt(&idtr);

	scheduler_init();
	_Cli();
	
	/* Habilito interrupcion de timer tick*/
	_mascaraPIC1(0xFC);
	_mascaraPIC2(0xFF);
	_Sti();

	idle = create_process("idle", idle_main, 0, 0, 0, 0, 0, 0, 0, NULL, 0);
	tty_init(0);
	tty_init(1);
	tty_init(2);
	tty_init(3);
	tty_init(4);
	tty_init(5);

	// We soon exit out of here :)
	while (1);

}

///////////// Fin KMAIN