require 'class'

Point=class()

function Point:init(x,y)
  self.x=x
  self.y=y
end

function Point:isNear(p)
  local vx=self.x-p.x
  local vy=self.y-p.y
  vx=vx*vx
  vy=vy*vy
  return vx+vy<5
end

function Point:calcDir(p)
  self.dx=p.x-self.x
  self.dy=p.y-self.y
  local l=math.sqrt(self.dx*self.dx+self.dy*self.dy)
  self.dx=self.dx/l
  self.dy=self.dy/l
end

function Point:move()
  self.x=self.x+self.dx
  self.y=self.y+self.dy
end

function Point:travel(dst)
  self:calcDir(dst)
  while not self:isNear(dst) do
    self:move()
  end
end

local x=0.0
for i=1,10000 do
  local src=Point(0.0,0.0)
  local dst=Point(100.0,100.0)
  src:travel(dst)
  x=x+src.x+src.y
end
print(x)
