#!/bin/bash

function get_option() {
    cat $1 | sed '/^#/d' | tr '\n' '|'
}

fn=`get_option "./tpl/options/common/function.txt"`
ty=`get_option "./tpl/options/common/type.txt"`
va=`get_option "./tpl/options/common/var.txt"`

lnxfn=`get_option "./tpl/options/linux/function.txt"`
lnxty=`get_option "./tpl/options/linux/type.txt"`
lnxva=`get_option "./tpl/options/linux/var.txt"`

bindgen \
    --allowlist-function "$fn|$lnxfn" \
    --allowlist-type "$ty|$lnxty" \
    --allowlist-var "$va|$lnxva" \
    tpl/tpl.h -o src/tpl.rs

glslc src/shader.vert -o shader.vert.spv
glslc src/shader.frag -o shader.frag.spv
