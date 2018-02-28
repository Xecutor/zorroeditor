local x={}
for i=1,1000000 do
  x[i]=i+1
end
local y=0.0
for j=1,10 do
--  for i,v in ipairs(x) do
  for i=1,1000000 do
    y=y+x[i]
  end
end
print(y)
