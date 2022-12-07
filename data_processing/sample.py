index = 0
with open('output.txt', 'r') as f:
    for line in f:
        if int(line) != index:
            print(line)
            exit(0)
        index += 1
print('success')