--
-- Created by IntelliJ IDEA.
-- User: markplagge
-- Date: 5/1/17
-- Time: 10:28 PM
--





function loadFile(filename)
    dofile(filename)
end

function genID(ncore,nlocal,ntype)
    local nid = ntype .. "_" .. ncore .."_"..nlocal
    return nid
end

function getNeuron (ncore, nlocal, ntype)
    local nttl = genID(ncore,nlocal,ntype)
    local v = neurons[nttl]
    return v

end

function getNeuronParam(ncore, nlocal, ntype, paramName)
    local rv = getNeuron(ncore, nlocal, ntype)
    print(rv[paramName])
    return rv[paramName]
end

function doesNeuronExist(ncore, nlocal, ntype)
    print("Nexist from " .. ncore .. " | " .. nlocal .. " | " .. ntype)

    if getNeuron(ncore,nlocal,ntype) == nil then
        return false
    else
        return true
    end

end

-- loadFile("/Users/mplagge/development/model_reader/model_reader/ex.nfg1")

print("Config File Loaded.")
