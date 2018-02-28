func ftrue()
  return true
end

func ffalse()
  return false
end

if ffalse() or ftrue()
  print("ok")
else
  print("fail")
end

if ffalse() or ffalse() or ftrue()
  print("ok")
else
  print("fail")
end

for x in 1..2
if (x==2 or x==1) and (x==1 or x==2)
  print("ok")
else
  print("fail")
end
end

x=0
if ++x
  print("ok")
else
  print("fail")
end

if x--
  print("ok")
else
  print("fail")
end

y=0
x=y or y or 1
if x==1
  print("ok")
else
  print("fail")
end

y=2
x=x and y and y
if x==2
  print("ok")
else
  print("fail")
end
