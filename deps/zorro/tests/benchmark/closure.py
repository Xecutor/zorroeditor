def g(x,y,z):
  def q():
    return x+y+z
  return q()

def f(x,y,z):
  def c():
    return g(x,y,z)
  return c

def xrange(x):
    return iter(range(x))

def t():
  z=f(1,2,3)
  y=0
  for i in xrange(1000000):
    y+=z()
  print(y)
t()
