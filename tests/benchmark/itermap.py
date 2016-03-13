def f():
  m={}
  for i in range(1,1000001):
    m["key%d"%i]=i
  x=0
  y=0
  for k,v in m.iteritems():
    x+=len(k)
    y+=v
  print("x=%d,y=%d"%(x,y))
f()
