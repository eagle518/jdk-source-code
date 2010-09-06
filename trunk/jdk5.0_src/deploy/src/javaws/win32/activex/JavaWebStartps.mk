
JavaWebStartps.dll: dlldata.obj JavaWebStart_p.obj JavaWebStart_i.obj
	link /dll /out:JavaWebStartps.dll /def:JavaWebStartps.def /entry:DllMain dlldata.obj JavaWebStart_p.obj JavaWebStart_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del JavaWebStartps.dll
	@del JavaWebStartps.lib
	@del JavaWebStartps.exp
	@del dlldata.obj
	@del JavaWebStart_p.obj
	@del JavaWebStart_i.obj
