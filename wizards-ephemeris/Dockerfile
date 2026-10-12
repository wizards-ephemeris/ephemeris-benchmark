# Stock Emscripten 6.0.11 builds the wasm. Node 20 runs the tests and packs
# the tarball. The package stays private: this image does not publish it.

FROM emscripten/emsdk:6.0.11 AS wasm
WORKDIR /src
COPY . .
RUN emcmake cmake -S . -B build-js -DEPH_JS=ON \
 && cmake --build build-js --target eph_js

FROM node:20-bookworm AS pack
WORKDIR /src
COPY . .
COPY --from=wasm /src/js/dist/eph.js /src/js/dist/eph.wasm js/dist/
WORKDIR /src/js
RUN npm ci \
 && npm test \
 && mkdir -p /out \
 && npm pack --pack-destination /out

FROM scratch
COPY --from=pack /out/ /
