function: 
```
gwas = hl.linear_regression_rows(y=common_mt.CaffeineConsumption, x=common_mt.GT.n_alt_alleles(), covariates=[1.0])
```

y: Caffeine.tsv
x: alleles.tsv (alleles_1.tsv: first 100 columns of alleles.tsv, alleles_2.tsv the rest 150 columns of alleles.tsv)

gwas: result.tsv