local x=0
local ti=table.insert
local tc=table.concat
for i=1,100000 do
  local a={}
  for j=1,40 do
    ti(a,"hello")
  end
  x=x+#tc(a,"")
end
print(x)
