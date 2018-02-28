class B
  func f()
    x=15
    x-=10
    z=++x
    print(z)
    z=x++
    print(z)
    print(x+=5)
    print(++x)
    print(x++)
  end
  xval=0
  prop x
    get
      return xval
    end
    set(val)
      print("x=$x,val=$val")
      xval=val
    end
  end
end
b=B()
b.f()
print(b.x)

