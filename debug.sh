#!/bin/sh -e

ninja -C bin && lldb bin/blur
