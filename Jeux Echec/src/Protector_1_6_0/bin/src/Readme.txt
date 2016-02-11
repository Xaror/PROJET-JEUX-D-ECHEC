Protector is a Bitboard-based chess program that communicates with a chess GUI via the UCI protocol.

### TERMS OF USE

Protector is free, and distributed under the GNU General Public License(GPL). Essentially, this means that you are free to do almost what you want with the program, including distributing it among your friends, making it available for download from your web site, selling it (either by itself or as part of some bigger software package), or using it as the starting point for a software project of your own.

Please note that these terms do not apply for the contents of the two files "egbt.cpp" and "tbdecode.h". These files contain software from a third party (Eugene Nalimov and Andrew Kadatch) and they are not an integral part of Protector but an optional supplement.

The only real limitation is that whenever you distribute Protector in some way, you must always include the full source code, or a pointer to where the source code can be found.  If you make any changes to the source code, these changes must also be made available under the GPL.

For full details, read the copy of the GPL found in the file named Copying.txt.

### COPYRIGHTS

Please note that the sources for the endgame table access from Eugene Nalimov and Andrew Kadatch are NOT distributed under GPL. In order to use them for any other purpose than building Protector you will need an own separate permission from Eugene Nalimov and Andrew Kadatch (see the copyright remarks in their files [egtb.cpp and tbdecode.h] for more details). 

### SUPPORTED PLATFORMS

Currently Protector can be compiled and run on the following platforms:

- Windows Vista 32-bit. Use Microsoft Visual Studio 2005. Create a new "Project from existing code", add file types "*.cpp;*.c;*.h". Set the project type to "Console Application Project", and add "Support for MFC". Add "MSFT_CC" to the Preprocessor definitions. Protector should also run on Windows 2000 and XP 32-bit but I haven't tested this.

- Windows Vista/Windows 7 64-bit. Use Microsoft Visual Studio 2005 with the 64-bit libraries and look above (Windows 32-bit) for project setup. Protector also was reported to run on XP 64-bit systems.

- Ubuntu 9 Linux 32-bit and 64-bit. Use GCC or ICC (Intel C++ Compiler) and the Makefile. Protector should also run on other Linux distributions, but I haven't tested this.

- Mac OS X. Use GCC and the Makefile.

### CREDITS

Protector is based on many great ideas from the following people: Fabien Letouzey (pvnodes, blending of opening and endgame values, eval params), Thomas Gaksch (pvnode extensions, extended futility pruning, space attack eval), Robert Hyatt (consistent hashtable entries),  Stefan Meyer-Kahlen (UCI), Gerd Isenberg/Lasse Hansen (magic bitboards), Marco Costabla/Tord Romstad/Joona Kiiski (Glaurung/Stockfish sources), Igor/Yakov (Robbolito sources), Andrew Kadatch/Eugene Nalimov (endgame tablebases), Frank Rahde (testing) and Wolf Stephan Kappesser (Adaptations for Mac OS). Without their contributions Protector would not be what it is. Thank you so much.
