for x in [true,false,1,0]
  for y in [true,false,1,0]
    z=x and y
    print("1:$x and $y=$z")
    print("2:$x and $y=",x and y)
    if x and y
      print("3:$x and $y=true")
    else
      print("3:$x and $y=false")
    end
    z=x or y
    print("1:$x or $y=$z")
    print("2:$x or $y=",x or y)
    if x or y
      print("3:$x or $y=true")
    else
      print("3:$x or $y=false")
    end
  end
end
