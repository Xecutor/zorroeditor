class A
  on getProp(name)
    print("get prop.$name")
    return 1
  end
  on setProp(name,val)
    print("set prop.$name=$val")
  end
end

a=A()
print(a.someprop)
a.someprop="hello"
