' Script to upgrade _Validation table for 64-bit MSI's
' For use with Windows Scripting Host, CScript.exe or WScript.exe

'
Option Explicit

Const msiOpenDatabaseModeReadOnly     = 0
Const msiOpenDatabaseModeTransact     = 1
Const msiOpenDatabaseModeCreate       = 3

Const msiViewModifyInsert         = 1
Const msiViewModifyUpdate         = 2
Const msiViewModifyAssign         = 3
Const msiViewModifyReplace        = 4
Const msiViewModifyDelete         = 6

Const ForAppending = 8
Const ForReading = 1
Const ForWriting = 2
Const TristateTrue = -1

' Check arg count, and display help if argument not present or contains ?
Dim argCount : argCount = Wscript.Arguments.Count
If argCount > 0 Then If InStr(1, Wscript.Arguments(0), "?", vbTextCompare) > 0 Then argCount = 0
If (argCount = 0) Then
	Wscript.Echo "Script to upgrade _Validation table for 64-bit MSI's" &_
		vbNewLine & " 1st argument is the path to MSI database (installer package)"
	Wscript.Quit 1
End If 

' Evaluate command-line arguments and set open and update modes
Dim databasePath
' databasePath = "C:/OUTPUTDIRs/6u10.win64jre/tmp/ishield/patch/jre/jre/iftw/DiskImages/DISK1/test.msi"
databasePath = Wscript.Arguments(0)


' Connect to Windows Installer object
On Error Resume Next
Dim oInstaller : Set oInstaller = Wscript.CreateObject("WindowsInstaller.Installer") : CheckError

Dim oDatabase  : Set oDatabase = oInstaller.OpenDatabase(databasePath, msiOpenDatabaseModeTransact) : CheckError
Dim view     : Set view = oDatabase.OpenView("SELECT * FROM `_Validation`") : CheckError
Dim record	 : Set record = oInstaller.CreateRecord(10) : CheckError

view.Execute : CheckError
Dim sTable, sColumn

Do
	Set record = view.Fetch : CheckError
	If record Is Nothing Then Exit Do

	sTable = record.StringData(1)
	sColumn = record.StringData(2)
	If (sTable = "RegLocator") Then
		If (sColumn = "Type") Then
			Wscript.Echo "Original row = {" & sTable & vbTab & sColumn & vbTab & record.StringData(3) & vbTab & record.StringData(4) & vbTab & record.StringData(5) & " } "

			'change validation info
			record.StringData(5) = "18"
			view.Modify msiViewModifyReplace, record : CheckError
			Wscript.Echo "New table row= {" & sTable & vbTab & sColumn & vbTab & record.StringData(3) & vbTab & record.StringData(4) & vbTab & record.StringData(5) & " } "
		End If
	End If
Loop
view.close : CheckError
Set view = Nothing

'commit validation info
oDatabase.Commit : CheckError

Set oDatabase = Nothing
CheckError


Sub CheckError
	Dim ErrMessage, errRec
	If Err = 0 Then Exit Sub
	Set ErrMessage = Err.Source & " " & Hex(Err) & ": " & Err.Description
	If Not installer Is Nothing Then
		Set errRec = installer.LastErrorRecord
		If Not errRec Is Nothing Then ErrMessage = ErrMessage & vbNewLine & errRec.FormatText
	End If
	Wscript.Echo ErrMessage
	Wscript.Quit 2
End Sub
