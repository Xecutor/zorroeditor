class A
  on boolCheck
    return false
  end
end

class B
  on boolCheck
    return true
  end
end

class C
end

func test(cls,ta,fa)
  v=cls()
  cn=cls.getName()
  if v
    print("$cn::$(ta)1")
  else
    print("$cn::$(fa)1")
  end
  if not v
    print("$cn::$(fa)2")
  else
    print("$cn::$(ta)2")
  end
end

test(A,"false","true")
test(B,"true","false")
test(C,"true","false")
