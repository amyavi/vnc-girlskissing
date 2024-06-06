# vnc-girlskissing
static image vnc server

## building
requires `libvncserver`

instructions:
1. `cmake -B build -DCMAKE_BUILD_TYPE=Release`
2. `cmake --build build -j$(nproc)` 

### static build
just add `-DBUILD_STATIC=1` to your cmake parameters

note that you will have to go through the hell that is building `libvncserver` statically,
if you're not too sure how to do that, look at the dockerfile

## running (docker)
1. put your image in `./image.png`
2. edit the vnc name in docker-compose.yml
3. `docker compose up --build`

## running
`./vnc-girlskissing [image path] [vnc name]`
- image path defaults to `image.png` in the current directory
- vnc name defaults to `amyavi/vnc-girlskissing`
