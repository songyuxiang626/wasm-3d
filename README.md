#### Install emscripten sdk and prepare build environment

* `git clone https://github.com/emscripten-core/emsdk.git`
* `cd emsdk`
* `./emsdk install 3.1.51 #only tested version. I've heard newer builds don't work, but they might now`
* `./emsdk activate 3.1.51`
* `source ./emsdk_env.sh`
* `cd ..`

### Building (WebAssembly)

* `git clone https://github.com/songyuxiang626/wasm-3d.git`
* `cd wasm-3d`
* `git submodule update --init --recursive`
* `cd bgfx`
* `emmake make wasm-debug #it will probably error out on one of the examples, this is fine. just make sure bgfxDebug.bc, bxDebug.bc, and bimgDebug.bc exist in bgfx/.build/wasm/bin/`
* `cd ..`
* ` emcmake cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1`
* `cd buildwasm`
* `emmake make`