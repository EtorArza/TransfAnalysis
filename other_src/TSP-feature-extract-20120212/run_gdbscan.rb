#
# This software is licensed under the Academic Free License version 3.0
# 
# Please refer to the full license included in LICENSE.txt
#
mypath=ARGV[0]
infile=ARGV[1]
file1=ARGV[2]
file2=ARGV[3]
file3=ARGV[4]


tmpfile="/tmp/tmp_#{rand()}"
tmpfile2="/tmp/tmp_#{rand()}"
f1=File.new(file1, 'w')
f2=File.new(file2, 'w')
f3=File.new(file3, 'w')
mycmd="sh -c '#{mypath}/gdbscan.pl -m 5 -e 40 #{infile} 1> #{tmpfile2} 2>  #{tmpfile}'"
system mycmd
File.open(tmpfile){ |lines|
  while thisline=lines.gets
    if thisline=~/^OUTLIERS\s(.*)$/
       f2.puts $1
    end 
    if thisline=~/^CLUSTERS\s(.*)$/
       f2.puts $1
    end
    if thisline=~/^CLUSTER-(\S*)\s(.*)$/
       foo=$2.split(" ")
       
   #    f3.puts "#{$1}, #{foo.join(', ')}"
       f3.puts "#{foo[0]}"
    end
  end
}


numnode=0
File.open(tmpfile2){|lines|
  while thisline=lines.gets
     good=1
      foo=thisline.split(" ")
    if thisline=~/noise/
        f1.puts "#{foo[0]}, #{foo[1]}, 0"
        good=0
    end
      if thisline=~/undef/
        f1.puts "#{foo[0]}, #{foo[1]}, -1"
        good=0
    end
    if good >0
          f1.puts "#{foo[0]}, #{foo[1]}, #{foo[2]}"
    end
    numnode=numnode+1;

  end

}

f2.puts numnode
f1.close()
f2.close()
f3.close()
File.delete(tmpfile)
File.delete(tmpfile2)
