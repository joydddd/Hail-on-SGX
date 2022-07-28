import random

def weighted_random_by_dct(dct):
    rand_val = random.random()
    total = 0
    for k, v in dct.items():
        total += v
        if rand_val <= total:
            return k
    assert False, 'unreachable'

isFemale_odds = {'0': .5, '1': .5}

age_odds = {}

BMI_odds = {}

pancCancer_odds = {'0' : 63/64, '1': 1/64}

CLIENT_COUNT = 250

# Phenotype: Sex
phenotypes = [['isFemale', isFemale_odds]]

for phenotype in phenotypes:
    name = phenotype[0]
    odds = phenotype[1]
    with open(f'{name}.tsv', 'w') as f:
        f.write(f's\t{name}\n')
        for i in range(CLIENT_COUNT):
            f.write(f'p {weighted_random_by_dct(odds)}\n')