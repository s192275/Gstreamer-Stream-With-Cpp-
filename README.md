## Gstreamer Stream With C++
This project is made in Windows environment and it aims to create a Gstreamer stream and communicate into client and server. It uses RTSP protocol to communication.
You can reach to Gstreamer's website using this [link](https://gstreamer.freedesktop.org/documentation/installing/index.html?gi-language=c) This project is compiled using MSYS2.
If you don't know what is MSYS2 you can use this [link](https://www.msys2.org/wiki/MSYS2-introduction/) to learn and you can watch this [video](https://www.youtube.com/watch?v=oC69vlWofJQ&t=1s) to install into Visual Studio Code.
### Execution Steps
First you need to provide a MP4 video to code because it uses .MP4 video. I used video source as test.mp4 but you can change the file's name in main.cpp file. 
Running steps in MSYS2
```
pacman -Syu  -> For the system update
```

```
pacman -S mingw-w64-x86_64-gcc -> For gcc installation
```

```
pacman -S mingw-w64-x86_64-gstreamer \ mingw-w64-x86_64-gst-plugins-base \ mingw-w64-x86_64-gst-plugins-good \ mingw-w64-x86_64-gst-plugins-bad \ mingw-w64-x86_64-gst-plugins-ugly
\ mingw-w64-x86_64-gst-libav \ mingw-w64-x86_64-gst-plugins-gl \ mingw-w64-x86_64-gst-plugins-gtk \ mingw-w64-x86_64-gst-plugins-qt \ mingw-w64-x86_64-gst-plugins-pulse   -> For installing gstreamer library
```

```
export PATH=/mingw64/bin:$PATH -> To add to PATH system variable  
```

```
pkg-config --modversion gstreamer-1.0 -> To check if the library is loaded
```

```
g++ -o rtsp_server test.cpp $(pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0) -> To compile the server file
```

```
./rtsp_server -> Runs the file compiled as rtsp_server.
```

This was for the server side. To make it client side you should do this commands in order: 

```
g++ -o rtsp_client rtsp_client.cpp $(pkg-config --cflags --libs opencv4) -lws2_32 -> To compile the client file
```

```
./rtsp_client -> Runs the file compiled as rtsp_client.
```
