#ifndef _ENCLAVE_H_
#define _ENCLAVE_H_

#ifdef NON_OE
#include "host_glue.h"
#else
#include "gwas_u.h"
#endif

void mark_eof_wrapper(const int thread_id);

int start_enclave();

#endif