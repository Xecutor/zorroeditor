func clsgen(n)
  return func(i)
    return i+n
  end
end

func ff(f)
  print(f(3))
end

ff(clsgen(7))
