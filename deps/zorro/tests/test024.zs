class A
  x
  yVal
  zVal
  prop y
    get=yVal
    set=yVal
  end
  prop z
    get
      return zVal
    end
    set(val)
      zVal=val
    end
  end
  arr=[]
  on getIndex(idx)
    return arr[idx]
  end
  on setIndex(idx,val)
    arr[idx]=val
  end
  dyn={=>}
  on getProp(name)
    return dyn{name}
  end
  on setProp(name,val)
    dyn{name}=val
  end
end

a=A()
a.x=0
print(a.x++)
print(++a.x)
print(a.x)
print(a.x+=5)
print(a.x)

a.y=10
print(a.y++)
print(++a.y)
print(a.y)
print(a.y+=5)
print(a.y)

a.z=100
print(a.z++)
print(++a.z)
print(a.z)
print(a.z+=5)
print(a.z)

a[0]=1000
print(a[0]++)
print(++a[0])
print(a[0])
print(a[0]+=5)
print(a[0])

a.t=10000
print(a.t++)
print(++a.t)
print(a.t)
print(a.t+=5)
print(a.t)
