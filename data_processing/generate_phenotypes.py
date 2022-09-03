import random
import scipy.stats
import sys

random.seed('0x8BADF00D')

CLIENT_COUNT = 5000 if len(sys.argv) != 3 else int(sys.argv[1])

def weighted_random_by_dct(dct):
    rand_val = random.random()
    total = 0
    for k, v in dct.items():
        total += v
        if rand_val <= total:
            return k
    assert False, 'unreachable'

isFemale_odds = {'0': .5, '1': .5}
age_odds = dict()

age_ranges = [[[18,19], .0288], [[20,24], .067], [[25,34], .142], [[35,44], .16], [[45,54], .134], [[55,59], .048], [[60,64], .038], [[65,74], .065]]#, [[75,84],.044], [[85,100], .015]]

for age_range in age_ranges:
    first_age = age_range[0][0]
    second_age = age_range[0][1]
    dist_prob = age_range[1] / (second_age - first_age + 1)
    for age in range(first_age, second_age + 1):
        age_odds[age] = dist_prob

age_sum = 0
for age in age_odds:
    age_sum += age_odds[age]

for age in age_odds:
    age_odds[age] = age_odds[age] / age_sum

BMI_shape = [0, 50, 26.6, 4.527]

pancCancer_odds = {'0' : 3/4, '1': 1/4}

rand_binary_weighted_odds = {'0': .9, '1': .1}

rand_discrete_weighted_odds = {'1': .3, '2': .3, '3': .3, '4': .05, '5': .05}


if len(sys.argv) == 3:
    if int(sys.argv[2]) == 0:
        with open(f'../client/client_data/disease-{CLIENT_COUNT}.tsv', 'w') as f:
            f.write(f's\tdisease-{CLIENT_COUNT}\n')
            for i in range(CLIENT_COUNT):
                f.write(f'p\t{weighted_random_by_dct(isFemale_odds)}\n')
    for cov in range(int(sys.argv[2])):
        with open(f'../client/client_data/{cov + 1}-{CLIENT_COUNT}.tsv', 'w') as f:
            f.write(f's\t{cov+1}-{CLIENT_COUNT}\n')
            for i in range(CLIENT_COUNT):
                f.write(f'p\t{weighted_random_by_dct(isFemale_odds)}\n')
    exit(0)

# Phenotype: Sex
phenotypes = [['isFemale', isFemale_odds], ['age', age_odds], ['BMI', BMI_shape], ['pancCancer', pancCancer_odds], ['rand_binary', rand_binary_weighted_odds], ['rand_discrete', rand_discrete_weighted_odds]]

for phenotype in phenotypes:
    name = phenotype[0]
    odds = phenotype[1]
    with open(f'../client/client_data/{name}.tsv', 'w') as f:
        f.write(f's\t{name}\n')
        for i in range(CLIENT_COUNT):
            if type(odds) == dict:
                f.write(f'p\t{weighted_random_by_dct(odds)}\n')
            else:
                lower = odds[0]
                upper = odds[1]
                mu = odds[2]
                sigma = odds[3]
                f.write(f'p\t{int(scipy.stats.truncnorm.rvs((lower-mu)/sigma,(upper-mu)/sigma,loc=mu,scale=sigma,size=1)[0])}\n')