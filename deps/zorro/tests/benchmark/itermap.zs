m={=>}
for i in 1..1000000
  m{"key$i"}=i
end
x=0
y=0
for k,v in m
  x+=#k
  y+=v
end
print("x=$x,y=$y")
