import itertools
from functools import reduce
import random
import numpy as np
from joblib import Parallel, delayed

def rl(n,x=4):
    return np.random.randint(x, size=n)
    return random.sample(list(range(x)) * (2*n), n)
    return [random.randint(0,4) for i in range(0,10000)]

def newTN(cd):
    coret = cd[0]
    idt = cd[1]
    ttp = "TN"
    core_id = coret
    local_id = idt
    connectivity = []
    synapse_type = []
    sign_g = [1,1,-1,-1]
    sign_l = -1
    sign_vr = 1
    alpha = 1
    beta = 2
    synaptic_weights = []
    leak = 0
    deterministic = 1
    monotonic = 1
    TM = 1010010
    VR = 0
    connectivity = [1] * 256
    synapse_type = rl(256) #[random.randint(0,4)],size=256)
    sign_bits = 1,1,-1,-1
    synaptic_weights = rl(256,5) #random.randint(0,5,size=4)
    os = lambda xx,yy: str(xx) + "," + str(yy)

    out = ""
    out += ttp + ","
    out += "{},{},".format(core_id,local_id)
    out += reduce(os,connectivity) + ","
    out += reduce(os,synapse_type) + ","
    out += reduce(os,sign_bits) + ","
    out += reduce(os,synaptic_weights) +  ","
    out += str(leak) + ","
    out += str(deterministic) + ","
    out += str(monotonic) + ","
    out += str(alpha) + ","
    out += str(beta) + ','
    out += str(TM) + '\n'

    return np.array(out)

cores = [x for x in range(0,2048)]
neurons = [x for x in range(0,256)]

def fun1(c,n):
    return "ASDF"   


ns = Parallel(n_jobs=8)(delayed (newTN)(c) for c in itertools.product(cores,neurons))
print("Neuron Defs Created (demo). creating CSV file.")
with open('demo.csv', 'w') as f:
    for i in ns:
        f.write(i.tolist())

