#test values:
tests = [1,2,3,4,5,6,7,8,9,10]
testadds= [1,2,3,4,5,6,7,8,9,0]

arr_setup = 'uint32_t *'
arr_val = ' = {'
outArrs = []
for t,a in zip(tests, testadds):
    tb = bin(t)[2:]
    ab = bin(a)[2:]
    txto = arr_setup + "a" + arr_val
    txtb = arr_setup + "b" + arr_val
    for ac,bc in zip(tb,ab):
        txto = f"{txto}{ac},"
        txtb = f"{txtb}{bc},"

    txto = txto + "};"
    txtb = txtb + "};"

    outArrs.append(txto)
    outArrs.append(txtb)

for i in outArrs:
    print(i)
