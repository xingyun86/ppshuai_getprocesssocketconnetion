
GetProcessConnectionps.dll: dlldata.obj GetProcessConnection_p.obj GetProcessConnection_i.obj
	link /dll /out:GetProcessConnectionps.dll /def:GetProcessConnectionps.def /entry:DllMain dlldata.obj GetProcessConnection_p.obj GetProcessConnection_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del GetProcessConnectionps.dll
	@del GetProcessConnectionps.lib
	@del GetProcessConnectionps.exp
	@del dlldata.obj
	@del GetProcessConnection_p.obj
	@del GetProcessConnection_i.obj
