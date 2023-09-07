a = set()
with open('a.txt', 'r') as f:
    for line in f:
        if 'Type: 3' in line:
            a.add(int(line.split(' ')[1]))

for i in range(510):
    if i not in a:
        print(i)