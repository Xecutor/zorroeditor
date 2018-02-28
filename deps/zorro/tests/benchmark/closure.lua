local function g(x,y,z)
  local q=function()
    return x+y+z
  end
  return q()
end
local function f(x,y,z)
  return function()
    return g(x,y,z)
  end
end

local z=f(1,2,3)
local x=0
for i=1,1000000 do
  x=x+z()
end
print(x)
