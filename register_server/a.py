sbs = dict()
with open('scaling-big.txt', 'r') as sb:
    for line in sb:
        if '.' in line:
            ip = line.split(' ')[0]
            if ip not in sbs:
                sbs[ip] = 0
            sbs[ip] += 1

with open('total-ips.txt', 'r') as ips:
    for line in ips:
        if '.' in line:
            ip = line.split('\n')[0]
        if ip not in sbs:
            print(ip)