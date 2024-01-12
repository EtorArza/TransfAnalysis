#
#   This software is licensed under the Academic Free License version 3.0
#
#   Please refer to the full license included in LICENSE.txt
#

ifile=ARGV[0];
ofile=ARGV[1]

ff=File.new(ofile, 'w')
aa=Array.new
bb=Array.new
ind=0;
File.open(ifile){ |lines|
  while thisline=lines.gets
     if thisline=~/^\d/
        foo=thisline.split(" ")
        aa[ind]=foo[1].to_f
        bb[ind]=foo[2].to_f
        ind=ind+1
     end
  end
}
aamax=aa.max();
aamin=aa.min();
bbmax=bb.max();
bbmin=bb.min();
for i in 0...ind
  ff.puts "#{(aa[i]-aamin)*400/(aamax-aamin)}, #{(bb[i]-bbmin)*400/(bbmax-bbmin)} "
end
ff.close()
