Function GetOption($path) {
    return (gc $path | ? { $_ -notmatch "^#" }) -join "|"
}

$fn = GetOption ".\tpl\options\common\function.txt"
$ty = GetOption ".\tpl\options\common\type.txt"
$va = GetOption ".\tpl\options\common\var.txt"

$winfn = GetOption ".\tpl\options\windows\function.txt"
$winty = GetOption ".\tpl\options\windows\type.txt"
$winva = GetOption ".\tpl\options\windows\var.txt"

bindgen `
    --allowlist-function "$fn|$winfn" `
    --allowlist-type "$ty|$winty" `
    --allowlist-var "$va|$winva" `
    tpl/tpl.h -o src/tpl.rs

glslc src/shader.vert -o shader.vert.spv
glslc src/shader.frag -o shader.frag.spv
