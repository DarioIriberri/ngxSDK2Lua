ngxSDK2Lua
==========

A dll that allows to read the PMDG NGX's SDK data from FSUIPC4/Lua

ngxSDK2Lua version 0.5
======================

----------------------------------------------------------------------------------------------------

This is a dll I've programmed that allows to read the PMDG NGX data from FSUIPC4/Lua. 
There's no support for sending stuff to the NGX as that's already covered by FSUIPC4 / LINDA.
It's an interface, a "link" between the NGX SDK and FSUIPC4, but the installer includes some modules
ready to use for some devices. I plan on including new modules as (if) they become available.

It's worth noting that at the time of  writing this (March, 2012), PMDG and Pete Dowson are 
supposed to be in talks to reach an agreement that may allow Pete to provide native support for 
the NGX in FSUIPC4, so you may want to check out his forums and use that instead if available. 
I started with this a couple of days before that happened so decided to finish it anyway. 
I want to thank Pete for his time and awesome help, PMDG for their SDK and both of them for their 
great products.
Also thanks to Paul Dumke for helping in the testing & debugging process.

Whatever you do, make sure you abide by everything in PMDG's SDK EULA and terms of use, 
that you must agree to upon installation of this software.

----------------------------------------------------------------------------------------------------

REQUIREMENTS

- FSX
- PMDG 737NGX - SP1c
- FSUIPC4, registered & up-to-date

----------------------------------------------------------------------------------------------------

SUPPORTED DEVICES / INCLUDED MODULES IN THIS VERSION

- GoFlight MCP
- VRInsight MCP Combo (stand alone module)
- VRInsight MCP Combo (to use in combination with LINDA)

----------------------------------------------------------------------------------------------------

HOW TO USE THIS THING

Simply run the installer and pick the modules for the devices you want to use with the NGX.
If you want to modify the installation, simply re-run the installer and add or remove the modules
as required. 

After running the installer you will get a number of files copied to you FSX installation:
<FSX ROOT>/Modules/lua/ngxSDK2Lua.dll -> the module library
<FSX ROOT>/Modules/ngxSDK2Lua -> every folder here is a module installed, typically you have one 
module per device.
<FSX ROOT>/Modules/startNGXSDK2Lua.lua -> the initialization script for the module

In the installation folder you'll have this Readme, the uninstaller and some other files the 
installer and uninstaller need.

The installer will edit your <FSX ROOT>/PMDG/PMDG 737 NGX/737NGX_Options.ini to include the entry:

[SDK]
EnableDataBroadcast=1

to enable the data communication output with the NGX.

It will also add (or edit) the file <FSX ROOT>/Modules/ipcReady.lua to launch the dll when you start 
a flight with the NGX.
----------------------------------------------------------------------------------------------------

PROGRAM YOUR OWN MODULES

Each module should go in it's own folder at <FSX ROOT>/Modules/ngxSDK2Lua. For example, the sample
module included in the installer is in the folder called "sample module". In that folder you should
create a lua script called "moduleConfig.lua" with the module implementation. 

When the NGX starts running, the dll will look for folders/modules in <FSX ROOT>/Modules/ngxSDK2Lua, 
and will load the moduleConfig.lua script.
First thing the dll does with moduleConfig.lua is to call a function moduleConfig(). There you can 
start other scripts, register offsets (more on this later) or whatever you need. 

There are 3 things you can do:

<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

A) Implement a set of Lua functions that will run automatically every time a certain item in 
the SDK changes value. Example:

require "ngxSDK2Lua" -- loads the module

function IRS_DisplaySelector(param) 
	ipc.log("IRS_DisplaySelector: " .. param) 
end

This function will run every time the IRS DisplaySelector switch changes position. In the example 
it simply will print  the value in the FSUIPC log. 
The names of the functions should match exactly the names of the items as defined in the NGX SDK 
that you should already have.

For array members like IRS_ModeSelector[2] the convention for the function names is to add "_index#": 

IRS_ModeSelector_0 for the first member of the array IRS_ModeSelector[0]
IRS_ModeSelector_1 for the second member of the array IRS_ModeSelector[1]

And so on:

function IRS_ModeSelector_0(param) ipc.log("IRS_ModeSelector_0: " .. param) end
function IRS_ModeSelector_1(param) ipc.log("IRS_ModeSelector_1: " .. param) end

You can use the moduleConfig.lua file provided in the sample module as a template, 
implement the functions you need and delete the rest.

If you need those functions in a different Lua script for whatever reason, you can instruct the dll
to look for the functions in another script adding something like this to the moduleConfig() function:

ngxSDK2Lua.registerLuaFile("<path of the script relative to the modules\ngxSDK2Lua folder>") 

for instance:

function moduleConfig() 
	require "ngxSDK2Lua" 
	ngxSDK2Lua.registerLuaFile("sample module\\myFunctions.lua")
end

Remember to use double back-slashes

<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

B) Register items to be written to specific offsets in FSUIPC every time they change. 
This will keep the NGX items you want in the offsets of your choice.
For all I know this is how opencockpits devices work.

An example of the moduleConfig() function with an item registration:

function moduleConfig() 
	require "ngxSDK2Lua" 
	ngxSDK2Lua.registerOffsetMap("IRS_DisplaySelector", 0x66C1) -- store the item IRS_DisplaySelector in the offset 0x66C1
end

It will map internally to the appropiate data size depending on the item type at hand:

unsigned char  -> writeUB
char           -> writeSB
float          -> writeFLT
unsigned short -> writeUW
short          -> writeSW
unsigned int   -> writeUD
boolean        -> setbitsUB for 0 <= bitPos <= 7, setbitsUW for 8 <= bitPos <= 15 or setbitsUD for 16 <= bitPos <= 31

For boolean items you need another parameter to inform the bit position in the offset:

ngxSDK2Lua.registerOffsetMap("IRS_SysDisplay_R", 0x66C0, 0)

This keeps the value of the boolean item "IRS_SysDisplay_R" in the least significant (0) bit of the 
offset 0x66C0. So the third parameter (0 in the example) is the bit position where the item will be 
stored. 

You can combine A) and B), meaning that you can have some items processed in the lua
functions and some others stored in offsets, or even have items use both. 

<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

C) The last thing you can do is to call "get" functions to retrieve the data at will from your own scripts. 
You can do this from any Lua script in your modules/LINDA folders adding a "get_" prefix to the name of 
the item that should follow the same naming convention as in the callback.lua functions for array items:

require "ngxSDK2Lua" -- loads the module
resp = ngxSDK2Lua.get_IRS_SysDisplay_R() -- retrieves the "IRS_SysDisplay_R" data
ipc.log("get_IRS_SysDisplay_R() = "..resp)

<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

UNINSTALLING:

From the control panel -> uninstall a program, look for ngxSDK2Lua in the list and uninstall it.

----------------------------------------------------------------------------------------------------

And that's it. If you have any questions, bug reports, suggestions...

mail me at dario.iriberri@gmail.com