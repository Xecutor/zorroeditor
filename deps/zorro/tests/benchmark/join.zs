x=0
for i in 1..100000
  a=[]
  for j in 1..40
    a+="hello"
  end
  x+=#$a
end
print(x)
