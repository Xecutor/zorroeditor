h={
  a=>1
  b=>2
}
for i in 1..10000
  h{"key$5z:i"}=i
end
for i in 1..10000000
  h->a,h->b=h->b,h->a
end
//print("a=$h->a,b=$h->b")
