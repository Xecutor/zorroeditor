func g(x,y,z)
  q=func()
    return x+y+z
  end
  return q()
end
func f(x,y,z)
  return func()
    return g(x,y,z)
  end
end

z=f(1,2,3)
x=0
for i in 1..1000000
  x+=z()
end
print(x)
