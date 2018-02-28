enum test
  t1,t2
  t3,t4
end

a=[100,t1,t2,t3,t4,200]


for x in a
  switch x
    t1..t4:print("from enum")
    *:print("nope")
  end
if x in test
  print("yeah")
else
  print("huh?")
end
end

print()

for x in 0..12
  switch x
    0..10@2:s="even"
    1..11@2:s="odd"
    *:s="something else"
  end
  print(x,":",s)
end
print("after")


for s in ["hello","ciao","fuck","bye","damn","zombie"]
  print(s)
  switch s
    "hello","hi":print("it was greeting")
    "ciao",
    "bye":print("it was parting")
    "fuck","damn":print("it was swearing")
    *:print("it was something else")
  end
end

for x in 0..5
switch x
  1+1:print("1+1")
  2+2:print("2+2")
  *:print("none")
end
end

func f()
  return false
end
func g()
  return false
end

switch
  f():print("f")
  g():print("g")
  *:print("...")
end
