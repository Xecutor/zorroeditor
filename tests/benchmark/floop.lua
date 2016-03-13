local function f(a,b)
  local v=a
  if v>=b then
    v=v+b
  end
  return v
end
local x=0.0
for i=0,9999999 do
 x=f(x,i)
end
print(x)
