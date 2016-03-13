class A
  on getIndex(idx)
    print("get index[$idx]")
    return 1
  end
  on setIndex(idx,val)
    print("set index[$idx]=$val")
  end
end

a=A()
print(a[0])
a[1]="hello"
