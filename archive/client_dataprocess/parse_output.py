hail = []
us = []

beta_diffs = []
se_diffs = []

import time
with open('output10.txt', 'r') as f:
    for line in f:
        hail.append([s for s in line.split(' ') if len(s) and s != '\n'])

with open('result.out', 'r') as f:
    for line in f:
        us.append([s for s in line.split('\t') if len(s) and s != '\n'])

for hval in hail:
    for usval in us:
        if hval[0] == usval[0] and hval[1] == usval[1]:
            # if hval[6] != usval[5]:
                # print(hval[6], usval[5])
                # print(hval, usval)
            # try:
            if 'NA' in usval[2] or 'NA' in hval[2]:
                # beta_diffs.append(0)
                # se_diffs.append(0)
                continue
            if abs(float(hval[5]) - float(usval[2])) > .01:
                # print(abs(float(hval[2]) - float(usval[2])))
                # print(hval, usval)
                print(hval[5], usval[2])
                exit(0)
            # if abs(float(hval[3]) - float(usval[3])) > .01:
            #     # print(abs(float(hval[3]) - float(usval[3])))
            #     # print(hval, usval)
            #     print(hval[0])

            # beta_diffs.append(abs(float(hval[2]) - float(usval[2])))
            # se_diffs.append(abs(float(hval[3]) - float(usval[3])))
            beta_diffs.append(abs(float(hval[5]) - float(usval[2])))
            se_diffs.append(abs(float(hval[6]) - float(usval[3])))
            continue
            # except:
                # print(hval, usval)
print('\n\n\n\n\n\n')
print(max(beta_diffs), max(se_diffs))
print(sum(beta_diffs) / len(beta_diffs), sum(se_diffs) / len(se_diffs))
# #print(hail[beta_diffs.index(max(beta_diffs))], '\n', us[beta_diffs.index(max(beta_diffs))], '\n', hail[se_diffs.index(max(se_diffs))], '\n', us[beta_diffs.index(max(beta_diffs))])


