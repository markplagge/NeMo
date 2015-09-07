require("open3")
clockRate = 1000
np = [8,16,32]
neurons = 256
cores = 4096
endts=1000
rv=1
sync=3
extramem=10


results = Array.new
benchmarkLoc = ARGV[0]

if(ARGV[2])
  endts = ARGV[1]
end
if(ARGV[1])
	cores=ARGV[1]
end
puts ""
ctr = 1 
while ctr != 0 do
  for nps in np
    cmd = "mpirun --np=#{nps} #{benchmarkLoc} --sync=3 --neurons=#{neurons} --cores=#{cores} --end=#{endts} --extramem=#{extramem}" 
    puts cmd
    stdout, stderr, status = Open3.capture3(cmd)
    results.push(stderr)
  end
  ctr -= 1

end

for result in results
  puts result
end

