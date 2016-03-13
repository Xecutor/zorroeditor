class Test
  x=1
  on create
    print("hello:",x)
  end
  func ttt()
    print("ttt:",x)
    return 321
  end
  on destroy
    print("ciao:",x)
    ttt()
  end
end

func f()
  o=Test()
  return o.ttt()
end

print(f())
