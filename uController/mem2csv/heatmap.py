filename = "memory.txt"

f=file(filename).read()
words=f.split()
r=open('mem.csv','w')

row=2
col=256
for i in range(row):
    for j in range(col):
        r.write(str(words[i*col+j+1]))
        r.write(",")
    r.write("\n")
    