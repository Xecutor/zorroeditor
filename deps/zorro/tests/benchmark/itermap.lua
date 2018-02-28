local m={}
local fmt=string.format
for i=1,1000000 do
  m[fmt("key%d",i)]=i
end
local x=0
local y=0
for k,v in pairs(m) do
  x=x+#k
  y=y+v
end
print(fmt("x=%d,y=%d\n",x,y))
