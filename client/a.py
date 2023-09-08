# a = set()
# with open('a.txt', 'r') as f:
#     for line in f:
#         if 'Type: 3' in line:
#             a.add(int(line.split(' ')[1]))

# for i in range(510):
#     if i not in a:
#         print(i)

a = dict()
with open('a.txt', 'r') as f:
    for line in f:
        if '.' in line:
            ip = line.split(' ')[0]
            if ip not in a:
                a[ip] = 0
            a[ip] += 1

for e in a:
    if a[e] != 8:
        print(e, a[e])
