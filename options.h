#ifndef __OPTIONS_H
#define __OPTIONS_H

// This is the option to use DMA Segmentation
// Using this option, it dramtically increase compatibility for a lot of games
// Comment the following line out to turn this option off
#define DODMASEGMENT

#ifdef	DODMASEGMENT

// Will do SI IO DMA in segment
#define DOSIIODMASEGMENT

// Will do SP DMA in segment
#define DOSPDMASEGMENT

// Will do PI DMA in segment
// In actual, PI DMA usually copies large amount of data, should do in segment
// SI and SP usually do not copy large amount of data, it does not matter to do
// SI and SP DMA in segment or not
#define DOPIDMASEGMENT

#endif

//This option is to mark SP busy, and delay SP task for a moment of time
#define DOSPTASKCOUNTER

//This option is to mark SI IO Busy, and delay SI IO task for a moment of time
#define DOSIIOTASKCOUNTER

// Use speedhack
#define DOSPEEDHACK

// This option will use a different variable to count for VI interrupts, not use
// COUNT register any more, so COUNT register is for COMPARE interrupt only, this should
// helps some how because a lot of games will read COUNT register for timing purpose
#define SAVEVICOUNTER

// This option will enable CPU COUNT register increase at the half speep of PCLOCK. This is the
// correct way to be. While the COUNT register increase at the same speed of PCLOCK in the
// old way
#define SAVECPUCOUNTER

// This option will precisely emulate CPU PCLOCK counter for all the multi-pclock opcode
// Like integer MUL, DIV, TLBP and so on
#define SAVEOPCOUNTER

// This option will lock ROM memory to be read only, not writeable
#define LOCKROMMEMORY

// This option will let CPU to check Exceptions and Interrupt in CPUDoOtherTask function,
// CPU will not check in main emulator loop, should speed up the emulator some how
// This option will increase fps at intepreter mode from 36 to 38-40
#define CPUCHECKINTR

#ifdef CPUCHECKINTR

// This option will let CPU to check Exception and Interrupt after each opcode in Dyna mode
// If there is an exception or an interrupt happens, CPU will call the interrupt service
// routine to serve the exception or interrupt
// This option will reduce the speed of Dyna mode by 10% because it add extra checking after
// each opcode
#define DYNCPUCHECKINTR

#ifdef	DYNCPUCHECKINTR
//#define DYNCPUCHECKINTRAFTEREACHINSTRUCTION
#endif
#endif

// Ignore compare interrupt if COMPARE Register = 0
#define FASTERCOMPARE

#endif