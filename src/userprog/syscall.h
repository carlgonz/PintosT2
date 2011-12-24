#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdio.h>
#include <stdlib.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "threads/init.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/malloc.h"

//#include "devices/shutdown.h"

void syscall_init (void);
#endif /* userprog/syscall.h */
