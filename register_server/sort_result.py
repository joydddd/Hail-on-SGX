from functools import cmp_to_key
lines = []
with open('results.out', 'r') as f:
    for line in f:
        lines.append(line)


def compare(l1, l2):
    a = l1.split('\t')[0].split(':')
    b = l2.split('\t')[0].split(':')
    if a[0] == b[0]:
        if len(b[1]) != len(a[1]):
            if len(b[1]) > len(a[1]):
                return -1
            else:
                return 1

        if a[1] > b[1]:
            return 1
        return -1
    
    if a[0] == 'X' and b[0] != 'X':
        return 1
    if a[0] != 'x' and b[0] == 'X':
        return -1

    if len(b[0]) != len(a[0]):
        if len(b[0]) > len(a[0]):
            return -1
        else:
            return 1

    if a[0] > b[0]:
        return 1
    return -1 

lines = sorted(lines, key=cmp_to_key(compare))

with open('results.out', 'w') as f:
    for line in lines:
        f.write(line)