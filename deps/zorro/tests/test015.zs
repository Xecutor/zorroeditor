class Ex(msg)
  msg
end

func f(n)
  try
    for i in 1..5
      try
        print("i=$i")
        try
          return if i==n
        catch in e
          print("fuck")
        end
      catch in e
        print("wtf?")
      end
      x=0
      while x<6
        try
          x=x+1
          print("x=$x")
          throw Ex("3") if x==3
          break if x==5
          redo
        catch in e
          print("e=$e.msg")
        end
      end
      next
    end
    return 1;
  catch in e
    print("huh?",e.msg)
  end
end

func g()
  try
    f(1)
    f(6)
    throw Ex("g")
  catch in e
    print("ok:$e.msg")
  end
end

g()
