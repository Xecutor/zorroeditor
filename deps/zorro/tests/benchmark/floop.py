def g(a,b):
  v=a
  if v>=b:
    v+=b
  return v

def xrange(x):
    return iter(range(x))

def f():
  y=0.0
  n=10000000
  gf=g
  for i in xrange(n):
    y=gf(y,i)
  print(y)
f()
