function: 
(without covariant)
```
gwas = hl.linear_regression_rows(y=common_mt.CaffeineConsumption, x=common_mt.GT.n_alt_alleles(), covariates=[1.0])
```
(with covariant)
```
gwas = hl.linear_regression_rows(y=common_mt.CaffeineConsumption, x=common_mt.GT.n_alt_alleles(), covariates=[1.0, common_mt.is_Female])
```

y: Caffeine.tsv
x: alleles.tsv (alleles_1.tsv: first 100 columns of alleles.tsv, alleles_2.tsv the rest 150 columns of alleles.tsv)
covariant: isFemale.tsv

intermedia results: 
1. client to enclave: XTX, XTY
2. enclave to client: beta
3. client to enclave: SSE


gwas original result (from hail): with_convariant.tsv, without_convariant.tsv