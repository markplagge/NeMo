"""A quick and dirty script that creates configuration 'files' for NeMo. """
import itertools
from functools import reduce
import random 
def rl(n,x=4):
    return random.sample(list(range(x)) * (2*n), n)
    return [random.randint(0,4) for i in range(0,10000)]
class TN:
    type = "TN"
    core_id = 0
    local_id = 0
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
    TM = 1001
    VR = 0

    def __init__(self):
        """Creates a connected neuron with random weights, no leak, and a threshold of 10."""
        self.connectivity = [1] * 256
        self.synapse_type = rl(256) #[random.randint(0,4)],size=256)
        self.sign_bits = 1,1,-1,-1
        self.synaptic_weights = rl(256,5) #random.randint(0,5,size=4)


    def to_csv(self):
        os = lambda xx,yy: str(xx) + "," + str(yy)
        out = ""
        out += self.type + ","
        out += "{},{},".format(self.core_id,self.local_id)
        out += reduce(os,self.connectivity) + ","
        out += reduce(os,self.synapse_type) + ","
        out += reduce(os,self.sign_bits) + ","
        out += reduce(os,self.synaptic_weights) +  ","
        out += str(self.leak) + ","
        out += str(self.deterministic) + ","
        out += str(self.monotonic) + ","
        out += str(self.alpha) + ","
        out += str(self.beta) + ','
#        out += str(self.M) + ','
        out += str(self.TM) + '\n'
#        out += str(self.Vrst) + ','
#        out += str(self.VRJ) + '\n'
#        out += "\n"

        return out



class ConfigFile:
    neuron_list = []

    ns_cores = 1024
    neurons_per_core = 256
    neuron_weight_count = 4




    def add_neuron(self, neuron):
        self.neuron_list.append(neuron )

    def to_csv(self):
        out = ""
        out += str(self.ns_cores) + "\n"
        out += str(self.neurons_per_core) + "\n"
        out += str(self.neuron_weight_count) + "\n"
        for n in self.neuron_list:
            out += n.to_csv()
        return out

    def save_csv(self, filename):
        with open(filename,'w') as f:
            f.write(self.to_csv())

if __name__ == '__main__':
    #create a file with one neuron for testing.
    cfg = ConfigFile()
    
#    for i in range(0, 4096)
    for core in range(0,4096):
        for neuron in range(0,256):
            n1 = TN()
            n1.core_id = core
            n1.local_id = neuron
            cfg.add_neuron(n1)
#   print(cfg.to_csv())
    print("Created neurons. Saving....")
    cfg.save_csv("demo.csv")



