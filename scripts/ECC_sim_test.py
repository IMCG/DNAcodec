import sys,os,subprocess,re,random
import xml.dom.minidom
from xml.dom.minidom import Node
import numpy as np
import matplotlib.pyplot as plt

def get_data(x):
    nodes = x.childNodes
    for node in nodes:
        if node.nodeType == node.TEXT_NODE:
            return node.data
        
        
doc = xml.dom.minidom.parse(sys.argv[-1])

mapping = []
 
root = doc.getElementsByTagName("Blocks")[0]
blocks = root.getElementsByTagName("Block")

for block in blocks:
    header=block.getElementsByTagName("Header")[0]
    position=header.getElementsByTagName("Position")[0]
    data=block.getElementsByTagName("Data")[0]
    
    mapping.append(get_data(data))

outfile=open("temp.enc","w")
    
for i in xrange(5):
    data=mapping[i]
    
    data=data.replace(u'A','00')
    data=data.replace(u'T','01')
    data=data.replace(u'C','10')
    data=data.replace(u'G','11')
    
    outfile.write(data+"\n")

outfile.close()

result=np.array([])
    
for i in xrange(0,200):
    if i==0:
        continue
    temp=[]
    for j in xrange(20):
        if i<10:
            e="00"+str(i)
        elif i<100:
            e="0"+str(i)
        else:
            e=str(i) 
        os.system("./transmit temp.enc temp.rec %d bsc 0.%s" % (random.randint(1, 100),e))
        command="./decode ECC.pchk temp.rec temp.dec bsc 0.%s prprp -100" % e
        p = subprocess.Popen(command, shell=True,stdout = subprocess.PIPE, stderr=subprocess.PIPE)
        p.wait()
        (stdout, stderr) = p.communicate()
        print "Error rate 0.%s, trial %d: result:%s" % ( e, j, stderr)
        m=re.match("Decoded (\d+) blocks, (\d+) valid.*",stderr)
        perc=float(m.group(2))/int(m.group(1))*100
        temp.append(perc)
    if i==1:
        result=np.array([temp])
    else:
        result=np.append(result,[temp],axis=0)
    
stds=result.std(axis=1)
y=result.mean(axis=1)
x=np.arange(0.001,0.2,0.001)

coefficients = np.polyfit(x, y, 6)
polynomial = np.poly1d(coefficients)
ys = polynomial(x)

plt.figure()
plt.plot(x,ys)
plt.errorbar(x, y, yerr=stds,fmt='o', ecolor='g')
plt.ylabel('Percentage of recovered blocks')
plt.xlabel('Percentage bit error rate')
plt.title("Codec percentage bit error tolerance")
plt.ylim( 0, 100 )
plt.savefig("results.png", dpi=300, format="png")
