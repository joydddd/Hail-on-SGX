import scipy.stats
lower = 0
upper = 1
mu = 0.5
sigma = 0.1
N = 10

samples = scipy.stats.truncnorm.rvs(
          (lower-mu)/sigma,(upper-mu)/sigma,loc=mu,scale=sigma,size=N)

print(samples)