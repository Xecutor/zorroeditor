x=[]
for i in 0..<1000000
  x[i]=i+1
end
y=0.0
for j in 1..10
  for i in x
    y+=i
  end
end
print(y)
