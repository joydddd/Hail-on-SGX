import hail as hl
import helpers
hl.init()

# Load data
hl.import_vcf('vcf/1kg.vcf').write('hail-data/1kg.mt', overwrite=True)

mt = hl.read_matrix_table('hail-data/1kg.mt')
table = (hl.import_table('hail-data/1kg_annotations.txt', impute=True)
         .key_by('Sample'))
mt = mt.annotate_cols(**table[mt.s])
mt = hl.sample_qc(mt)

# Filter data
mt = mt.filter_cols((mt.sample_qc.dp_stats.mean >= 4)
                    & (mt.sample_qc.call_rate >= 0.97))
ab = mt.AD[1] / hl.sum(mt.AD)
filter_condition_ab = ((mt.GT.is_hom_ref() & (ab <= 0.1)) |
                       (mt.GT.is_het() & (ab >= 0.25) & (ab <= 0.75)) |
                       (mt.GT.is_hom_var() & (ab >= 0.9)))
mt = mt.filter_entries(filter_condition_ab)
mt = hl.variant_qc(mt).cache()
common_mt = mt.filter_rows(mt.variant_qc.AF[1] > 0.01)

print('Sending data to SECRET system')
helpers.send_data_to_SECRET(common_mt)

print('Running regression analysis')
helpers.peform_logistic_regresion()

print('Exporting final result')
helpers.export()

# # Analyze data
# gwas = hl.logistic_regression_rows(test="wald",
#                                    y=common_mt.PurpleHair, x=common_mt.GT.n_alt_alleles(), covariates=[1.0, common_mt.isFemale])

# pca_eigenvalues, pca_scores, _ = hl.hwe_normalized_pca(common_mt.GT)

# gwas.export("logistic_result.vcf")