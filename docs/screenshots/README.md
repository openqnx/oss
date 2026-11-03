# Screenshots

Real screenshots from QNX SDP 8.0 QEMU images made with `mkqnximage --graphics=yes` (virtio-gpu with
virgl, 1280x800), taken on the target with the SDP's `screenshot` tool. Every program was installed with
`apk add` from this repository's releases. How to build the same image and run them:
[docs/qemu-test-vms.md](../qemu-test-vms.md).

| | |
|---|---|
| ![labwc + Xfce panel on QNX Screen, fastfetch in xfce4-terminal](desktop-fastfetch.png)<br>**labwc + Xfce panel on QNX Screen, fastfetch in xfce4-terminal** <sub>(x86_64)</sub> | ![Geany (GTK 3) on the labwc desktop](geany.png)<br>**Geany (GTK 3) on the labwc desktop** <sub>(x86_64)</sub> |
| ![Thunar file manager](thunar.png)<br>**Thunar file manager** <sub>(x86_64)</sub> | ![Xfce settings manager](xfce4-settings.png)<br>**Xfce settings manager** <sub>(x86_64)</sub> |
| ![QDirStat (Qt 6, QNX Screen platform plugin)](qdirstat.png)<br>**QDirStat (Qt 6, QNX Screen platform plugin)** <sub>(x86_64)</sub> | ![Qt Quick (QML) straight on QNX Screen](qt-quick.png)<br>**Qt Quick (QML) straight on QNX Screen** <sub>(x86_64)</sub> |
| ![VTE terminal on GTK 4 with the GL renderer](vte-gtk4.png)<br>**VTE terminal on GTK 4 with the GL renderer** <sub>(x86_64)</sub> | ![gThumb image browser](gthumb.png)<br>**gThumb image browser** <sub>(x86_64)</sub> |
| ![MAME system list (SDL2 on QNX Screen)](mame.png)<br>**MAME system list (SDL2 on QNX Screen)** <sub>(x86_64)</sub> | ![RetroArch menu (GLES over EGL on Screen)](retroarch.png)<br>**RetroArch menu (GLES over EGL on Screen)** <sub>(x86_64)</sub> |
| ![VLC playing a test video (QNX Screen window module)](vlc.png)<br>**VLC playing a test video (QNX Screen window module)** <sub>(x86_64)</sub> | ![EmulationStation (SDL2, GLES)](emulationstation.png)<br>**EmulationStation (SDL2, GLES)** <sub>(x86_64)</sub> |
| ![raylib shapes (SDL2 + GLES 2)](raylib.png)<br>**raylib shapes (SDL2 + GLES 2)** <sub>(x86_64)</sub> | ![pygame window](pygame.png)<br>**pygame window** <sub>(x86_64)</sub> |
| ![Geany on the aarch64le desktop](geany-aarch64.png)<br>**Geany on the aarch64le desktop** <sub>(aarch64le)</sub> | ![Qt Quick on aarch64le](qt-quick-aarch64.png)<br>**Qt Quick on aarch64le** <sub>(aarch64le)</sub> |

Taken 2026-11-03. The desktop is labwc (wlroots on QNX Screen) with swaybg and the Xfce panel
(`apk add oss-desktop`); Qt programs draw on QNX Screen directly (`QT_QPA_PLATFORM=qnx`); SDL2, RetroArch
and VLC use QNX Screen with GLES over EGL.
