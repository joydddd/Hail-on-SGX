`make tests`: make all tests (build enclave & host/tests local data test without client)
`make runtests`: run all tests on open enclave
`make build`/`make`: build enclave & host
`make run`: run enclave & host
`make debugrun`: build with `-g` flag and run with DEBUGER

### enclave/gwas_enc.conf
before deployment, change debug to 0
for mulithreading, change NumTCS to the number of threads running