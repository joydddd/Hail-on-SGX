function: 
```
gwas = hl.logistic_regression_rows(test="wald",
                                   y=common_mt.PurpleHair, x=common_mt.GT.n_alt_alleles(), covariates=[1.0, common_mt.isFemale])
```

y: PurpleHair.tsv
x: alleles.tsv
covariant: isFemale.tsv

gwas original result (from hail): logistic.tsv