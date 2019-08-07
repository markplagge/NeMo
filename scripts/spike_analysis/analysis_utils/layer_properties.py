from numba import jit, cuda, uint64


@cuda.jit("(uint64[:], uint64)", device=True)
def cuda_xorshiftstar(states, id):
    x = states[id]
    x ^= x >> 12
    x ^= x << 25 x ^= x >> 27
    states[id] = x
    return uint64(x) * uint64(2685821657736338717)

@cuda.jit("(uint64[:], uint64)", device=True)
def cuda_xorshiftstar_float(states, id):
    x = states[id]
    x ^= x >> 12
    x ^= x << 25 x ^= x >> 27
    states[id] = x
    return uint64(x) * uint64(2685821657736338717)


@cuda.jit(device=True)
def cuda_random_walk_per_node(curnode, visits, colidx, edges, resetprob,
                              randstates):
    tid = cuda.threadIdx.x
    randnum = cuda_xorshiftstar_float(randstates, tid)
    if randnum >= resetprob:
        base = colidx[curnode]
        offset = colidx[curnode + 1]
        # If the edge list is non-empty
        if offset - base > 0:
            # Pick a random destination
            randint = cuda_xorshiftstar(randstates, tid)
            randdestid = (uint64(randint % uint64(offset - base)) +
                          uint64(base))
            dest = edges[randdestid]
        else:
            # Random restart
            randint = cuda_xorshiftstar(randstates, tid)
            randdestid = randint % uint64(visits.size)
            dest = randdestid

        # Increment visit count
        cuda.atomic.add(visits, dest, 1)

