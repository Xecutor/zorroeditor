class A(x)
  x
  on less(val)
    if val is A
      print("$x < $val.x")
      return self<val.x
    else
      print("$x < $val")
      return x<val
    end
  end
  on boolCheck
    return false
  end
end

class B
end

func f()
  b=B()
  if b
    print("B")
  end
  if not b
  else
    print("not not b")
  end
  x="hello"
  if not A(0)
    print("not A")
  end
  y="world"
  if A(1)<A(2)
    print("1 < 2")
  end
  if not Bool(A(3))
    print("not Bool(A(3))")
  end
  print("$x $y")
  return 0
end
print(f())
