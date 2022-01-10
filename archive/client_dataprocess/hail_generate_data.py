from bokeh.layouts import gridplot
from bokeh.io import show, output_notebook
import hail as hl
hl.init()
output_notebook()

# Second chunk
mt = hl.experimental.load_dataset(name='1000_Genomes_HighCov_autosomes',
                                  version='NYGC_30x_phased',
                                  reference_genome='GRCh38',
                                  region='us',
                                  cloud='aws')

mt.describe()

# Third chunk
# mt = mt.filter_cols((mt.sample_qc.dp_stats.mean >= 4)
#                     & (mt.sample_qc.call_rate >= 0.97))
# ab = mt.AD[1] / hl.sum(mt.AD)
# filter_condition_ab = ((mt.GT.is_hom_ref() & (ab <= 0.1)) |
#                        (mt.GT.is_het() & (ab >= 0.25) & (ab <= 0.75)) |
#                        (mt.GT.is_hom_var() & (ab >= 0.9)))
# mt = mt.filter_entries(filter_condition_ab)
# mt = hl.variant_qc(mt).cache()
# common_mt = mt.filter_rows(mt.variant_qc.AF[1] > 0.01)
# gwas = hl.logistic_regression_rows(test="wald",
#                                    y=common_mt.PurpleHair, x=common_mt.GT.n_alt_alleles(), covariates=[1.0, common_mt.isFemale])
# pca_eigenvalues, pca_scores, _ = hl.hwe_normalized_pca(common_mt.GT)

# # Fourth chunk
# p = hl.plot.qq(gwas.p_value, collect_all=True)
# p2 = hl.plot.qq(gwas.p_value, n_divisions=75)

# show(gridplot([p, p2], ncols=2, plot_width=400, plot_height=400))