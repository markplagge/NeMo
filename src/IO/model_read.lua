--
-- Created by Mark Plagge
-- User: markplagge
-- Date: 5/1/17
-- Time: 10:28 PM
--





function loadFile(filename)
    dofile(filename)
end

function genID(ncore, nlocal, ntype)
    function gen()
        local nid = string.format(ntype .. "_%G_%G", ncore, nlocal)
        return nid
    end

    local status, result = pcall(gen)
    if status then
        return result
    end
    print("Error when converting ")
    print("type,core,local")
    print(ntype .. "," .. ncore .. "," .. nlocal)
    print(result)

    --    local nid = ntype .. "_" .. ncore .."_"..nlocal
    return nid
end

function getNeuron(ncore, nlocal, ntype)
    local nttl = genID(ncore, nlocal, ntype)
    local v = neurons[nttl]
    return v
end

function getNeuronParam(ncore, nlocal, ntype, paramName)
    local rv = getNeuron(ncore, nlocal, ntype)
    --- print(rv[paramName])
    return rv[paramName]
end

function clearNeuron(ncore, nlocal, ntype, paramName)
    local nttl = genID(ncore, nlocal, ntype)
    neurons[nttl] = nil
end

function doesNeuronExist(ncore, nlocal, ntype)

    if getNeuron(ncore, nlocal, ntype) == nil then
        return false
    else
        return true
    end
end

function findLine(nid, paramName, fn)
    local file = io.open(fn)
    local linetxt = ""
    for line in file:lines() do
        linetxt = linetxt .. line
    end
    local txt = ""
    local ss, es = string.find(linetxt, "" .. nid)

    local neuronLine = string.sub(linetxt, ss)
    local line = string.match(neuronLine, paramName .. "[%s=]+%b{}")
    return line
end

function modelErr(ncore, nlocal, ntype, paramName, errno, fn)
    local file = io.open(fn)
    local linetxt = ""
    for line in file:lines() do
        linetxt = linetxt .. line
    end
    local gg = genID(ncore, nlocal, ntype)
    local n = getNeuron(ncore, nlocal, ntype)
    local param = getNeuronParam(ncore, nlocal, ntype, paramName)
    local cstart, e = string.find(linetxt, "" .. gg)
    print("Bad neuron starts at " .. cstart)
    if param == nil then
        print("Neuron " .. gg .. " parameter " .. paramName .. " does not exist. \n")
        print("Config line is: \n" .. string.match(linetxt, gg .. ".-%b{}"))

    else

        print("Param misconfigured. Check parameter: \n" .. findLine(gg, paramName, fn))
    end
    print("Exists!") print("Exists!")
    --see if the parameter is in the file at all
end

-- loadFile("/Users/mplagge/development/model_reader/model_reader/ex.nfg1")

