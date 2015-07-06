require("open3")
clockRate = 1000
np = [16,32]
neurons = 256
cores = 32
endts=500
lt=0.01
rm=0
extramem=8000
rv=1
sync=3


results = Array.new
benchmarkLoc = ARGV[0]

if(ARGV[2])
  endts = ARGV[1]
end
if(ARGV[1])
	cores=ARGV[1]
end
puts ""
ctr = 10
while ctr != 0 do
  for nps in np
    cmd = "mpirun --np=#{nps} #{benchmarkLoc} --sync=3 --neurons=#{neurons} --cores=#{cores} --end=#{endts} --lt=#{lt} --rm=#{rm} --extramem=#{extramem} --rv=#{rv}"
    puts cmd
    stdout, stderr, status = Open3.capture3(cmd)
    results.push(stderr)
  end
  ctr -= 1

end

for result in results
  puts result
end

