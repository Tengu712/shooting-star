Function GetOption($path) {
    return (gc $path | ? { $_ -notmatch "^#" }) -join "|"
}

Function DoBindgen() {
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
}

Function DoGLSLC() {
    glslc src/shader.vert -o shader.vert.spv
    glslc src/shader.frag -o shader.frag.spv
}

Function BuildTPL() {
    clang -c -o tpl/tpl.obj tpl/tpl.c
    llvm-ar rc tpl/tpl.lib tpl/tpl.obj
}

If ( $Args[0].Length -eq 0 ) {
    DoBindgen
    DoGLSLC
    BuildTPL
} ElseIf ( $Args[0] -eq "bindgen" ) {
    DoBindgen
} ElseIf ( $Args[0] -eq "glslc" ) {
    DoGLSLC
} ElseIf ( $Args[0] -eq "tpl" ) {
    BuildTPL
} Else {
    $Args0 = $Args[0]
    echo "invalid argument $Args0 passed."
    exit 1
}
