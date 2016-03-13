func f(a,b)
  v=a
  v+=b if v>b
  return v
end
x=1.0
for i in 0..<10000000
 x=f(x,i)
end
print(x)
