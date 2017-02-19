 ____________________________________________________________
|                                                            |
| 1964: N64 Emulator Project with GNU General Public License |
| Copyright (c) 1999  Joel Middendorf - <schibo@emuhq.com>   |
|                                                            |
| This project is a direct documentation-to-code translation |
| of the n64toolkit by my friend anarko and RSP info has     |
| been provided by zilmar :). Most source code comments are  |
| taken directly from anarko's n64toolkit with consent and   |
| are the property of anarko.                                |
|____________________________________________________________|


 ____________________________________________________________
|                                                            |
|            Table of Contents for Readme.txt                |
|                                                            |
|  I...................................1964 Version History  |
|  II............................................How to Use  |
|  III.....................................Development Team  |
|  IV.......................................Greets & Thanks  |
|____________________________________________________________|

_____________________________________________________________________________
-----------------------------------------------------------------------------

I: 1964 Version History
-----------------------

 ___________________________
|                           |
| v0.4.9 (June 12, 2000)    |
|___________________________|
  -- 2 binaries: 1964interpreter.exe and 1964dynarec.exe
     1964interpreter.exe = more compatibility but slower
     1964dynarec.exe     = less compatibility but faster
     Once I handle self-modifying code, compatibility should
     be the same in both cores. Fortunately the interpreter
     still seems a good speed on my overclocked celery 300A.

  -- What's new in 1964interpreter.exe:
       - Quest64
       - Waverace
       - Bomberman Hero (game needs serious work)
       - There's likely to be a few others.

  -- What's new in 1964dynarec.exe:
       - More speed ( what else? :) )

  -- Much cleaner and more manageable code

  -- The latest graphics plugin is included with this release so you're
     up-to-date.

 ___________________________
|                           |
| v0.4.7 (June 05, 2000)    |
|___________________________|
  -- Automatic ucode detection
  -- Menu options for changing video resolution
  -- More gfx cleanup
  -- More speed

 ___________________________
|                           |
| v0.4.5 (May 15, 2000)     |
|___________________________|
  -- Major graphics improvements; Fixed up TLB support; Improved interrupts.

  -- Included in this build is version 0.0.1 of our own hle graphics plugin. The filename is 1964ogl.dll.
     For additional version information about our dll, right click on the dll file and select "Properties".
     In the near future, Corn, TR64 and TRWin will have their own graphics plugins as well and we will 
     eventually all be compatible.

  -- The dll plugin system also allows us to release new 1964 graphics dll versions to you even before new
     1964.exe releases are available.

  -- Sad news for Geforce and Matrox G200 card owners:
     After beta testing 1964, we have learned that the 3D graphics do not work on these cards yet :\
     Other cards should work just fine, and once we've found the problem, look for a new graphics dll
     to be released shortly. Thank you for your patience. I do not have either of these cards and will need to
     get them for testing. Owners of tnt/tnt2/voodoo cards appear to be having the best results visually, 
     although OpenGL on voodoo is sluggish. Thanks to all the people in #emulation64 who have helped test.

     Playable Titles:
     Super Mario64 
     =============
         - issues: Pressing the start button to pause the game will freeze;
                   Game will also freeze after completing level 1 and returning
                   to the castle. I hope to have these problems fixed by the next release.
     Mario Kart
     ==========
          - Playable, but with serious graphics issues.

     Rampage
     =======
          - Playable as it was in v0.4.0..but known to freeze after a while.

     I don't own many games, so I don't know that much else works.

  -- As in the last release, v0.4.5 uses a "safe" dynarec method. I will
      gradually move back to the faster method which means speed will continue to improve.


 ___________________________
|                           |
| v0.4.0 (May 5, 2000)      |
|___________________________|
   -- Ported Gerrit's HLE to Windows; Rampage "works". Many more demos work.
      Most notable about this build is that the HLE engine is quite peppy. It uses OpenGL and was orginally
      designed and tested on an ATI Rage card on an iMac. I currently don't have an ATI card to test
      with.. , but I suspect that ATI cards will give the best overall ogl performance. If you have a
      card that has poor OpenGL support, I recommend downloading GLDirect from Scitech Software. GLDirect is an 
      OpenGL to Direct3D wrapper that will improve performance _dramatically_ on some cards. You can get it at the following link:
      http://www.3dfiles.com/utility/scitechgldirect.shtml.
      I found this wrapper it to work well with a Voodoo3 card, but poorly with a Voodoo2.
   -- NooTE added zlib zip support; Now you can zip your images. "Wazzaaap!"
   -- Don't hesitate to let off some steam on our messageboard. You can access it from the "MBoard" link on the 1964
      web site. "True.. True..."
   -- If you have received a prerelease beta of 1964 0.4.0, download this new release. It fixes the graphics "flicker"
      problem and is faster.
   -- I've managed to break 2D cfb demo support in this build, (and a few other things) but I hope to have that problem
      solved by the next release.
   -- Releases will likely pick up again.
   -- Right CTRL & Right Shift= A&B, Arrow keys move, A=L, S=R, 
      Home, Del, End, PageDN=C buttons. Enter=Start..a funky config, but works 4 now..
      Known controller bug: If the 1964 window loses focus, (as in: you switch to another window) controllers cease to work..
      to be fixed. "Hey! Pick up the phone!"

 ___________________________
|                           |
| v0.3.3 (January 08, 2000) |
|___________________________|
   -- More speed for Windows;
   -- Windows dev on "hold" for a month or so while Gerrit takes break from Mac
      and schibo attempts dynarec engine for Mac for more speed.
   -- dynarec source released

 ___________________________
|                           |
| v0.3.2 (December 12, 1999)|
|___________________________|
   -- Last release of 1999
   -- Whew! It's been a while, yes ? :) I didn't want to release until 
      Gerrit's Mac1964 3d hle gl fun was working in the Windows version 
      (it still doesn't, btw) but I figured it's time we showed that we're
      still alive here and back in business, lol :) (schibo)
   -- Speed optimization to FPU..Try fractal zoomer :)
   -- Various additional speed enhancements and bug fixes
   -- Thanks to icepir8 for fixes to dmult & dmultu

 ___________________________
|                           |
| v0.3.1 (October 8, 1999)  |
|___________________________|
   -- Bug fixes to dynarec and interpreter;
      Compatibility of dynarec improved- appears to have same compat
      as interpreter with the exception of firedemo and audio- which  
      do not work yet in dynarec
   -- Dynarec speed increased (schibo)
   -- Significant code cleanup

 ___________________________
|                           |
| v0.3.0 (October 1, 1999)  | 
|___________________________|
   -- First release with dynarec core; (schibo)

      There are 2 binaries: 1
         1964i.exe = interpreter (slower core)
         1964d.exe = dynarec     (faster core)

      compatibility is not quite what the interpretive core is yet..
      Here's a rough demo compatibility list from those demos i've tested:

         1964.bin
         dextrose
         fzoom (fractal screen doesn't work yet)
         hi_cfb
         liner
         lfc-fmi
         pong
         n64stars
         rotate
         rpa
         sinus
         sp_crap

      Many thanks to zilmar for helping me with dynarec and showing me a _lot_. 

   -- Various bug fixes...no more crash on exit :)
   -- RDP and dynarec continue to be in development
   -- It's October already ?! :)


 ___________________________
|                           |
| v0.2.4 (August 22, 1999)  | 
|___________________________|
   -- Started hle framework- The file 1964_HLE.exe differs from 1964.exe in that
      it traps rdp calls and shows a messagebox for each  (steb)
   -- Various optimizations to core- this will (hopefully) be the last build without
      any dynarec, so this is about as fast as the intrepreter will be... (schibo)
   -- Glide option disabled until that dll functions properly
   -- Window can now be resized and maximized and the demos will resize to fit the 
      window in DirectX


 ___________________________
|                           |
| v0.2.3 (August 16, 1999)  | 
|___________________________|
   -- More RSP added;  Many more demos play audio than before like psycho and 
      dnxintro..the skipping you will hear is because we need to work on speed 
      and buffering (steb)
   -- I would like to make a formal apology to everyone. The theft of our code was
      real, but my reaction was unprofessional in the heat of the moment. 1964 is and
      will always be licensed under the terms and conditions of the GNU GPL. Gray-area 
      legalities aside, collectively we feel too strongly about it. This will not 
      happen again. :) (schibo)


 ___________________________
|                           |
| v0.2.2 (August 12, 1999)  | 
|___________________________|
   -- Fixed a silly bug in JALR...that fixed a lot of problems, heh..(schibo)
   -- more rsp coded (steb)
   -- More demos work. We will release a new demo compatibility list soon
   -- This is a very stable build..and i wanted to sneak it in before the huge
      release of the "mystery" n64 emu on Friday! You should all be as excited
      about it as i am...and no..it is not by us (but don't i wish) :) (schibo)


 ___________________________
|                           |
| v0.2.1 (August 9, 1999)   | 
|___________________________|
   -- Improved Audio Interface support..(one buffer only) allowing 
      goldcrap demo to work 100% properly now
   -- A VI hack allows Kid Stardust intro and sp_crap demos to work
   -- TLB ops coded and more RSP code written; (unimplemented)
   -- How-To-Use section in readme updated (steb)


 ___________________________
|                           |
| v0.2.0 (August 4, 1999)   | 
|___________________________|
   -- Implemented DirectSound; Repaired hi-res demos in DirectX by resizing the
      window (steb)
      Goldcrap is a good demo to see and hear those improvements. However, goldcrap
      stops at the end of the intro...to be fixed soon :)
   -- Fixed controller input. This is the first version to play pong properly without
      any bugs. Press F1 (start button) to start. To play the liner demo, press the 
      left-shift key (A button) to start. (steb)
   -- Fixed div; Improved dll video switcher; We don't recommend
      trying the glide dll yet, though.. :) (schibo)
   -- Started RSP framework (steb)


 ___________________________
|                           |
| v0.1.9 (July 31, 1999)    | 
|___________________________|
   -- Major fixes to COP1...many more 2D demos should work properly now (schibo)
   -- plasma, spicerot, and pause are the 3 demos that I used when debugging and
      they work fine now. Freekworld now displays properly as well.
      fzoom demo (Mandelbrot zoomer) shows the fractal properly...To view it, press
      the start button- F1
   -- Thanks again to zilmar for his help :)
   -- Implemented full-screen mode for the 1964 DirectX dll (NooTe)
      Full screen does not exit gracefully yet...you will need to press Alt-Tab
      to exit. Also there is a bug that occasionally shows the window's title bar
      in full screen... that will be fixed :)
   -- The buggy fps counter has been removed but shall return when it works properly


 ___________________________
|                           |
| v0.1.8 (July 28, 1999)    | 
|___________________________|
   -- Repairs to the "likely" branch instructions
   -- Various COP1 fixes; a few speed improvements
   -- Firedemo now displays properly, FreekWorld
      demo starts to work, Mandelbrot Zoomer finally
      shows the N64 console pic on the title screen
   -- Fixed a bug in the delay slot during interrupt
      exception


 ___________________________
|                           |
| v0.1.7a (July 22, 1999)   | 
|___________________________|
   -- Fixed the directx code that i broke in 0.1.7, oops (schibo)


 ___________________________
|                           |
| v0.1.7 (July 22, 1999)    | 
|___________________________|
    -- Fixed DirectX color depth endian problems and wrong colors. 24bit will still show
       the wrong colors (NooTe)
    -- Speed improvements through code and some MSVC compiler switches (schibo)
    -- Pong works better...press F1 to start, then press the down arrow to control the
       ball...heheh..a bug that lets you cheat! (schibo)
    -- The default video mode is DirectX. To switch to OpenGL, pause the emulator first, then
       Select the OpenGL video mode...then press play...taking any other switching steps
       will crash at the moment, sorry..it will improve :)


 ___________________________
|                           |
| v0.1.6 (July 18, 1999)    | 
|___________________________|
    -- Worked DirectX, glide code into dlls (NooTe)
    -- Debugged some more COP1 ops...firedemo now just about works
       properly...the fire burns a little low on the screen, though (schibo)
    -- This build uses the DirectX dll only. You'll notice that it
       outperformes the OpenGL that existed in previous builds fairly 
       considerably. This is because with 2D screen plotting, OpenGL is more 
       awkward. The video dll implementation is part of our plan to allow the user 
       to choose which type of video to use at runtime. This will be a plugin
       system similar to that used in PsemuPro (once it works ;) ).
    -- Please only use 16bit colors for this build...we need to iron out
       the issues with the other color depths :)
    -- Also, pong starts to work, but i screwed it up, heheh, so we'll be fixing
       that too...


 ___________________________
|                           |
| v0.1.5 (July 15, 1999)    | 
|___________________________|
    -- Fixed a major bug. For some of you, 1964 crashed upon startup...yuck! The reason 
       for this was the bitmap background. I have released this version to fix that.
    -- A few more demos work with this release... you can see our screenshots at:
       http://www.emuhq.com/1964/images.htm 
       Thanks and sorry for that problem! (schibo)


 ___________________________
|                           |
| v0.1.4 (July 12, 1999)    | 
|___________________________|
    -- Implemented FiRES' OpenGL as a dll; reset CPU added to ROM options; (steb, schibo)
    -- Various minor bug fixes- Including one that fixed a graphics problem with the 
       goldcrap demo (schibo)


 ___________________________
|                           |
| v0.1.3 (July 04, 1999)    | 
|___________________________|
    -- Steb has implemented controller support and now you can play the "Liner" demo.
       You can grab a copy of this game at www.dextrose.com. The object (I think)is to
       hit the other lines with your line using the D-Pad keys. At the moment, the game 
       plays slowly, so you'll need to hold down those D-Pad keys for a little while 
       before seeing your line change direction.

       Please refer to section II below for the controls.

       You can also find this information in the file keymap.txt.
       This uses DirectInput, and therefore requires DirectX 5 or newer. 
       Also, to build the source, you'll need the DirectX 5 SDK or newer.

    -- This build also repairs the crashes that occured in a few of the demos that
       showed video on the screen then crashed. For instance, N64stars works 100%
       and goldcrap demo doesn't crash either (among others). Oddly enough, by 
       implementing a Jump opcode speed hack which Lionel showed me, that fixed 
       the crashing in those demos...bonus! :) Yes, it was likely a pointer problem.
    -- btw, our 1964.bin demo does not do anything after the first screen, so we 
       apologize if the "Press Start" message on that screen frustrated you :)
    -- Also, I have found that in 1964, if your machine does not show our logo in the
       main window, chances are that your machine will have more problems with
       the emulator. We need to find out why this varies from system to system.
       If you do not see the logo, please email your system specs to beta1964@emuhq.com
       Thanks (schibo)
    -- Implemented FPS counter (steb)


 ___________________________
|                           |
| v0.1.2 (July 02, 1999)    | 
|___________________________|
    -- Here's a build that actually plays the demo "rotate.v64" 100% :)
       Many, many thanks to Zilmar for his rewrite of the interrupts code
       and for his time. Interrupts had been driving me nuts for weeks.
       I think we still have some issues with it, but now it is much better.
       There are still many demos that will not work and will crash, but
       I suspect that many more of the more basic demos will work properly
       now. Also, I have learned a few new cool speed hacks and will 
       implement them in a future release. (schibo) 
    BB New binaries released


 ___________________________
|                           |
| v0.1.1 (June 30, 1999)    | 
|___________________________|
    -- Thanks to all of you who have emailed me your offers to help beta test
       1964. Very cool :) Do to a minor communication mix-up, the news release of 
       the beta report project was much earlier than I had planned, ( which is okay, tho :) ) 
       In the near future, we plan to set up a beta test web page with a form on it 
       for submitting reports. The form will send the beta reports to 1964@emuhq.com.
       Please do not send beta reports to my address schibo@emuhq.com, but use the
       beta1964 account instead. Thanks, everyone. Our web site will explain the 
       procedure in much more detail when we're prepared for it, but the first series
       of beta tests will basically be to measure demo compatibility and 
       anyone can participate. Other than this, there is not much need for 
       beta testing at this time. :)
    -- Repaired the nasty endian bugs for double, half, and byte loads and
       stores in the main processor (schibo);
    -- Improvement to threads (FiRES)


 ___________________________
|                           |
|  v0.1.0 (June 21, 1999)   |
|___________________________|
    -- This is the first version to play some demos; Thus far I have tested
       1964.bin and a demo by my friend anarko; I would be very grateful
       if someone could beta test the GL version and email me which demos they 
       are able to get working. I do not know all the ones which work and which 
       do not yet. :)
    -- This version exists to test the video and the GL code is threfore currently
       slow. Many, many thanks to FiRES for use of his GL video code for testing. 
       FiRES implemented the GL and was the first to ever see video on 1964! :)
    -- Improvements to interrupts (steb, schibo)
    -- Fixed the crazy memory window (schibo)
    -- Many opcode cleanups and fixes (schibo)
    -- Eudaemon: i'll include your new optimized cp0 ops soon :)
    bb New binary builds


 ___________________________
|                           |
|  v0.0.18 (June 14, 1999)  |
|___________________________|
    -- Fixed the opcodes with 64-bit sign-extended errors that caused
       crc to fail (schibo)
    -- Added memory window to debugger (steb, schibo)
    -- The debugger version no longer requires DirectX, 3Dfx to be on your
       system (schibo)
    -- As of this release, emu accuracy is 100% up to the following
       instruction in firedemo:
            0x802047D4: LW t8, t7=BFC007FC (pif ram)
            t8 should be 0 but isn't
    -- New icon (schibo); Thanks to everyone for their icon submissions. I may 
       use some of them in future releases :)
    -- New binary builds


 ___________________________
|                           |
|  v0.0.17 (June 10, 1999)  |
|___________________________|
    -- Implemented DMA and interrupts into core (steb, schibo)
    -- New memory model with lookup table thanks to Lionel of
       N64VM for the help :) (schibo)
    -- Endian support (for .v64 file types) (schibo)
    -- New binary builds- 3 binaries included: debug, release and benchmark
       versions


 ___________________________
|                           |
|  v0.0.16 (June 6, 1999)   |
|___________________________|
    -- New FP ops:
       ABS, ADD, DIV, DMFC1,BC1F, BC1FL, BC1T, BC1TL, C.Cond,
       Ceil.W, Ceil.L, CFC1, CTC1, CVT.D, CVT.L, CVT.S, CVT.W,
       DMTC1, FLOOR.L, FLOOR.W, LDC1, LWC1, MFC1, MOV, MTC1,
       MUL, NEG, ROUND.L, ROUND.W, SDC1, SQRT, SUB, SWC1
       TRUNC.L, TRUNC.W- This covers all floats, TLB is being
       started (Eudaemon)
    -- Various debugger enhancements; Improved debug printing design;
       Additional AI, VI docs; interrupts started (steb)
    -- DMA started; Fixed memory bug with MemGaps# integers (schibo)
    -- Various bug fixes (thanks DrNono & Jade)


 ___________________________
|                           |
|  v0.0.15a (June 1, 1999)  |
|___________________________|
    -- All pointers to memory are accounted for;
       New FPR Memory management; some FPR ops implemented; 
       To make debugging easier, the debugger interface has been redesigned
       to look like the debugger in PUR3a (if anyone at dextrose.com
       objects, we will revert to the previous interface);
       ROM image is now "linked" to Cartridge Domain 1 (see 
       LinkInsertRomImage() ) (schibo)
    -- Graphics code by NooTe and FiRES added.
    -- Updated video docs "vi.txt" 0.02 (steb)
    -- Note: Only the Windows GUI version will compile in this release for now.
    -- Added a new 1964 demo "1964.v64" using anarko's assembler- best viewed 
       with Nemu64; Updated "vi.txt" to 0.03 (steb)


 ___________________________
|                           |
|  v0.0.14 (May 25, 1999)   |
|___________________________|
    -- New opcodes:
         J, DSRAV, DSRVL, SRAV, SLLV, SRLV, DSLLV;
         various binary shift optimizations (Themedes)
    --   added LLBit; updates to DWORD ops (Eudaemon)
    --   implemented debugger breakpoint; most all necessary
         memory locations are "mapped" via pointers (schibo)
    --   DirectDraw interface started (NooTe)*;  
         OpenGL interface started (FiRES)*: 
         * some of your code is not present in this release 
           as we are working on a common video interface in VI[]
           for it and debugging the core


 ___________________________
|                           |
|  v0.0.13 (May 17, 1999)   |
|___________________________|
    ** Improved memory- see changes to LoadMemory(),
       StoreMemory(). No more separate arrays for mem.
       Possibly ready for VI implementation soon. 
       (Eudaemon & schibo)
    -- Improved Windows debugger interface (steb)


 ___________________________
|                           |
|  v0.0.12A (May 14, 1999)  |
|___________________________|
    (This is not a very newsworthy version)
    -- 1964 is now licensed under the terms and conditions of the 
       GNU GPL: http://www.gnu.org. As such, 1964 should no longer 
       be identified as an open source emulator.


 ___________________________
|                           |
|  v0.0.12 (May 12, 1999)   |
|___________________________|
    -- New ops:
          DDIV, DDIVU, DIV, DIVU, DMULT, DMULTU, MULT,
          MULTU, MFLO, MFHI, MTHI, MTLO, BLEZ, BGTZ,
          BGTZL, BLEZL, LDC1, LWC1, SWC1, SDC1 - Testing Required (Themedes)
    -- Removed parse function calls from opcode functions (schibo)
    -- Various Debugger repairs (thanks macDennis)


 ___________________________
|                           |
|  v0.0.11 (May 08, 1999)   |
|___________________________|
    -- New ops:
          DADDU, DSLL, DSLL32, DSUB, DSUBU, DSRA, 
          DSRA32, DSRL, DSRL32 (Themedes)
    -- Started new docs for coders- video (steb)
    -- New binary build released


 ___________________________
|                           |
|  v0.0.10 (May 04, 1999)   |
|___________________________|
    -- Major core overhaul :) Now using array of pointers
       to functions to call opcode functions. Nice! (steb)
    -- SLL & other pseudo ops compromised slightly as a 
       result, but it's well worth it. Expect a new binary build
       when that's fixed.
    -- Various Windows gui repairs- mainly, open
       dialog repaired (schibo)
    -- 1964 Win32 Console App and Win32 Application each have 
       separate project workspaces.
    -- New debugger preprocessor defs. See #defines.txt for 
       further details.


 ___________________________
|                           |
|  v0.0.9 (May 02, 1999)    |
|___________________________|
    -- DoubleWord MainCPU ops updated (Eudaemon) 
    -- New ops:
          LDC2, LWC2, SDC2, SWC2 (Eudaemon)
    -- Repaired all code causing level3 MSVC compiler warnings. (schibo)


 ___________________________
|                           |
|  v0.0.8 (April 29, 1999)  |
|___________________________|
    -- Memory remodeled.
    -- Load/store functions adapted to new model. (Thanks Eudaemon)
    -- Minimal 3Dfx glide template implemented. If interested 
       in developing for glide, email me asking how to build it. (schibo)
    

 ___________________________
|                           |
|  v0.0.7 (April 26, 1999)  |
|___________________________|
    -- We are pleased to bring you our first binary builds for DOS &
       MS-Windows!
       Much of the debugger has been given a MS-Windows overhaul
       while still maintaining the DOS version. For debugging 
       purposes, please continue to use the DOS port. Thanks to 
       steb for the help. -schibo
    -- Added benchmark mode (steb)
    -- Know issues: 
       1) "Show Header" option in Windows GUI is currently very buggy:
              -Fields are poulated incorrectly and may crash.
              -Use the DOS version for correctness. (schibo)


 ___________________________
|                           |
|  v0.0.6 (April 23, 1999)  |
|___________________________|
    -- Implemented optional Windows GUI template
       to demonstrate conditional compilation and to further
       GUI development. GUI help is welcome. :) (schibo)
       Updated SLTI, SLTIU, XORI. (schibo)
    -- New opcodes:
          SDL, SDR, SH, SWL, SWR (Eudaemon)


 ___________________________
|                           |
|  v0.0.5 (April 21, 1999)  |
|___________________________|
    -- anarko's new n64 docs v1.1 released yesterday. 
       Updates made to 1964. COP2 opcodes removed. 
       Some others are unnecessary. (schibo)
    -- Added DoFPU.c, DoVector.c, DoTLB.c (schibo)


 ___________________________
|                           |
|  v0.0.4B (April 20, 1999) |
|___________________________|
    -- (Released for testing. Testing is needed on load/store instructions)
    -- Opcodes updated:
          LDL, LDR, LWL, LWR (Eudaemon)
    -- Jump/branch fixes (schibo)
    -- Various load/store opcode repairs (schibo)


 ___________________________
|                           |
|  v0.0.4A (April 19, 1999) |
|___________________________|
    -- Improvement to delay slot (steb)
       New opcodes added: 
          JALR, BGEZ, BGEZALL, BGEZL, BLTZ, BLTZAL, BLTZALL, 
          BLTZL, BGEZAL, JR, JAL, BEQ, BEQL, BNE, BNEL (steb)
    -- Type cast target param when calling 
       LoadMemory(void* target),StoreMemory()
    -- Opcodes revised for new syntax: (Themedes)
          LB, LDU, LD, LH, LHU, LL, LLD, 
          LWU, SB, SC, SCD, SD, SH
    -- New opcodes added:
          DADDIU, DADDI, DADD, DADDU, SLL, SRA, SRL (Themedes)


 ___________________________
|                           |
|  v0.0.3B (April 18, 1999) |
|___________________________|
    -- Virtual memory addressing vastly improved (schibo)
    -- Improved debugger. CoProcessor registers now visible (schibo)
       New commands- n=next instruction
                     left arrow = back one screen
                     right arrow = forward one screen


 ___________________________
|                           |
|  v0.0.3 (April 15, 1999)  |
|___________________________|
     -- New opcodes added: 
	   SLTI, SLTIU, NOR, ADD  (steb)
           ANDI, SUB, SUBU (Themedes)
           LB, LBU, LD (Eudaemon)
     -- ROM is specified as command line parameter (steb)
     -- Ok...MainCPUReg[] & Coprocessors are now _int64. ;) (schibo)
     -- New opcodes added:
           LDL, LDR, LH, LHU, LL, LLD, 
           LWL, LWR, LWU, SB, SC, SCD (Eudaemon) 


 ___________________________
|                           |
|  v0.0.2 (April 12, 1999)  |
|___________________________|
     -- New opcodes added:
           XORI, ADDU, AND, OR, SLT, SLTU, XOR, MFCO, MTCO    (steb)
     -- Fixed "JumpToNextInstruction()" - no longer needed.   (schibo)
        Fixed another nono of mine: COP0Reg[32] is now of type int32.
        Sorry for the confusion.
     -- Removed COP0.C - Pointless file.


 ___________________________
|                           |
|  v0.0.1 (April 09, 1999)  |
|___________________________|
     -- First version of 1964 released on internet: (schibo)
           Most opcodes decoded; only a handful implemented.
           Simple debugger added. Shows MainCPU Regs only.

_____________________________________________________________________________
-----------------------------------------------------------------------------

II  How to Use
--------------

    This section still needs to be written, but I think the most 
    important information here at the moment is the controller input 
    :) so here it is. (controllers are obviously incomplete at the 
    moment):

	   START       = Enter

           UP          = Up Arrow 
           DOWN        = Down Arrow
           LEFT        = Left Arrow
           RIGHT       = Right Arrow 

           D-Pad UP    = HOME	
           D-Pad DOWN  = END 
           D-Pad LEFT  = DELETE
           D-Pad RIGHT = PAGE DOWN (NEXT)

	   A Button    = Keypad Z
           B Button    = Keypad X
	
     Demos can be downloaded from www.dextrose.com. Many thanks to dextrose
     for all the demos :) 2 playable demos in 1964 include Pong and Liner.

_____________________________________________________________________________
-----------------------------------------------------------------------------

  III 1964 Development Team
  -------------------------
  1964 was created by Joel Middendorf (schibo). schibo is
  the lead programmer

  1964 is:
  --------
      ___________       ___________
     |           |     |           |
     | Core Dev  |     |  DirectX  |
     |___________|     |___________|
         schibo            NooTe
         steb

      __________      
     |          |     
     |  OpenGL  |     
     |__________|     
        Gerrit        


Past 1964 team members:
======================
Themedes, Eudaemon
_____________________________________________________________________________
-----------------------------------------------------------------------------

IV Greets & Thanks
------------------

anarko, whose N64 Toolkit documentation made this project possible.

- Adrox, anarko, atila, BWB, Coolbest, darkreign, dave, EmuManiac, LUIGI, ManBeast,
Nicotine, Zach, zerospunk, zophar
The cool guys at darkmazda.com, emulation64.com, glideunderground.com, Einstein II,
n64zone.com and anyone else I'm forgetting.
- Ubiquity for the original 1964 logo!
- Zico for the new web site layout and logo!


	   Greets to all N64 authors
        -------------------------------
        Angelo                Ultra64
	Lionel                  N64VM
	N. Waibel               TrueReality
	F|RES, icepir8          TRWinGL
	Scav                    Pagan
	Lemmy, LaC, hWnd        Nemu64
	Subice			NSFE
	

Check out http://www.emuhq.com for the latest in emulation news
Check with http://www.emuhq.com/1964 for latest info and code updates.


1964 is not affiliated with any company in any way. Trademarks, if any are mentioned, 
are the rights and property of the respective company or companies. 1964 does not condone
software piracy.