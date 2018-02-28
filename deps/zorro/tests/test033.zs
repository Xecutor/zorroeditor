class A
  a=false
  b=false
  func f()
    return a if a
    return b if b
  end
  prop ppp
    get
      return a if a
    end
  end
end

l=func(a,b)
  return a if a
  return b if b
end

print(l(false,false))
a=A()
print(a.f())
print(a.ppp)
