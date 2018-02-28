x=0
y=2
z=x or y
print(z)
for:outer i in 1..3
  print("i=",i)
  print(">>inner")
  for:inner j in 1..4
    break if j==4
    print("    j=",j," i=",i)
    if j==2 and i==1
      print("    next")
      next outer
    end
    if i==3 and j==3 and z==2
      z=1
      redo
    end
    print("<<inner")
  end
  print("outer")
end
print("hello")

for i in 1..3
  break
end
