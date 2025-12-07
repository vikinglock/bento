# bento

bento game engine by yours truly :)
^^^
ベント 🤯🤯🤯🤯🤯🤯🤯🤯🤯🤯

this project uses the MIT license so it's gonna be free; just include that in your project

though in the bento/lib/ folder there are a bunch of files and libraries that are not in fact mine
(these include glfw, khr, bullet, glad, stb image, glm, glslang & spirv-cross, and dear imgui)

## roadmap

i don't know man

finish vulkan

webgl (sooon)

i plan on adding a bunch of things to this
i want to keep this free and open source but i am broke
for now it's gonna stay free also because no one uses it but we'll see
also i'm probably not gonna work on it for a bit

### bentoooooooooooooo

#### easy to use

people are lying to you

C++ is easy

creating a window is as easy as running

```c++
Bento* bento;
int main(){
    bento = new Bento("TITLE",width,height);
    while(bento->isRunning()){
        bento->poll();
    }
}


```

now unfortunately ios doesn't like while loops so if you wanna do ios you have to make `loop()` and `exit()` and call `Bento::startLoop()`
also ios most likely does not work at the moment *so do not use ios as a target option*

#### lightweight

in its entirety bento is less than 100MB

it's not that much i swear

#### cross-platform

**linux is untested** but macos and windows work fine i think

## include

all you gotta do is put this in the top of your code

```cpp
#include <bento/bento.h>
```

double quotes or arrows work fine i think

## building

first you gotta 'install' it with

```sh
sh bento/install.sh
```

or

```sh
./bento/install.bat
```

(all it does is compile a script that compiles ur code and puts it where it can be run as a command)

then you can run

```sh
bentoc help
```

and then something like

```sh
bentoc -macos -opengl -cached -imgui main main.cpp
bentoc -macos -metal -timed -convert main main.cpp
bentoc -windows -opengl -cached -imgui game main.cpp somethingelse.cpp
bentoc -windows -opengl -cached -convert -imgui main main.cpp
```

to give a few examples
**without -convert the shaders will not be converted**

if you don't have glfw installed

```sh
bentoc cache
```

might help

alright that aught to be enough

i would add documentation but nobody's gonna use this so i'm not gonna
like and subscribe and if we get to 5 likes i'll add documentation
