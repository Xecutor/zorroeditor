class B
  on format(w,p,f,x)
    return "b"
  end
end

class A
  b=B()
  on format(w,p,f,x)
    return $b
  end
end

a=[A(),A()]
print($a)
