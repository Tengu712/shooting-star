Function GetOption($path) {
    return (gc $path | ? { $_ -notmatch "^#" }) -join "|"
}

$fn = GetOption ".\tpl\options\common\functions.txt"
$ty = GetOption ".\tpl\options\common\types.txt"
$va = GetOption ".\tpl\options\common\variables.txt"

$winfn = GetOption ".\tpl\options\windows\functions.txt"
$winty = GetOption ".\tpl\options\windows\types.txt"
$winva = GetOption ".\tpl\options\windows\variables.txt"

bindgen `
    --allowlist-function "$fn|$winfn" `
    --allowlist-type "$ty|$winty" `
    --allowlist-var "$va|$winva" `
    tpl/tpl.h -o src/tpl.rs

glslc src/shader.vert -o shader.vert.spv
glslc src/shader.frag -o shader.frag.spv
