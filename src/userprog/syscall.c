#include "userprog/syscall.h"

static struct lock syscall_lock;
static void syscall_handler (struct intr_frame *);
static bool is_valid_arg (void *arg);
static void halt(void);
static void exit(int status);
static int write(int fd, const void  *buffer, unsigned size);
static tid_t exec(const char  *cmd_line);
static int wait(tid_t pid);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
	lock_init(&syscall_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{
	int call;
	int status;
	
	int *args = f->esp; 
	args++;
	
	//if(is_valid_arg(f->esp))
		call = *(int*)(f->esp);
	//else
	//	call = -1; /* Se ira al default del switch */
		
	/**/printf ("\nSYSCALL\n");
	switch(call)
	{
		case SYS_HALT:
		/**/printf ("SYS_HALT\n");
			halt();
			break;

		case SYS_EXIT:
		/**/printf ("SYS_EXIT\n");
			/* Revisar puntero */
			if(is_valid_arg(f->esp+4))
				status = *(int *)(f->esp + 4); /* Valido */
			else
				status = -1; /* Error */
			
			exit(status);
			break;

		case SYS_EXEC:
		/**/printf ("SYS_EXEC\n");
			/* Revisar puntero */
			if(is_valid_arg(f->esp+4))
			{
				status = exec((*(char **)(f->esp + 4)));; /* Valido */
				f->eax = status;
			}
			else
			{
				status = -1;
				exit(status);
			}
			/*Hay que hacer un llamado a Exec con el cmd se deberia encontrar en el esp, 
			 no estoy seguro si es en el lugar que se encuentra el esp o el primero, asi como f->esp*/
			break;
		
		case SYS_WAIT:
		/**/printf ("SYS_WAIT\n");
			if(is_valid_arg(f->esp+4))
			{
				status = wait(*(tid_t *)(f->esp + 4)); /* Valido */
				/* Retorno codigo del wait */
				f->eax = status;
			}
			else
			{
				status = -1; /* Error */
				exit(status);
			}
			break;
			
		case SYS_WRITE:		
			/**/printf ("SYS_WRITE\n");
			/* Verificar punteros */
			//for(i=4;i<0x0c;i+=4)
			//{
			//	if(!is_valid_arg(f->esp+i))
			//	{
			//		exit(-1);
			//		return;
			//	}
			//}
			
			/* Estando comprobados dereferencio los punteros */
			//for(i=0;i<16;i++)
				//printf("arg(%d) = %u\n",i,*(arg+i));
			f->eax = write (args[0], (char *)args[1], args[2]);			
			//f->eax = write(f_des, buffer,size); /* Retorna la cantidad de bytes escritos */
			break;
		
		default:
			printf ("INVALID SYSCALL\n");
			exit (-1);

	}
}

/*SYS_HALT:*/
static void 
halt(void)
{
  power_off();
}

/* SYS_EXIT */
static void 
exit(int status)
{
	/* TODO: Liberar los recursos extras del thread */
	struct thread *t;
	t = thread_current();
	//t->ret_status = status;			
	// Se imprime mensaje de salida
	printf("%s: exit(%d)\n",t->name, status);
	thread_exit ();
	NOT_REACHED ();
}

/* SYS_WRITE */
static int 
write(int fd, const void  *buffer, unsigned size)
{
	if(fd == STDOUT_FILENO) /* Solo se implemente STDOUT_FILENO */
			{	
				/**/printf("WRITING IN STDOUT\n");
			/* Se escribe en la consola sicronnizando con un mutex */	
			lock_acquire(&syscall_lock);
				putbuf(buffer,size);
			lock_release(&syscall_lock);
			}
			else /* De otra forma se retorna 0 */
			{
				size = 0;
			}
			return size;
}

/* SYS_EXEC */
static tid_t 
exec(const char  *cmd_line)
{
	/* Uso el proccess_execute de process.c y lo confirmo*/
	/* Agrego sincronizacion con mutex */
	lock_acquire(&syscall_lock);
		tid_t tid = process_execute (cmd_line);
	lock_release(&syscall_lock);
	if (tid == TID_ERROR) 
		return -1;
	else return tid;
}

/* SYS_WAIT */
static int
wait(tid_t pid)
{
	/* Se llama a process_wait */
	return process_wait(pid);
}

/*------------------------------------------------------------------------------
 * IS_VALID_ARG: Revisa que un puntero entregado como parámetro no sea un
 * puntero nulo, un puntero a memoria virtual no mapeada o un puntero a
 * espacio de direcciones del kernel.
 *----------------------------------------------------------------------------*/ 
static bool 
is_valid_arg(void *arg)
{
	lock_acquire(&syscall_lock);	
	bool value = true;
	if(	arg == NULL																								/* Nulo */
		||!is_user_vaddr(arg)																				/* Kernel */
	 	||pagedir_get_page(thread_current()->pagedir, arg) == NULL) /* No mapped */
		value = false;
	lock_release(&syscall_lock);
	return value;
}
