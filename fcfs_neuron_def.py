import json 
import numpy as np


class lif_def:
    n_name = ""
    n_id = 0
    weights = []
    connectivity = []
    membrane_pot = 0.0
    threshold = 0.0
    leak_rate = -1
    reset_value = 0



class lif_group:
    lifs = []



## 64 bit spike encoder


c = lif_group


nids = np.arange(0, 64)
for i in range(0, 64):
    v = lif_def()
    v.weights = [1] * 64
    v.connectivity = [0] * 64
    v.connectivity[i] = 1
    v.n_id = i 
    v.threshold = 1
    v.leak_rate = 0
    v.reset_value = 0
    c.lifs.append(v)



lif_gp = [x.__dict__ for x in c.lifs]
print(json.dumps({"lif_group_a":lif_gp}))
