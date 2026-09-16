<div align="center">
<img height="80" alt="hopmon-log" src="https://github.com/user-attachments/assets/daf0a87b-6f45-49fd-979a-d21fad28dd2a" />
</div>

<div align="center">Decompilation of the 2001 video game Hopmon</div>

---

<div align="center">
  <img src="http://saitogames.com/_images/hopmon_screen01.jpg"></img>
  <img src="http://saitogames.com/_images/hopmon_screen03.jpg"></img>
</div>

---

Hi! This is my decomp attempt of Hopmon (2001).
Nearly all functions are byte compatible with the original, but I never bothered to figure out how to link it correctly, so the final exe differs significantly from the original.

The logic is exactly the same as the original.

In addition to the decomp, I also included an optional widescreen support with proper 16:9 aspect ratio handling. It can be enabled in cmake with the `HOPMON_FHD` flag.

I do not intend to undermine Saito's distribution rights, therefore I do not ship any textures, music, or even maps in this repo. Instead you should get them from Saito, he made [the game a freeware a few years ago](http://saitogames.com/hopmon/index.htm), this repo only provides a simple python tool for asset extraction: `./tools/extract_resources.py --exe Hopmon.exe`

# Build

Only msvc build on linux is tested for now, but it should build fine on Windows as well, just drop the `--toolchain` part
```sh
cmake -B build --toolchain cmake/msvc-wine.cmake
cmake --build build

./build/game/Hopmon.exe

# Remember to copy the original music MIDI files into Hopmon.exe directory, to get proper music playback
```

For better linux experience, I highly recommend [d7vk](https://github.com/WinterSnowfall/d7vk), just download the dll and drop it in the Hopmon.exe folder.
