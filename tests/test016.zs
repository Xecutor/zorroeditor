class A(x)
  x
  on create
    print("create A:x=$x")
  end
  on destroy
    print("destroy A:x=$x")
  end
end

class B(x=1,y=2):A(x)
  y
  on create
    print("create B:x=$x,y=$y")
  end
  on destroy
    print("destroy B:x=$x,y=$y")
  end
end

class C
  c=f()
  func f()
    return 2+2
  end
end

b=B()
b=nil
b=B(3,4)
b=nil

c=C()
print(c.c)
