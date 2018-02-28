
include 'editor.zs'


enum CompletionMode
  cmNone
  cmOnType
  cmInvoked
end

class ZEditor(editor): Editor(editor)

  cmpl=[]
  cmplId=''
  cmplIdx=0

  cmplMode=cmNone

  func endCmpl()
    editor.hideCmpl()
    cmplMode=cmNone
    onCursorMove-=cmplCursorMove
  end

  func cmplCursorMove(id)
    switch id
      'up':
        if editor.prevCmpl()
          --cmplIdx
          return erHandledCont
        else
          endCmpl()
          return erProcessedEnd
        end
      'down':
        if editor.nextCmpl()
          ++cmplIdx
          return erHandledCont
        else
          endCmpl()
          return erProcessedEnd
        end
      *:
       endCmpl()
       return erProcessedEnd
    end
  end

  func getNextIndent(lineIdx)
    while lineIdx>=0
      line=editor.getLine(lineIdx)
      if line=~`^ *$`
        --lineIdx
      end
      if line=~`^(?{spaces} *)(?{kw}(?:(?:\b(?:if|while|for|switch|func|on|class|prop|namespace|try|else|enum|elsif|catch|liter|macro|bitenum)\b)|(?:[\[\{]$))?)`
        return #kw?spaces+options->indentLevel:spaces
      else
        console.addMsg("no match for '$(line)'")
        return ''
      end
    end
    return ''
  end

  func returnCmd()
    console.addMsg("cm=$CompletionMode{cmplMode}")
    if cmplMode
      complete(cmplIdx)
      return
    end
    l=editor.curLine
    cx=editor.cursorX
    dl=editor.getDepthLevel()
    if editor.cursorX==#l
      if l=~`\s*(?{kw}else|end)$`
        editor.curLine=options->indentLevel*(dl-(kw=='else'?1:0))+kw
      end
      editor.insertLine(editor.cursorY+1,"")
      editor.cursorY++
    else
      editor.curLine=*l[0..<editor.cursorX]
      editor.insertLine(editor.cursorY+1,*l[editor.cursorX..<#l])
      editor.cursorY++
      l=editor.curLine
      if l=~`\s*(?{kw}else|elsif|end)$`
        indent=options->indentLevel*(dl-1)
        editor.curLine=indent+kw
        editor.cursorX=#indent
        return
      end
    end
//    console.addMsg("l='$l', cx=$editor.cursorX")
//    --dl if l=~`\s*end$` and cx<#l
    indent=options->indentLevel*dl
    sy=editor.cursorY
    if l=~`\s*try$`
      editor.insertLine(sy+1,indent+"catch in e")
      editor.insertLine(sy+2,indent+"end")
      indent+=options->indentLevel
    end
    editor.curLine=indent+editor.curLine
    editor.cursorX=#indent
  end
  func delete()
    sel=editor.getSelection()
    if sel
      editor.delSelection()
      console.addMsg("sx=$sel[0]->startX, sy=$sel[0]->startY")
      editor.cursorY=sel[0]->startY
      editor.cursorX=sel[0]->startX
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
    if cmplMode
      if cmplId=getIdent(-1)
        console.addMsg("cmplId=$cmplId")
        showCompletions(cmplMode)
      else
        endCmpl()
      end
    end
  end
  func input(c)
    if editor.getSelection()
      editor.delSelection()
    end
    l=editor.curLine
    cl=#c
    x=editor.cursorX
    if editor.isInsideString() or editor.isInsideComment()
      editor.curLine=l.insert(x,c)
      editor.cursorX+=cl
      return
    end
    if (c=='\'' or c=='"') and x==#l
      c=c+c
    end
    if c==' ' and l=~`^\s*elsif$`
      dl=editor.getDepthLevel()
      console.addMsg("dl=$dl")
      l=options->indentLevel*(dl-1)+'elsif'
    end
    editor.curLine=l.insert(x,c)
    editor.cursorX+=cl
    if c==' ' and l=~`^(?{spaces}\s*)(?{kw}if|func|for|while|class|switch|on|prop|get|set|namespace)$`
      newline=spaces+'end'
      if editor.cursorY+1==editor.linesCount or editor.getLine(editor.cursorY+1)!=newline
        editor.insertLine(editor.cursorY+1,newline)
      end
    end
    //oldcx=editor.cursorX
    //console.addMsg("cl=$cl, ocx=$oldcx, cx=$editor.cursorX")
    saveCmplMode=cmOnType
    if cmplMode
      saveCmplMode=cmplMode
      endCmpl()
    end
    if cl==1 and c=~`[a-zA-Z_\.]`
      showCompletions(saveCmplMode)
    end
  end
  func jumpToDef()
    console.addMsg("Context:$editor.getContext()")
    ident=getIdent()
    if ident
      console.addMsg(ident)
      loc=editor.getDefLoc()
      if loc
        console.addMsg("result:$(',',loc)")
        //editor.cursorY=loc[0]
        //editor.cursorX=loc[1]
        project.openFile(loc[0],loc[1],loc[2])
      else
        console.addMsg("result:nil")
      end
    end
  end
  func showCompletions(cm=cmInvoked)
    cmplId=getIdent(-1)
    console.addMsg("id=$cmplId, cm=$CompletionMode{cm}")
    cmplId="" if not cmplId
    cmpl=editor.getCompletions(cmplId)
    return if not cmpl or not #cmpl
    editor.clearCmpl();
    for s in cmpl
      console.addMsg(">$s")
      editor.addCmpl(s)
    end
    return if #cmpl==1 and cmpl[0]==cmplId
    if #cmpl==1 and cm!=cmOnType
      input(cmpl[0][#cmplId..<#cmpl[0]])
    else
      editor.showCmpl()
      cmplMode=cm
      cmplIdx=0
      onCursorMove+=cmplCursorMove
    end
  end
  func hideCompletions()
    endCmpl()
  end
  func complete(idx)
    s=cmpl[idx]
    s=s[#cmplId..<#s]
    input(*s)
    endCmpl()
  end
  func paste()
    editor.startUndoGroup()
    if editor.haveSelection() and not options->lockedSelection
      delete()
    end
    txt=editor.getClipboard()/`\x0d?\x0a`
    if #txt==1
      input(txt[0])
    else
      l=editor.curLine
      if #l>0 and editor.cursorX!=0
        tail=l.substr(editor.cursorX)
        editor.curLine=l[0..<editor.cursorX]+txt[0]
        startIdx=1
        startY=++editor.cursorY
      else
        startIdx=0
        tail=''
        startY=editor.cursorY
      end
      editor.cursorX=0
      console.addMsg("startY=$startY")
      for l in txt[startIdx..<#txt]
        editor.insertLine(editor.cursorY+1,l)
        ++editor.cursorY
      end
      endY=--editor.cursorY
      off=#editor.curLine
      editor.curLine+=tail
      editor.indexate()
      for y in startY..endY
        editor.cursorY=y
        dl=editor.getDepthLevel()
        console.addMsg("y=$y, dl=$dl")
        l=editor.curLine
        if l=~`(?:\s*)(?{rest}.*)$`
          --dl if rest=~`^(?:end|else|elsif|catch)\b`
          console.addMsg("indent:$dl")
          editor.curLine=options->indentLevel*dl+rest
          if y==endY
            off=#editor.curLine
          end
        end
      end
      editor.cursorX=off
    end
    editor.endUndoGroup()
    /*indentLevel=''
    if editor.cursorY>0
      indentLevel=getNextIndent(editor.cursorY-1)
      if txt[0]=~`^(?: +)(?{rest}.*)`
        console.addMsg("rest=$(*rest)")
        txt[0]=indentLevel+rest
      end
    end
    console.addMsg("txt0=$txt[0]")
    input(txt[0])
    for l in txt[1..<#txt]
      returnCmd()
      if indentLevel and l=~`(?: +)(?{rest}.*)`
        l=rest
      end
      input(l) if #l
    end*/
  end
  func testfunc()
    console.addMsg("!!!")
    //w=Window()
    //w.addObj(Button())
    editor.addGhostText(editor.cursorY,editor.cursorX,"hello")
    onCursorMove+=func(id)
      editor.removeGhostText()
      return erProcessedEnd
    end
  end
  func getIdent(xshift=0)
    l=editor.curLine
    return if not #l
    start=editor.cursorX+xshift
    return if not l[start]=~`\w`
    --start while start>0 and l[start-1]=~`^[@\w]`
  //  console.addMsg("getIdent: start=$start")
    if l[start..<#l]=~`(?{ident}(?:@@)?\w+)`
      return *ident
    end
    //return "test"
  end
end



bindings+={
  'ctrl-j'=>'jumpToDef'
  'alt-space'=>'showCompletions'
  'escape'=>'hideCompletions'
  'ctrl-t'=>'testfunc'
}
