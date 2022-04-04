count = 0
with open('alleles.tsv', 'r') as fread, open('alleles-multiplied.tsv', 'w') as fwrite:
    # Copy header line once!
    fwrite.write(fread.readline())
    for line in fread:
        ogline = line
        for end in range(10):
            line = ogline
            for index in range(len(line)):
                if line[index] == '\t':
                    line = line[:index - 1] + str(end) + line[index:]
                    break
            for _ in range(1):
                count += 1
                fwrite.write(line)

print('Total alleles in new tsv: ' + str(count))
