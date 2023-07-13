import re

in_ocall = False
curr_func = ''

with open('output.txt', 'r') as r:
    with open('selective_output_nosgx.txt', 'w') as w:
        for line in r:
            if '>:' in line:
                curr_func = line
                in_ocall = '<exp>:' in line
                # if in_ocall and 'write' in line:
                #     w.write(line + '\n')
                
            
            if in_ocall:
                # if 'syscall' in line:
                w.write(line + '\n')


# import re

# in_func = False
# curr_func = ''

# with open('output.txt', 'r') as r:
#     with open('selective_output.txt', 'w') as w:
#         for line in r:
#             if '>:' in line:
#                 curr_func = line
#                 in_func = '00000000000670e0 <_ZN7Log_row15update_estimateEv>:' in line
#                 # if in_ocall and 'write' in line:
#                 #     w.write(line + '\n')
                
            
#             if in_func:
#                 w.write(line + '\n')