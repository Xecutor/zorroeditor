x=10
while x
  print(x--)
end

while x!=10
  print(x++)
end

while x
  print(x--)
  break if x==5
end

while x
  print(x++)
  next if x<10
  break
end

x=0
while not x
  print(x++)
  redo if x<10
end
