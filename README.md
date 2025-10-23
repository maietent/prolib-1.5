# prolib's Gunz1.5

I put this together in about a month. The state the Jetman source was released in wasn’t ideal, so I fixed almost all issues and added some improvements. This should be a pretty stable base to run a server.  

**Guide for setup:** [YouTube Tutorial](https://youtu.be/TxHCIKvAmRQ)  
**GunZ Development Center:** [Discord](https://discord.gg/ytWyx3MSW4)  
**RageZone Post:** [prolib's Gunz1.5](https://forum.ragezone.com/threads/jetman-source-if-he-wasnt-drunk.1230537/)  

Compile in **Release x64**.

---

## Features
- Achievement system commented out; unfinished and causes a deathmatch server crash)  
- Quest fixes from Jetman  
- 'ZPackedBasicInfo' converted from 'int' to 'float' for character position  
- Ammo bar works correctly  
- Medkits fixed  
- Sword reload fix (checks for 'nullptr'; credits to Lotus)  
- Compiles with C++17  
- FPS limiter improvements  
- Duel tournament functional (ensure proper DB setup)  
- Damage counter counts both HP and AP damage  
- FOV option  
- UI changes  
- FPS counter updates every 0.25 seconds  
- Can read from unpacked MRSES (toggle via 'config.h')  
- UpTimeFaker to resolve uptime-related issue  
- Fixed Jetman's broken DPI fix  
- Proper DXVK support (toggle in options)  

> I probably forgot a few changes, but this covers most of them.  

---

## Original README

This source contains all official game modes, as well as 
several custom game modes, and has alterations
To the engine. X64 support, direct3d9ex support, challengequest,
,blitzkrieg(unfinished), gungame, infection, and skillmap. 

A big thanks to Secrets Of The Past, Gunblade, Nick, for the amazing amount 
Of work performed on challengequest. 

