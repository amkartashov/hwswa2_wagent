====================
HWSWA2 Windows Agent
====================

Compilation
-----------

You will need to have MS Visual Studio installed to have cl.exe.

Edit env variables if needed (or just start Developer Command Prompt from Visual Studio Tools).

	SET INCLUDE=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\INCLUDE;C:\Program Files (x86)\Windows Kits\8.1\include\shared;C:\Program Files (x86)\Windows Kits\8.1\include\um;C:\Program Files (x86)\Windows Kits\8.1\include\winrt;
	SET LIB=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\LIB;C:\Program Files (x86)\Windows Kits\8.1\lib\winv6.3\um\x86;
	SET LIBPATH=C:\Windows\Microsoft.NET\Framework\v4.0.30319;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\LIB;C:\Program Files (x86)\Windows Kits\8.1\References\CommonConfiguration\Neutral;C:\Program Files (x86)\Microsoft SDKs\Windows\v8.1\ExtensionSDKs\Microsoft.VCLibs\12.0\References\CommonConfiguration\neutral;
	SET WindowsSdkDir=C:\Program Files (x86)\Windows Kits\8.1\
	SET WindowsSDK_ExecutablePath_x64=C:\Program Files (x86)\Microsoft SDKs\Windows\v8.1A\bin\NETFX 4.5.1 Tools\x64\
	SET WindowsSDK_ExecutablePath_x86=C:\Program Files (x86)\Microsoft SDKs\Windows\v8.1A\bin\NETFX 4.5.1 Tools\
	SET PATH=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\

	cl.exe /nologo /W3 /TC wagent.c debug.c selfdelete.c pipefunc.c auxiliary.c

To have debug version add switch /DDEBUG.

To install wagent.exe as a service, run in elevated command prompt:

sc create hwswa2_agent binpath= P:\ath\to\wagent.exe

.system-build file for Sublime Text:

	{
		"cmd": ["C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\bin\\cl.exe", "/nologo", "/W3", "/TC", "$file"],
	   	"selector": "source.c",
	   	"env": {
	   		"INCLUDE": "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\INCLUDE;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\shared;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\um;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\winrt;",
	   		"LIB": "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\LIB;C:\\Program Files (x86)\\Windows Kits\\8.1\\lib\\winv6.3\\um\\x86;",
	   		"LIBPATH": "C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319;C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\LIB;C:\\Program Files (x86)\\Windows Kits\\8.1\\References\\CommonConfiguration\\Neutral;C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v8.1\\ExtensionSDKs\\Microsoft.VCLibs\\12.0\\References\\CommonConfiguration\\neutral;",
	   		"WindowsSdkDir": "C:\\Program Files (x86)\\Windows Kits\\8.1\\",
	   		"WindowsSDK_ExecutablePath_x64": "C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v8.1A\\bin\\NETFX 4.5.1 Tools\\x64\\",
	   		"WindowsSDK_ExecutablePath_x86": "C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v8.1A\\bin\\NETFX 4.5.1 Tools\\"
	   	}
	}
