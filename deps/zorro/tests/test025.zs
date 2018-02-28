liter mask m$val
  return "mask $val"
end
v='world'
a=m"hello $v"
print(a)

liter time +hours:h?+mins:m?+secs:s?
  print("h=$hours,m=$mins,secs=$secs")
  rv=0
  rv+=hours*60*60 if hours
  rv+=mins*60 if mins
  rv+=secs if secs
  return rv
end

print(1h)
print(2h10m)
print(2.5h15s)
print(3h20m30s)
print(5m)
print(5m10s)
print(1s)


liter diceroll %cnt:d%rol
  return ^1..cnt:v=0:>rol/2
end

r=!3d6

print(r())
