class A
  on getKey(idx)
    print("get key{$idx}")
    return 1
  end
  on setKey(idx,val)
    print("set key{$idx}=$val")
  end
end

a=A()
print(a->somekey)
a->somekey="hello"
