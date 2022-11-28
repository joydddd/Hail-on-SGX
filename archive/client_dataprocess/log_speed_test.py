import sys
sys.path.append('/home/jonaher/hail/')
print(sys.path)

from bokeh.layouts import gridplot
from bokeh.io import show, output_notebook
import hail as hl
import time
hl.init()


# hl.utils.get_1kg('data/')

hl.import_vcf('vcf/output.vcf').write('data_test/1kg.mt', overwrite=True)

mt = hl.read_matrix_table('data_test/1kg.mt')
table = (hl.import_table('data/1kg_annotations.txt', impute=True)
         .key_by('Sample'))
mt = mt.annotate_cols(**table[mt.s])
mt = hl.sample_qc(mt)


# mt = mt.filter_cols((mt.sample_qc.dp_stats.mean >= 4)
#                     & (mt.sample_qc.call_rate >= 0.97))
# ab = mt.AD[1] / hl.sum(mt.AD)
# filter_condition_ab = ((mt.GT.is_hom_ref() & (ab <= 0.1)) |
#                        (mt.GT.is_het() & (ab >= 0.25) & (ab <= 0.75)) |
#                        (mt.GT.is_hom_var() & (ab >= 0.9)))
# mt = mt.filter_entries(filter_condition_ab)
# mt = hl.variant_qc(mt).cache()
# common_mt = mt.filter_rows(mt.variant_qc.AF[1] > 0.01)

mt.GT.n_alt_alleles().export('test.tsv')
mt.PurpleHair.export('PurpleHair.tsv')
mt.isFemale.export('isFemale.tsv')

# mt.GT.n_alt_alleles().export('test.tsv')

start = time.time()
gwas = hl.linear_regression_rows(#test="wald",
                                   y=mt.PurpleHair, x=mt.GT.n_alt_alleles(), covariates=[1.0, mt.isFemale])
end = time.time()
print("LOG REG TIME: ", end - start)

pca_eigenvalues, pca_scores, _ = hl.hwe_normalized_pca(mt.GT)

gwas.show(n=10000, width=10000)