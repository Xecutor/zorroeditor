import math

class Point:
  def __init__(self,x,y):
    self.x=x
    self.y=y

  def move(self):
    self.x+=self.dx
    self.y+=self.dy
  def isNear(self,p):
    vx=self.x-p.x
    vx*=vx
    vy=self.y-p.y
    vy*=vy
    return vx+vy<5
  def calcDir(self,p):
    self.dx=p.x-self.x
    self.dy=p.y-self.y
    l=math.sqrt(self.dx*self.dx+self.dy*self.dy)
    self.dx/=l
    self.dy/=l

  def travel(self,dst):
    self.calcDir(dst)
    while not self.isNear(dst):
      self.move()

def f():
  x=0.0
  for i in range(10000):
    src=Point(0.0,0.0)
    dst=Point(100.0,100.0)
    src.travel(dst)
    x+=src.x+src.y
  print(x)
f()
