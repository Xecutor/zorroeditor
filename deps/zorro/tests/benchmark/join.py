def f():
  x=0
  for i in range(100000):
    a=[]
    for j in range(40):
      a.append("hello")
    x+=len(''.join(a))
  print(x)

f()
