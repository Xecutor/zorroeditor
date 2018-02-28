defaultOptions={
  lockedSelection=>false
  indentLevel=>'  '
}

enum EventResult
  erHandledCont
  erHandledEnd
  erProcessedCont
  erProcessedEnd
end

class Bookmark(file,line,col)
  file
  line
  col
  func goto()
    project.openFile(file,line,col)
  end
end

bookmarks=[]

func getBookmark(idx)
  return bookmarks[idx] if idx<#bookmarks
end

func setBookmark(idx,editor)
  bookmarks[idx]=Bookmark(editor.fileName, editor.cursorY, editor.cursorX)
end

macro genBookmarkMethods()
  for i in 0..<10
    yield "
    func setBookmark$(i)()
      setBookmark($i, editor)
    end
    func gotoBookmark$(i)()
      b=getBookmark($i)
      b.goto() if b
    end
"
  end
end

class Editor(editor)
  editor
  options=*defaultOptions
  onCursorMove={}

  func up()
    return if not onCursorMoveHandler('up')
    lwc=editor.getLineWrapCount(editor.cursorY)
    if lwc==1
      --editor.cursorY
    else
      offX=editor.cursorX
      shift=nil
      for off in -editor.getLineWrapOffsets(editor.cursorY)
        if offX>=off
          shift=offX-off
          next
        end
        if shift!=nil
          editor.cursorX=off+shift
          return
        end
      end
      if shift!=nil
        editor.cursorX=shift
        return
      end
      --editor.cursorY
    end
    lwc=editor.getLineWrapCount(editor.cursorY)
    return if lwc==1
    offArr=editor.getLineWrapOffsets(editor.cursorY)
    if offArr and #offArr
      editor.cursorX+=offArr.back()
    end
  end
  func down()
    return if not onCursorMoveHandler('down')
    lwc=editor.getLineWrapCount(editor.cursorY)
    if lwc==1
      ++editor.cursorY
    else
      xoff=editor.cursorX
      last=0
      found=false
      for off in editor.getLineWrapOffsets(editor.cursorY)
        if xoff<off
          editor.cursorX+=off-last
          found=true
          break
        else
          last=off
        end
      end
      if not found
        ++editor.cursorY
        editor.cursorX=xoff-last
      end
    end
  end
  func left()
    return if not onCursorMoveHandler('left')
    if editor.cursorX==0
      --editor.cursorY
      editor.cursorX=#editor.curLine
    else
      --editor.cursorX
    end
  end
  func right()
    return if not onCursorMoveHandler('right')
    if editor.cursorX==#editor.curLine
      editor.cursorY++
      editor.cursorX=0
    else
      ++editor.cursorX
    end
  end
  func pagedown()
    return if not onCursorMoveHandler('pagedown')
    editor.pageDown()
  end
  func pageup()
    return if not onCursorMoveHandler('pageup')
    editor.pageUp()
  end
  func scrollUp()
    --editor.topLine
  end
  func scrollDown()
    ++editor.topLine
  end
  func home()
    return if not onCursorMoveHandler('home')
    if m=editor.curLine=~`^(\s+)`
      p=#m[1]
      if editor.cursorX==p
        editor.cursorX=0
      else
        editor.cursorX=p
      end
    else
      editor.cursorX=0
    end
  end
  func endCmd()
    return if not onCursorMoveHandler('end')
    editor.cursorX=#editor.curLine
  end
  func wordRight()
    return if not onCursorMoveHandler('wordright')
    l=editor.curLine
    if editor.cursorX==#l
      editor.cursorY++
      editor.cursorX=0
      return
    end
    ll=*l[editor.cursorX..<#l]
    if m=ll=~`^(\w+|\s+|\d+\.\d+)`
      editor.cursorX+=#m[1]
    else
      editor.cursorX++
    end
  end
  func wordLeft()
    return if not onCursorMoveHandler('wordleft')
    l=editor.curLine
    if editor.cursorX==0
      editor.cursorY--
      endCmd()
      return
    end
    ll=*l[0..<editor.cursorX]
    if m=ll=~`.*?(\w+|\s+|\d+\.\d+)$`
      editor.cursorX-=#m[1]
    else
      editor.cursorX--
    end
  end
  func beginOfFile()
    return if not onCursorMoveHandler('beginOfFile')
    editor.cursorX=0
    editor.cursorY=0
    editor.topLine=0
  end
  func endOfFile()
    return if not onCursorMoveHandler('endOfFile')
    editor.topLine=editor.linesCount-editor.pageSize
    editor.cursorY=editor.linesCount-1
    editor.cursorX=#editor.curLine
  end
  func backspace()
    if editor.cursorX==0
      return if editor.cursorY==0
      oldLine=editor.curLine
      editor.deleteLine(editor.cursorY)
      editor.cursorY--
      editor.cursorX=#editor.curLine
      editor.curLine+=oldLine
    else
      l=editor.curLine
      editor.curLine=l.erase(editor.cursorX-1,1)
      editor.cursorX--
    end
  end
  func delete()
    sel=editor.getSelection()
    if sel
      editor.delSelection()
      editor.cursorX=sel[0]->startX
      editor.cursorY=sel[0]->startY
      return
    end
    l=editor.curLine
    if editor.cursorX==#l
      return if editor.cursorY==editor.linesCount-1
      editor.curLine=l+editor.getLine(editor.cursorY+1)
      editor.deleteLine(editor.cursorY+1)
    else
      editor.curLine=editor.curLine.erase(editor.cursorX,1)
    end
  end
  func returnCmd()
    l=editor.curLine
    if editor.cursorX==#l
      editor.insertLine(editor.cursorY+1,"")
      editor.cursorY++
    else
      editor.curLine=*l[0..<editor.cursorX]
      editor.insertLine(editor.cursorY+1,*l[editor.cursorX..<#l])
      editor.cursorY++
    end
    if m=l=~`^(\s+)`
      indent=m[1]
      editor.curLine=indent+editor.curLine
      editor.cursorX=#indent
    else
      editor.cursorX=0
    end
  end
  func input(c)
    l=editor.curLine
    x=editor.cursorX
    editor.curLine=l.insert(x,c)
    editor.cursorX++
  end
  func delLine()
    editor.deleteLine(editor.cursorY)
  end
  func extendSelectionLeft()
    extendSelection('left')
  end
  func extendSelectionRight()
    extendSelection('right')
  end
  func extendSelectionUp()
    extendSelection('up')
  end
  func extendSelectionDown()
    extendSelection('down')
  end
  func extendSelectionHome()
    extendSelection('home')
  end
  func extendSelectionEnd()
    extendSelection('endCmd')
  end
  func extendSelectionPageUp()
    extendSelection('pageup')
  end
  func extendSelectionPageDown()
    extendSelection('pagedown')
  end
  func extendSelectionBeginOfFile()
    extendSelection('beginOfFile')
  end
  func extendSelectionEndOfFile()
    extendSelection('endOfFile')
  end
  func extendSelectionWordLeft()
    extendSelection('wordLeft')
  end
  func extendSelectionWordRight()
    extendSelection('wordRight')
  end
  func save()
    editor.save()
  end
  func copy()
    sel=editor.getSelection()
    return if not sel
    txtarr=[]
    for s in sel
      console.addMsg("sy=$s->startY,sx=$s->startX, ey=$s->endY, ex=$s->endX")
      if s->startY==s->endY
        txtarr+=*editor.getLine(s->startY)[s->startX..<s->endX]
      else
        l=editor.getLine(s->startY)
        txtarr+=*l[s->startX..<#l]+editor.getLineEoln(s->startY)
        for y in s->startY+1..<s->endY
          txtarr+=editor.getLine(y)+editor.getLineEoln(y)
        end
        txtarr+=*editor.getLine(s->endY)[0..<s->endX]
      end
    end
    txt=$txtarr
    console.addMsg("copy txt:$txt")
    editor.setClipboard($txtarr)
  end
  func paste()
    editor.startUndoGroup()
    if editor.haveSelection()
      delete()
    end
    txt=editor.getClipboard()/`\x0d?\x0a`
    console.addMsg($(',',^txt:v:>"'$(v)'"))
    input(txt[0])
    console.addMsg("after input:$editor.cursorY,$editor.cursorX")
    for l in txt[1..<#txt]
      returnCmd()
      console.addMsg("after return:$editor.cursorY,$editor.cursorX")
      input(l) if #l
      console.addMsg("after input($l):$editor.cursorY,$editor.cursorX")
    end
    editor.endUndoGroup()
  end
  func cut()
    if editor.haveSelection()
      copy()
      delete()
    end
  end
  func undo()
    editor.undo()
  end
  func softundo()
    editor.undo(true)
  end
  func focusFilter()
    editor.focusFilter()
  end
  func extendSelection(cmd)
    class LockGuard(options)
      options
      saveLock=options->lockedSelection
      on destroy
        options->lockedSelection=saveLock
      end
    end
    lockGuard=LockGuard(options)
    options->lockedSelection=true
    x=editor.cursorX
    y=editor.cursorY
    self.*cmd()
    editor.extendSelection(x,y,editor.cursorX,editor.cursorY)
  end
  func onCursorMoveHandler(id)
    for e in onCursorMove
      res=e(id)
      onCursorMove-=e if res==erHandledEnd or res==erProcessedEnd
      return false if res==erHandledEnd or res==erHandledCont
    end
    if not options->lockedSelection
      editor.clearSelection()
    end
    return true
  end
  @@genBookmarkMethods()

  func showFind()
    editor.showFind()
  end
  func findNext()
    editor.findNext()
  end
  func findBack()
    editor.findBack()
  end
end

ctrlOrCmd=osType=='macos'?'meta':'ctrl'

bindings={
  'ctrl-up'=>'scrollUp'
  'ctrl-down'=>'scrollDown'
  'ctrl-left'=>'wordLeft'
  'ctrl-right'=>'wordRight'
  'ctrl-home'=>'beginOfFile'
  'ctrl-end'=>'endOfFile'
  'ctrl-y'=>'delLine'
  'shift-left'=>'extendSelectionLeft'
  'shift-right'=>'extendSelectionRight'
  'shift-up'=>'extendSelectionUp'
  'shift-down'=>'extendSelectionDown'
  'shift-home'=>'extendSelectionHome'
  'shift-end'=>'extendSelectionEnd'
  'shift-pageup'=>'extendSelectionPageUp'
  'shift-pagedown'=>'extendSelectionPageDown'
  'ctrl-shift-right'=>'extendSelectionWordRight'
  'ctrl-shift-left'=>'extendSelectionWordLeft'
  'ctrl-shift-home'=>'extendSelectionBeginOfFile'
  'ctrl-shift-end'=>'extendSelectionEndOfFile'
  "$ctrlOrCmd-s"=>'save'
  "$ctrlOrCmd-c"=>'copy'
  "$ctrlOrCmd-v"=>'paste'
  "$ctrlOrCmd-x"=>'cut'
  "$ctrlOrCmd-z"=>'undo'
  "shift-$ctrlOrCmd-z"=>'softundo'
  "$ctrlOrCmd-o"=>'focusFilter'
  'return'=>'returnCmd'
  'end'=>'endCmd'
  "$ctrlOrCmd-f"=>'showFind'
  'f3'=>'findNext'
  'shift-f3'=>'findBack'
}

addBookmarkKeys()

func addBookmarkKeys()
  for i in 0..9
    bindings{"$ctrlOrCmd-shift-$i"}="setBookmark$i";
    bindings{"$ctrlOrCmd-$i"}="gotoBookmark$i";
  end
  for k,v in bindings
    console.addMsg("$k => $v")
  end
end
