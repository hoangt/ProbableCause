filename = "memory.txt"

f=file(filename).read()
r=open('mem.csv','w')

testCount=3
sampleCount=21
for j in range(testCount):
    for i in range(sampleCount):
        r.write(str(int(f.split()[j*sampleCount*4+4*i+2],16)*256*256*256+int(f.split()[j*sampleCount*4+4*i+1],16)*256*256+int(f.split()[j*sampleCount*4+4*i+4],16)*256+int(f.split()[j*sampleCount*4+4*i+3],16)))
        r.write(",")
    r.write("\n")