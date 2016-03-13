
macro observableProp(arg,defVal=nil)
  defVal="\"$defVal\"" if ?defVal==String
  yield "
  $(arg)Value=$defVal
  $(arg)Observers={}
  prop $arg
    get=$(arg)Value
    set(value)
      for obs in $(arg)Observers
        try
          obs($(arg)Value,value)
        catch in e
          $(arg)Observers-=obs
        end
      end
      $(arg)Value=value
    end
  end
"
end

class A
  @@observableProp("foo",0)
end

func f(old,new)
  print("old=$old,new=$new")
end

a=A()
a.fooObservers+=f
a.foo=5
a.foo+=1
