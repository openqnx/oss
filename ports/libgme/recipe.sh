# libgme (Game_Music_Emu): emulates the sound chips of video game consoles to
# play their music files -- NSF/NSFE (NES), SPC (SNES), GBS (Game Boy), VGM/VGZ
# and GYM (Mega Drive), HES, KSS, AY, SAP -- into 16-bit PCM.
VERSION=0.6.4; GIT=https://github.com/libgme/game-music-emu.git; REF=0.6.4
DEPS="zlib"
BUILDSYS=cmake
# All emulators, zlib for compressed VGZ files; the shared library only, no
# demo/player programs (the player needs SDL audio), no tests.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DGME_BUILD_SHARED=ON -DGME_BUILD_STATIC=OFF
                   -DGME_ZLIB=ON -DGME_BUILD_TESTING=OFF -DGME_BUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF )
