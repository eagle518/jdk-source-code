on error resume next
If isIE = "true" Then
  If Not(IsObject(CreateObject("JavaWebStart.isInstalled"))) Then
     javawsInstalled = 0
  Else
     javawsInstalled = 1
  End If
  If Not(IsObject(CreateObject("JavaWebStart.isInstalled.2"))) Then
     javawsJuniorInstalled = 0
  Else
     javawsJuniorInstalled = 1
  End If
End If
