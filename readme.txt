                   4   4
              666  4   4
       999   6   6 4   4
   1  9   9  6      4444
  11  9   9  6666      4
 1 1   9999  6   6     4
   1      9  6   6     4
   1  9   9   666
   1   999
 11111


Source Code
1964 0.8.3 Readme
May 24, 2002


To compile 1964, use Microsoft Visual C Version 6 with Service Pack 5.
Do not compile with Microsoft .NET or another compiler package. Some games break that way.
In the future after a few service packs, .NET may suffice, but as of now, no.

To build the release version of 1964, open the 1964.dsw file. Go to Build->Set Active Configuration... and choose "1964 - Win32 Release". The other 2 configurations are debug builds.
If you have questions about the debug builds, you can contact me via email at
< schibo #A#T# emulation64 ~D~O~T com. >





To Get The Most out of 1964, Read This:
=======================================

For advanced users, read Advanced_Users.txt.

Visit http://1964emu.emulation64.com. There you will find the latest downloads of 1964 
and additional links to the messageboards and faq's.


Requirements:
DirectX 8.x
PIII 600 MHz or comparable AMD Athlon Processor
Windows XP/NT/ME/98/2000
128MB RAM or better recommended


     [ Graphics ]

The latest build of 1964 features a very fast advanced recompiling engine, 
which means that many games can be played at really nice speeds, but you're
not going to enjoy it much with our OpenGL plugin.Our OpenGL graphics plugin
in its current condition is primitive. Fortunately, in the meantime, since 
several n64 emus share a compatible plugin spec,you can mix and match plugins 
from other emulators. We recommend that you download Jabo's Direct3D plugin which 
is available with PJ64. Go to http://www.pj64.net and download pj64. The filename 
will be "plugin/Jabo_Direct3D.dll" or similar. You can take this dll file and place 
it in the plugin subfolder in "1964". Then, from the plugin menu, you can choose 
Jabo's plugin in 1964.


     [ Audio and Input ]

When you first start 1964, the basic plugins will be used so that 1964 can start.
If you have the requirements to run 1964 properly,
choose Azimer Audio 0.30, or Jabo DirectSound with the rsp plugin for audio,
and NooTe_DI plugin for input.


     [ Rom Browser ]

If you do not see a list of games in your window when you start 1964, then your 
Rom Folder has not yet been configured. Go to File->Change Folder and browse for 
the path to your games. When the rom list refreshes, you can now easily choose a 
game from the window. By default, the Rom Browser is set to automatically update 
the folder path after you choose your first game from the "File->Load ROM...". 
After this, your Rom browser should be updated.


     [ Cheats ]

For help with game cheats:
1. Refer to the "Cheat Codes" subfolder in your 1964 folder
2. Visit http://cheats.emu64.com/1964/ 
                   and 
         http://dsf.emulation64.com


[ Summary of changes in 1964 0.8.3 ]

Emu Core:
1. Game save fixes. All native saves will be saved to file when the 
native save happens, not anymore at the end of the game. This will prevent game saves 
from being lost if emulator crashes
2. Supports copyright protected games Toy Story 2 and A Bug's Life
3. Supports ExciteBike64
4. Bug fix: Cannot load savestate, ITLB error 
5. Bug fix: Audio disappears when loading a state
6. EEPROM now using formal name as 4KB, 16KB instead of 2KB, 4KB
7. VI timing fixes
 
Cheat Codes Related:
1. 1964 emu core fully supports all gameshark codes, just like real 
gameshark pak
2. Bug fix: Cheat code country code


Plugins:
1. Zilmar RSP LLE plugin integration, please copy Zilmar's RSP LLE 
plugin to 1964's plugin directory in order to use it.
2. JPEG decompression via RSP LLE plugin
3. Supports Jabo's DirectSound pluign by using Zilmar RSP LLE plugin
4. Ready to support futher Netplay plugin

GUI:
1. Command line argument support and optional file extension 
association via Windows Explorer
1964 [-f] [-v video_plugin_filename] [-a audio_plugin_filename] [-c 
controller_plugin_filename] [-r directory] [-g rom_full_filename]
2. Bug fix: Statusbar flickers in full screen
3. Bug fix: GUI window coorodinates keep increasing after 1964 is 
closed
4. Bug fix: Game sorting in rom browser
5. Right-clicking on rom list header brings up popup menu
6. User can choose game name to display in the rom browser (internal game 
name, alternative game name or game filename)
7. More rom info columns are supported in rom list, right click on 
list header to select columns
8. Screen saver disabled if game is playing
9. Mouse pointer disabled in full screen mode


1964 Team is:
     1964 Development: Rice and schibo
     < schibo -A-T- emulation64 ~D~O~T~ com >
     < rice1964 -A-T- yahoo ~D~O~T~ com >

     1964 Ini: Duncan
     Cheats: Emu64 Cheats Staff
        
     FAQs: Raymond, Bobbi
     Input plugin, zip support: NooTe

For additional help, please visit:
http://www.emulation64.com, http://www.emutalk.net
http://www.ngemu.com
http://www.hosthq.net/~emuxhaven/
http://www.emuhq.com/

1964 is Copyright 2002 Joel Middendorf 
<schibo *A*T* emulation64 ~D~O~T~ com> 
under the terms and conditions of the 
GNU General Public License. 
http://www.gnu.org