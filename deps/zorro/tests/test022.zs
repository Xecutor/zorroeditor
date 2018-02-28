class A
  val="world"
  dlgStore
  on destroy
    print("destroy")
  end
  func someMethod()
    print("hello:$val")
  end
end

a=A()
a.dlgStore=&&a.someMethod
a.dlgStore()
a=nil
