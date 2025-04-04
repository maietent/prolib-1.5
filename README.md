## prolib's Gunz1.5 
I cooked this up in like a month, the state the Jetman source got released in isnt the greatest, so i fixed almost all of the issues with it and added a few things, This should be a pretty stable base to make a server in my opinion.

Guide for setting it up: https://youtu.be/TxHCIKvAmRQ
GunZ Development Center: https://discord.gg/ytWyx3MSW4
The RageZone Post: https://forum.ragezone.com/threads/jetman-source-if-he-wasnt-drunk.1230537/

Compile in Release x64.

# Features:
Achievement system commented out (its unfinished and causes a deathmatch matchserver crash... zz)
Quest Fixes from Jetman
ZPackedBasicInfo int to float for char pos
Ammo Bar works properly
Medkits fixed
Sword Reload Fix (checks for nullptr unlike my old fix, credits to Lotus)
Compiles with C++17
FPS Limiter improvements
Duel Tournament should be functional (make sure to set DB up properly)
Damage Counter counts both HP and AP damage
FOV Option
UI Changes
FPS Counter updates every .25 seconds (hopefully)
Can read from unpacked MRSES (gets enabled and disabled in config.h)
UpTimeFaker to fix an uptime related issue
Fixed Jetman's broken DPI Fix
Proper DXVK Support (toggle in options)

I probably forgot some stuff I did T-T

shoutout to ini because without her there wouldnt be any ini files

# original readme
# Gunz1.5

This source contains all official game modes, as well as 
several custom game modes, and has alterations
To the engine. X64 support, direct3d9ex support, challengequest,
,blitzkrieg(unfinished), gungame, infection, and skillmap. 

A big thanks to Secrets Of The Past, Gunblade, Nick, for the amazing amount 
Of work performed on challengequest. 
