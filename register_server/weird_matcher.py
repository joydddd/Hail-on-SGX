res = set()
res_baseline = set()
with open('results.out', 'r') as f:
    for line in f:
        res.add(" ".join(line.split('\t')[2:]))

with open('log_baseline.out', 'r') as f:
    for line in f:
        res_baseline.add(" ".join(line.split('\t')[2:]))

if len(res & res_baseline) != len(res_baseline):
    print("Doesn't Match")
else:
    print("Matches")
