class Point(x,y)
  x
  y
  dx=0
  dy=0
  func move()
    x+=dx
    y+=dy
  end
  func isNear(p)
    vx=x-p.x
    vx*=vx
    vy=y-p.y
    vy*=vy
    return vx+vy<5
  end
  func calcDir(p)
    dx=p.x-x
    dy=p.y-y
    l=math::sqrt(dx*dx+dy*dy)
    dx/=l
    dy/=l
  end
  func travel(dst)
    calcDir(dst)
    while not isNear(dst)
      move()
    end
  end
end

x=0.0
for i in 1..10000
  src=Point(0.0,0.0)
  dst=Point(100.0,100.0)
  src.travel(dst)
  x+=src.x+src.y
end
print(x)
