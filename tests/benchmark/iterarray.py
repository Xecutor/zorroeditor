def f():
  x=[None]*1000000
  for i in range(0,1000000):
    x[i]=i
  y=0.0
  for j in range(0,9):
#    y+=reduce(lambda x,y:x+y,x)
    for i in x:
      y+=i
  print(y)
f()
